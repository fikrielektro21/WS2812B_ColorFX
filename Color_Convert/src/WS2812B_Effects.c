/*
 * ws2812b_Effects.c
 *
 *  Created on: Nov 30, 2025
 *      Author: User
 *
 *  @brief High-level animated effects for WS2812B LEDs using RGB, HSV, or HSL color spaces.
 *  Supports automatic cycling, brightness/speed control, and stateful effect management.
 */

#include "WS2812B_Effects.h"

// Global state variables (internal to this module)
static uint16_t rainbow_hue = 0;
static uint8_t breathe_val = 50;
static int8_t breathe_dir = 1;
static uint8_t theater_frame = 0;
static uint8_t global_brightness = 100;  ///< 0–100%
static uint8_t global_speed = 50;        ///< 1–100 (higher = faster)

/**
 * @brief Initialize the effects state machine with default values.
 * @param effects Pointer to the effects configuration structure.
 * @note Sets initial effect to rainbow chase with auto-cycling every 5 seconds.
 */
void WS2812B_Effects_Init(ws2812b_effects_t* effects) {
    effects->current_effect = EFFECT_RAINBOW_CHASE;
    effects->hue = 0;
    effects->brightness = 50;
    effects->breathe_direction = 1;
    effects->theater_frame = 0;
    effects->effect_speed = 50;
    effects->auto_cycle = true;
    effects->cycle_duration = 5000; // 5 seconds
}

/**
 * @brief Main effect handler — call this in your main loop.
 * @param effects Pointer to the current effects state.
 * @note Automatically cycles effects if auto_cycle is enabled.
 *       Always calls WS2812_Send() at the end.
 */
void WS2812B_Effects_Handle(ws2812b_effects_t* effects) {
    static uint32_t last_cycle = 0;

    // Auto cycle effects
    if (effects->auto_cycle && (HAL_GetTick() - last_cycle > effects->cycle_duration)) {
        effects->current_effect = (effects->current_effect + 1) % 6;
        last_cycle = HAL_GetTick();
        WS2812_Clear();
    }

    // Execute current effect
    switch (effects->current_effect) {
        case EFFECT_STATIC_COLOR:
            WS2812B_SolidColor(COLOR_HSV, effects->hue, 100, global_brightness);
            effects->hue = (effects->hue + 1) % 360;
            break;

        case EFFECT_RAINBOW_CHASE:
            WS2812B_Rainbow(COLOR_HSV);
            break;

        case EFFECT_FIRE:
            WS2812B_Fire();
            break;

        case EFFECT_BREATHE:
            WS2812B_Breathe(COLOR_HSV, effects->hue, 100, global_brightness);
            effects->hue = (effects->hue + 1) % 360;
            break;

        case EFFECT_THEATER_CHASE:
            ws2812b_theater_chase(effects->hue, effects->theater_frame);
            effects->theater_frame = (effects->theater_frame + 1) % 3;
            effects->hue = (effects->hue + 5) % 360;
            break;

        case EFFECT_TWINKLE:
            WS2812b_Set_Color_HSL(300, 100, 50); // Magenta pastel
            break;
    }

    WS2812_Send();
}

/**
 * @brief Manually set the current effect (disables auto-cycling).
 * @param effects Pointer to effects state.
 * @param new_effect The desired effect from @ref ws2812b_effect_t.
 * @note Clears all LEDs immediately after switching.
 */
void WS2812B_Effects_SetEffect(ws2812b_effects_t* effects, ws2812b_effect_t new_effect) {
    effects->current_effect = new_effect;
    effects->auto_cycle = false; // Manual mode
    WS2812_Clear();
}

// ==================== RAINBOW EFFECTS ====================

/**
 * @brief Display a full rainbow across all LEDs.
 * @param colorspace Color model to use: @ref COLOR_HSV (vibrant), @ref COLOR_HSL (pastel), or @ref COLOR_RGB (classic).
 * @note Uses internal `rainbow_hue` that auto-rotates.
 *       Includes built-in delay based on `global_speed`.
 */
