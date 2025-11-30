/**
 * @file WS2812B.c
 * @brief WS2812B LED strip driver for STM32 Blue Pill with RGB, HSV, and HSL color support.
 *
 * This module provides low-level control of WS2812B LEDs using PWM + DMA on TIM3.
 * It includes color space conversion (RGB ↔ HSV ↔ HSL) and basic animation helpers.
 *
 * @note Designed for STM32F103C8T6 (Blue Pill) with 8 MHz APB1 clock.
 * @author Your Name
 * @date November 2025
 */

#include "WS2812B.h"
#include <stdlib.h>

// === Global Variables (must match main.c) ===
extern TIM_HandleTypeDef htim3;
extern DMA_HandleTypeDef hdma_tim3_ch1_trig;
/** @brief PWM buffer for WS2812B data (24 bits per LED + 50 reset bits) */
uint16_t pwmData[WS2812B_DATA_SIZE + 50];

/**
 * @brief DMA transmission complete callback.
 * @param htim Timer handle that triggered the callback.
 * @note Stops PWM to prevent re-triggering.
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim3)
    {
        HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_1);
    }
}

/**
 * @brief Start DMA transmission of LED data to WS2812B strip.
 * @note Automatically sends WS2812B_DATA_SIZE + 50 bits (includes reset pulse).
 */
void WS2812B_Send(void)
{
    HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t*)pwmData, WS2812B_DATA_SIZE + 50);
}

/**
 * @brief Set a single LED pixel using RGB color.
 * @param pixel Pixel index (0 to LED_NUM - 1)
 * @param red Red component (0–255)
 * @param green Green component (0–255)
 * @param blue Blue component (0–255)
 * @note Does nothing if pixel index is out of bounds.
 */
void WS2812B_SetPixelRGB(uint16_t pixel, uint8_t red, uint8_t green, uint8_t blue)
{
    if (pixel >= LED_NUM) return;

    // WS2812B uses GRB order
    uint32_t color = ((uint32_t)green << 16) | ((uint32_t)red << 8) | blue;
    uint16_t pos = pixel * 24;

    for (int i = 0; i < 24; i++)
    {
        if (color & (1U << (23 - i)))
        {
            pwmData[pos + i] = 58;  // High bit (~900ns HIGH)
        }
        else
        {
            pwmData[pos + i] = 29;  // Low bit (~350ns HIGH)
        }
    }
}

/**
 * @brief Turn off all LEDs by setting them to black.
 * @note Also ensures the 50+ reset bits at the end are zero.
 */
void WS2812B_Clear(void)
{
    for (int i = 0; i < WS2812B_DATA_SIZE + 50; i++)
    {
        pwmData[i] = (i < WS2812B_DATA_SIZE) ? 29 : 0;
    }
}

/**
 * @brief Set all LEDs to the same RGB color.
 * @param red Red (0–255)
 * @param green Green (0–255)
 * @param blue Blue (0–255)
 */
void WS2812B_SetColorRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    for (int led = 0; led < LED_NUM; led++)
    {
        WS2812B_SetPixelRGB(led, red, green, blue);
    }
    // Ensure reset tail is zero
    for (int i = WS2812B_DATA_SIZE; i < WS2812B_DATA_SIZE + 50; i++)
    {
        pwmData[i] = 0;
    }
}

// ===================================================================
// ==================== COLOR SPACE CONVERSIONS ======================
// ===================================================================

/**
 * @brief Convert HSV to RGB.
 * @param h Hue in degrees (0–359)
 * @param s Saturation in percent (0–100)
 * @param v Value (brightness) in percent (0–100)
 * @param[out] r Pointer to red output (0–255)
 * @param[out] g Pointer to green output (0–255)
 * @param[out] b Pointer to blue output (0–255)
 * @note Based on common HSV-to-RGB algorithm. Uses integer arithmetic.
 */
static void hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (s == 0)
    {
        uint8_t grey = (v * 255U) / 100U;
        *r = *g = *b = grey;
        return;
    }

    h %= 360;
    uint16_t hi = h / 60;
    uint8_t f = ((h % 60) * 255U) / 60U;

    uint8_t v255 = (v * 255U) / 100U;
    uint8_t s255 = (s * 255U) / 100U;

    uint8_t p = (v255 * (255U - s255)) / 255U;
    uint8_t q = (v255 * (255U - f)) / 255U;
    uint8_t t = (v255 * f) / 255U;  // simplified from original

    switch (hi)
    {
        case 0: *r = v255; *g = t;     *b = p; break;
        case 1: *r = q;     *g = v255; *b = p; break;
        case 2: *r = p;     *g = v255; *b = t; break;
        case 3: *r = p;     *g = q;     *b = v255; break;
        case 4: *r = t;     *g = p;     *b = v255; break;
        case 5: *r = v255; *g = p;     *b = q; break;
    }
}

/**
 * @brief Helper function for HSL-to-RGB conversion.
 * @param p First intermediate value
 * @param q Second intermediate value
 * @param t Hue segment (0–255)
 * @return RGB component (0–255)
 */
