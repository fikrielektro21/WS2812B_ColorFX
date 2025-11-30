# WS2812B Color Space Converter for STM32 Blue Pill


Real-time **RGB**, **HSV**, and **HSL** color control for WS2812B LED strips — running natively on **STM32F103C8T6 (Blue Pill)** with **zero external libraries** and **integer-only math**.

Perfect for embedded projects where floating-point is unavailable or inefficient.

![Demo: HSV vs HSL comparison](assets/hsl-hsv-comparison.gif)
> _Screenshot: Left = HSV (vibrant), Right = HSL (pastel)_

> 💡 **Why HSL?** HSV gives vibrant colors, while HSL produces soft pastels — ideal for ambient lighting, mood indicators, and artistic displays.

---

## ✨ Features

- ✅ **Full RGB ↔ HSV ↔ HSL conversion** in pure C (no `float` or `math.h`)
- ✅ **Per-pixel or global color control** in any color space
- ✅ **Hardware-accelerated** via **PWM + DMA** (non-blocking, timing-perfect)
- ✅ Built-in animated effects:
  - Rainbow (RGB/HSV/HSL)
  - Smooth breathe
  - Fire flicker
  - Theater chase
  - Pastel wave (HSL-only)
- ✅ **No dependency** on FastLED, Adafruit NeoPixel, or Arduino
- ✅ Ready for **real-time applications** (e.g., sensor-driven color feedback)

---

## 🛠️ Hardware Requirements

| Component | Notes |
|---------|------|
| **STM32 Blue Pill** | STM32F103C8T6 (72 MHz Cortex-M3) |
| **WS2812B LED Strip** | Any length (tested with 8–60 LEDs) |
| **Level Shifter (Optional)** | Recommended for >10 LEDs (3.3V → 5V) |
| **Power Supply** | External 5V for LED strip (do not power from Blue Pill!) |

> 🔌 **Pinout**:  
> - WS2812B **DATA IN** → **PA6** (TIM3_CH1)  
> - Shared **GND** between Blue Pill and LED strip

---

## 💻 Software Requirements

- **IDE**: STM32CubeIDE (v1.15+)
- **HAL**: STM32CubeF1 (v1.8.6+)
- **Clock**: 72 MHz (HSE + PLL)
- **Timer**: TIM3 in PWM mode
- **DMA**: Channel for TIM3_CH1

> 📦 **No external libraries** — everything is self-contained!

---

## 📂 Project Structure
