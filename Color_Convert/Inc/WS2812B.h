/**
 * @file WS2812B.h
 * @brief Driver untuk mengontrol strip LED WS2812B menggunakan STM32 Blue Pill.
 *
 * Mendukung pengaturan warna melalui RGB, HSV, dan HSL, serta berbagai efek animasi.
 * Menggunakan Timer3 + DMA untuk pengiriman data bitstream.
 *
 * @author Your Name
 * @date Nov 30, 2025
 */

#ifndef WS2812B_H
#define WS2812B_H

#include "main.h"
#include <stdint.h>

#define LED_NUM                8
#define WS2812B_LED_NUM        LED_NUM
#define WS2812B_DATA_SIZE      (24 * WS2812B_LED_NUM)

// === Fungsi Dasar (RGB) ===

/**
 * @brief Mengirim data PWM ke strip WS2812B melalui DMA.
 */
void WS2812B_Send(void);

/**
 * @brief Mengatur semua LED ke mati (warna hitam).
 */
void WS2812B_Clear(void);

/**
 * @brief Mengatur satu piksel LED dengan nilai RGB.
 * @param pixel Indeks LED (0 hingga LED_NUM - 1)
 * @param red   Komponen merah (0–255)
 * @param green Komponen hijau (0–255)
 * @param blue  Komponen biru (0–255)
 */
void WS2812B_SetPixelRGB(uint16_t pixel, uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Mengatur semua LED dengan warna RGB yang sama.
 * @param red   Komponen merah (0–255)
 * @param green Komponen hijau (0–255)
 * @param blue  Komponen biru (0–255)
 */
void WS2812B_SetColorRGB(uint8_t red, uint8_t green, uint8_t blue);

// === HSV (Hue, Saturation, Value) ===

/**
 * @brief Mengatur semua LED dengan warna HSV.
 * @param hue   Hue (0–359 derajat)
 * @param sat   Saturasi (0–100%, 0 = grayscale)
 * @param val   Nilai/brightness (0–100%)
 */
void WS2812B_SetColorHSV(uint16_t hue, uint8_t sat, uint8_t val);

/**
 * @brief Mengatur satu LED dengan warna HSV.
 * @param pixel Indeks LED
 * @param hue   Hue (0–359)
 * @param sat   Saturasi (0–100%)
 * @param val   Nilai (0–100%)
 */
void WS2812B_SetPixelHSV(uint16_t pixel, uint16_t hue, uint8_t sat, uint8_t val);

// === HSL (Hue, Saturation, Lightness) ===

/**
 * @brief Mengatur semua LED dengan warna HSL.
 * @param hue     Hue (0–359)
 * @param sat     Saturasi (0–100%)
 * @param light   Kecerahan (0–100%, 50% = netral)
 */
void WS2812B_SetColorHSL(uint16_t hue, uint8_t sat, uint8_t light);

/**
 * @brief Mengatur satu LED dengan warna HSL.
 * @param pixel   Indeks LED
 * @param hue     Hue (0–359)
 * @param sat     Saturasi (0–100%)
 * @param light   Kecerahan (0–100%)
 */
void WS2812B_SetPixelHSL(uint16_t pixel, uint16_t hue, uint8_t sat, uint8_t light);

// === Efek Dasar (Legacy - bisa diganti dengan Effects API) ===

void WS2812B_RainbowClassic(void);
void WS2812B_FireEffect(void);
void WS2812B_PastelLoop(uint16_t *hue);
void WS2812B_MonochromeBreathe(uint16_t hue, uint8_t *brightness, int8_t *direction);
void WS2812B_TheaterChaseSimple(uint16_t hue, uint8_t frame);

#endif /* WS2812B_H */