static uint8_t hue2rgb(uint8_t p, uint8_t q, uint8_t t)
{
    if (t < 43)      return p + (((q - p) * t) / 43);
    else if (t < 128) return q;
    else if (t < 171) return p + (((q - p) * (171 - t)) / 43);
    else              return p;
}

/**
 * @brief Convert HSL to RGB.
 * @param h Hue in degrees (0–359)
 * @param s Saturation in percent (0–100)
 * @param l Lightness in percent (0–100)
 * @param[out] r Pointer to red output (0–255)
 * @param[out] g Pointer to green output (0–255)
 * @param[out] b Pointer to blue output (0–255)
 * @note Uses standard HSL-to-RGB algorithm with integer math.
 */
static void hsl_to_rgb(uint16_t h, uint8_t s, uint8_t l, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (s == 0)
    {
        uint8_t grey = (l * 255U) / 100U;
        *r = *g = *b = grey;
        return;
    }

    // Scale hue to 0–255 range for compatibility with hue2rgb
    uint8_t h255 = (h * 255U) / 360U;

    uint8_t l255 = (l * 255U) / 100U;
    uint8_t s255 = (s * 255U) / 100U;

    // Compute q and p (standard formula)
    uint16_t temp_q = (l < 50)
        ? ((uint16_t)l255 * (255U + s255))
        : ((uint16_t)(l255 + s255) * 255U - (uint16_t)l255 * s255);
    uint8_t q = (uint8_t)(temp_q / 255U);

    uint8_t p = (2 * l255 > 255U) ? (2 * l255 - q) : 0;
    if (l < 50) {
        p = (2 * l255 * (255U - s255)) / 255U;
    }

    // Convert hue segments to RGB
    *r = hue2rgb(p, q, (h255 + 85U) % 256U);  // +120° → +85 in 0–255
    *g = hue2rgb(p, q, h255);
    *b = hue2rgb(p, q, (h255 + 171U) % 256U); // +240° → +171 in 0–255
}

// ===================================================================
// ==================== PUBLIC HSV FUNCTIONS =========================
// ===================================================================

/**
 * @brief Set all LEDs to a color defined in HSV space.
 * @param hue Hue (0–359)
 * @param sat Saturation (0–100%)
 * @param val Value/brightness (0–100%)
 */
void WS2812B_SetColorHSV(uint16_t hue, uint8_t sat, uint8_t val)
{
    uint8_t r, g, b;
    hsv_to_rgb(hue, sat, val, &r, &g, &b);
    WS2812B_SetColorRGB(r, g, b);
}

/**
 * @brief Set a single LED to a color defined in HSV space.
 * @param pixel Pixel index (0 to LED_NUM - 1)
 * @param hue Hue (0–359)
 * @param sat Saturation (0–100%)
 * @param val Value (0–100%)
 */
void WS2812B_SetPixelHSV(uint16_t pixel, uint16_t hue, uint8_t sat, uint8_t val)
{
    uint8_t r, g, b;
    hsv_to_rgb(hue, sat, val, &r, &g, &b);
    WS2812B_SetPixelRGB(pixel, r, g, b);
}

// ===================================================================
// ==================== PUBLIC HSL FUNCTIONS =========================
// ===================================================================

/**
 * @brief Set all LEDs to a color defined in HSL space.
 * @param hue Hue (0–359)
 * @param sat Saturation (0–100%)
 * @param light Lightness (0–100%)
 */
void WS2812B_SetColorHSL(uint16_t hue, uint8_t sat, uint8_t light)
{
    uint8_t r, g, b;
    hsl_to_rgb(hue, sat, light, &r, &g, &b);
    WS2812B_SetColorRGB(r, g, b);
}

/**
 * @brief Set a single LED to a color defined in HSL space.
 * @param pixel Pixel index (0 to LED_NUM - 1)
 * @param hue Hue (0–359)
 * @param sat Saturation (0–100%)
 * @param light Lightness (0–100%)
 */
void WS2812B_SetPixelHSL(uint16_t pixel, uint16_t hue, uint8_t sat, uint8_t light)
{
    uint8_t r, g, b;
    hsl_to_rgb(hue, sat, light, &r, &g, &b);
    WS2812B_SetPixelRGB(pixel, r, g, b);
}

// ===================================================================
// ==================== LEGACY / DEMO EFFECTS ========================
// ===================================================================

/**
 * @brief Classic rainbow effect using hardcoded RGB wheel.
 * @note Does not use HSV/HSL conversion — fast but less flexible.
 */
void WS2812B_RainbowClassic(void)
{
    for (int i = 0; i < LED_NUM; i++)
    {
        uint16_t hue = (i * 255U) / LED_NUM;
        if (hue < 85)
        {
            WS2812B_SetPixelRGB(i, hue * 3, 255 - hue * 3, 0);
        }
        else if (hue < 170)
        {
            hue -= 85;
            WS2812B_SetPixelRGB(i, 255 - hue * 3, 0, hue * 3);
        }
        else
        {
            hue -= 170;
            WS2812B_SetPixelRGB(i, 0, hue * 3, 255 - hue * 3);
        }
    }
    WS2812B_Send();
}