void WS2812B_Rainbow(color_space_t colorspace) {
    switch(colorspace) {
        case COLOR_HSV:
            for (int i = 0; i < LED_NUM; i++) {
                uint16_t hue = (rainbow_hue + (i * 360 / LED_NUM)) % 360;
                ws2812b_set_pixel_hsv(i, hue, 100, global_brightness);
            }
            break;

        case COLOR_HSL:
            for (int i = 0; i < LED_NUM; i++) {
                uint16_t hue = (rainbow_hue + (i * 360 / LED_NUM)) % 360;
                WS2812b_SetPixelHSL(i, hue, 100, 50); // Pastel = L=50%
            }
            break;

        case COLOR_RGB:
            for (int i = 0; i < LED_NUM; i++) {
                uint8_t wheel_pos = (rainbow_hue + (i * 255 / LED_NUM)) % 255;
                if (wheel_pos < 85) {
                    ws2812b_set_pixel_rgb(i, 255 - wheel_pos * 3, 0, wheel_pos * 3);
                } else if (wheel_pos < 170) {
                    wheel_pos -= 85;
                    ws2812b_set_pixel_rgb(i, 0, wheel_pos * 3, 255 - wheel_pos * 3);
                } else {
                    wheel_pos -= 170;
                    ws2812b_set_pixel_rgb(i, wheel_pos * 3, 255 - wheel_pos * 3, 0);
                }
            }
            break;
    }

    rainbow_hue = (rainbow_hue + 2) % 360;
    WS2812_Send();
    HAL_Delay(100 - global_speed);
}

/**
 * @brief Rainbow with a chasing motion.
 * @param colorspace See @ref WS2812B_Rainbow.
 * @note Faster motion than standard rainbow; uses different hue spacing.
 */
void WS2812B_RainbowChase(color_space_t colorspace) {
    static uint16_t chase_offset = 0;

    for (int i = 0; i < LED_NUM; i++) {
        uint16_t led_hue = (chase_offset + i * 30) % 360;

        switch(colorspace) {
            case COLOR_HSV:
                ws2812b_set_pixel_hsv(i, led_hue, 100, global_brightness);
                break;
            case COLOR_HSL:
                WS2812b_SetPixelHSL(i, led_hue, 100, 50);
                break;
            case COLOR_RGB:
                if (led_hue < 60) {
                    ws2812b_set_pixel_rgb(i, 255, (led_hue * 255) / 60, 0);
                } else if (led_hue < 120) {
                    ws2812b_set_pixel_rgb(i, 255 - ((led_hue-60) * 255) / 60, 255, 0);
                } else if (led_hue < 180) {
                    ws2812b_set_pixel_rgb(i, 0, 255, ((led_hue-120) * 255) / 60);
                } else if (led_hue < 240) {
                    ws2812b_set_pixel_rgb(i, 0, 255 - ((led_hue-180) * 255) / 60, 255);
                } else if (led_hue < 300) {
                    ws2812b_set_pixel_rgb(i, ((led_hue-240) * 255) / 60, 0, 255);
                } else {
                    ws2812b_set_pixel_rgb(i, 255, 0, 255 - ((led_hue-300) * 255) / 60);
                }
                break;
        }
    }

    chase_offset = (chase_offset + 3) % 360;
    WS2812_Send();
    HAL_Delay(100 - global_speed);
}

// ==================== BREATHE EFFECTS ====================

/**
 * @brief Smooth breathing/pulsing effect using a single base color.
 * @param colorspace Color model to interpret the next three parameters.
 * @param hue_or_red If HSV/HSL: hue (0–359). If RGB: red (0–255).
 * @param sat_or_green If HSV/HSL: saturation (0–100%). If RGB: green (0–255).
 * @param val_or_blue If HSV/HSL: value/lightness (0–100%). If RGB: blue (0–255).
 * @note Brightness is modulated by internal `breathe_val` (10–90%).
 */
void WS2812B_Breathe(color_space_t colorspace, uint16_t hue_or_red, uint8_t sat_or_green, uint8_t val_or_blue) {
    switch(colorspace) {
        case COLOR_HSV:
            ws2812b_set_color_hsv(hue_or_red, sat_or_green, breathe_val);
            break;
        case COLOR_HSL:
            WS2812b_Set_Color_HSL(hue_or_red, sat_or_green, breathe_val);
            break;
        case COLOR_RGB: {
            uint8_t r = (hue_or_red * breathe_val) / 100;
            uint8_t g = (sat_or_green * breathe_val) / 100;
            uint8_t b = (val_or_blue * breathe_val) / 100;
            ws2812b_set_color_rgb(r, g, b);
            break;
        }
    }

    breathe_val += breathe_dir;
    if (breathe_val >= 90 || breathe_val <= 10) {
        breathe_dir = -breathe_dir;
    }

    WS2812_Send();
    HAL_Delay(150 - global_speed);
}

