/**
 * @file WS2812B_Effects.h
 * @brief High-level animated LED effects with multi-colorspace support (RGB, HSV, HSL).
 *
 * Provides a stateful effect manager with auto-cycling, breathing, rainbow,
 * theater chase, fire, and more — all configurable via color space.
 */

#ifndef SRC_WS2812B_EFFECTS_H_
#define SRC_WS2812B_EFFECTS_H_

#include "WS2812B.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Available built-in animation effects.
 */
typedef enum {
    EFFECT_STATIC_COLOR,    ///< Rotating solid HSV color
    EFFECT_RAINBOW_CHASE,   ///< Moving rainbow pattern
    EFFECT_FIRE,            ///< Flickering fire simulation
    EFFECT_BREATHE,         ///< Smooth brightness pulse
    EFFECT_THEATER_CHASE,   ///< "Knight Rider" style chase
    EFFECT_TWINKLE          ///< Random star-like twinkles
} ws2812b_effect_t;

/**
 * @brief Effect configuration and state structure.
 */
typedef struct {
    ws2812b_effect_t current_effect;  ///< Currently active effect
    uint16_t hue;                     ///< Base hue for dynamic effects (0–359)
    uint8_t brightness;               ///< Global brightness (0–100%)
    int8_t breathe_direction;         ///< Breathing direction (+1 or -1)
    uint8_t theater_frame;            ///< Current theater chase frame (0–2)
    uint32_t effect_speed;            ///< Speed level (1–100, higher = faster)
    bool auto_cycle;                  ///< Enable automatic effect rotation
    uint32_t cycle_duration;          ///< Time per effect in milliseconds
} ws2812b_effects_t;

// ===================================================================
// ====================== EFFECT MANAGEMENT API ======================
// ===================================================================

/**
 * @brief Initialize the effects state machine with default values.
 * @param effects Pointer to the effect configuration structure.
 */
void WS2812B_Effects_Init(ws2812b_effects_t* effects);

/**
 * @brief Execute the current effect (call in main loop).
 * @param effects Pointer to the current effect state.
 * @note Automatically cycles effects if auto_cycle is enabled.
 */
void WS2812B_Effects_Handle(ws2812b_effects_t* effects);

/**
 * @brief Manually switch to a new effect (disables auto-cycle).
 * @param effects Pointer to the effect state.
 * @param new_effect The desired effect from @ref ws2812b_effect_t.
 */
void WS2812B_Effects_SetEffect(ws2812b_effects_t* effects, ws2812b_effect_t new_effect);

// ===================================================================
// ======================= COLOR SPACE SUPPORT =======================
// ===================================================================

/**
 * @brief Color space selection for effects.
 */
typedef enum {
    COLOR_HSV,      ///< Hue, Saturation, Value (vibrant)
    COLOR_HSL,      ///< Hue, Saturation, Lightness (pastel-friendly)
    COLOR_RGB       ///< Red, Green, Blue (classic)
} color_space_t;

// ===================================================================
// ========================== EFFECTS API ============================
// ===================================================================

/**
 * @brief Display a full rainbow across all LEDs.
 * @param colorspace Color model to use (@ref COLOR_HSV, @ref COLOR_HSL, or @ref COLOR_RGB).
 */
void WS2812B_Rainbow(color_space_t colorspace);

/**
 * @brief Rainbow with a chasing motion.
 * @param colorspace Color space to use.
 */
void WS2812B_RainbowChase(color_space_t colorspace);

/**
 * @brief Smooth breathing/pulsing effect.
 * @param colorspace Color space.
 * @param hue_or_red If HSV/HSL: hue (0–359). If RGB: red (0–255).
 * @param sat_or_green If HSV/HSL: saturation (0–100%). If RGB: green (0–255).
 * @param val_or_blue If HSV/HSL: value/lightness (0–100%). If RGB: blue (0–255).
 */
void WS2812B_Breathe(color_space_t colorspace, uint16_t hue_or_red, uint8_t sat_or_green, uint8_t val_or_blue);

/**
 * @brief Set all LEDs to a solid color.
 * @param colorspace Color space.
 * @param hue_or_red See @ref WS2812B_Breathe.
 * @param sat_or_green See @ref WS2812B_Breathe.
 * @param val_or_blue See @ref WS2812B_Breathe.
 */
void WS2812B_SolidColor(color_space_t colorspace, uint16_t hue_or_red, uint8_t sat_or_green, uint8_t val_or_blue);

/**
 * @brief Theater chase ("Knight Rider") effect.
 * @param colorspace Color space.
 * @param hue_or_red Base color (see @ref WS2812B_Breathe).
 * @param sat_or_green Saturation or green.
 * @param val_or_blue Value/lightness or blue.
 */
void WS2812B_TheaterChase(color_space_t colorspace, uint16_t hue_or_red, uint8_t sat_or_green, uint8_t val_or_blue);

/**
 * @brief Simulate flickering fire using random brightness on orange hues.
 */
void WS2812B_Fire(void);

/**
 * @brief Smooth pastel wave using HSL (S=60%, L=80%).
 */
void WS2812B_PastelWave(void);

// ===================================================================
// =========================== UTILITIES =============================
// ===================================================================

/**
 * @brief Turn off all LEDs.
 */
void WS2812B_Off(void);

/**
 * @brief Set global brightness for HSV-based effects.
 * @param brightness Brightness in percent (0–100).
 */
void WS2812B_SetBrightness(uint8_t brightness);

/**
 * @brief Set animation speed (affects built-in delays).
 * @param speed Speed level (1–100). Higher = faster.
 */
void WS2812B_SetSpeed(uint8_t speed);

#endif /* SRC_WS2812B_EFFECTS_H_ */