// ==================== SOLID COLORS ====================

/**
 * @brief Set all LEDs to a solid color in the specified color space.
 * @param colorspace See @ref color_space_t.
 * @param hue_or_red See @ref WS2812B_Breathe.
 * @param sat_or_green See @ref WS2812B_Breathe.
 * @param val_or_blue See @ref WS2812B_Breathe.
 */
void WS2812B_SolidColor(color_space_t colorspace, uint16_t hue_or_red, uint8_t sat_or_green, uint8_t val_or_blue) {
    switch(colorspace) {
        case COLOR_HSV:
            ws2812b_set_color_hsv(hue_or_red, sat_or_green, val_or_blue);
            break;
        case COLOR_HSL:
            WS2812b_Set_Color_HSL(hue_or_red, sat_or_green, val_or_blue);
            break;
        case COLOR_RGB:
            ws2812b_set_color_rgb(hue_or_red, sat_or_green, val_or_blue);
            break;
    }
    WS2812_Send();
}

// ==================== ANIMATED EFFECTS ====================

/**
 * @brief Theater chase (Knight Rider style) in selected color space.
 * @param colorspace Color model.
 * @param hue_or_red Base color (see @ref WS2812B_Breathe).
 * @param sat_or_green Saturation or green.
 * @param val_or_blue Value/lightness or blue.
 */
void WS2812B_TheaterChase(color_space_t colorspace, uint16_t hue_or_red, uint8_t sat_or_green, uint8_t val_or_blue) {
    for (int i = 0; i < LED_NUM; i++) {
        if (i % 3 == theater_frame) {
            switch(colorspace) {
                case COLOR_HSV:
                    ws2812b_set_pixel_hsv(i, hue_or_red, sat_or_green, val_or_blue);
                    break;
                case COLOR_HSL:
                    WS2812b_SetPixelHSL(i, hue_or_red, sat_or_green, val_or_blue);
                    break;
                case COLOR_RGB:
                    ws2812b_set_pixel_rgb(i, hue_or_red, sat_or_green, val_or_blue);
                    break;
            }
        } else {
            ws2812b_set_pixel_rgb(i, 0, 0, 0);
        }
    }

    theater_frame = (theater_frame + 1) % 3;
    WS2812_Send();
    HAL_Delay(200 - global_speed * 2);
}

/**
 * @brief Simulate flickering fire using random brightness on orange-red hues.
 * @note No parameters — uses internal logic.
 */
void WS2812B_Fire(void) {
    ws2812b_fire_effect();
    WS2812_Send();
    HAL_Delay(100 - global_speed);
}

/**
 * @brief Soft pastel wave using HSL (fixed S=60%, L=80%).
 * @note Rotating hue creates smooth color transition.
 */
void WS2812B_PastelWave(void) {
    ws2812b_pastel_loop(&rainbow_hue);
    WS2812_Send();
    HAL_Delay(100 - global_speed);
}

// ==================== UTILITY FUNCTIONS ====================

/**
 * @brief Turn off all LEDs.
 */
void WS2812B_Off(void) {
    WS2812_Clear();
    WS2812_Send();
}

/**
 * @brief Set global brightness for HSV-based effects.
 * @param brightness Brightness in percent (0–100). Clamped automatically.
 * @note Does not affect RGB or HSL effects directly.
 */
void WS2812B_SetBrightness(uint8_t brightness) {
    global_brightness = brightness;
    if (global_brightness > 100) global_brightness = 100;
}

/**
 * @brief Set animation speed.
 * @param speed Speed (1–100). Higher = faster animation.
 * @note Affects built-in delays in effect functions.
 */
void WS2812B_SetSpeed(uint8_t speed) {
    global_speed = speed;
    if (global_speed > 100) global_speed = 100;
    if (global_speed < 1) global_speed = 1;
}