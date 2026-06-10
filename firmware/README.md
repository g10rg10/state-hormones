# Hormy EMO Eyes — firmware (Arduino + Arduino_GFX)

A **faithful C++ port** of the web `states.html` gallery to the
**Waveshare ESP32-S3-Touch-LCD-2.8C** (2.8″ **480×480 round**, ST7701, 8 MB OPI
PSRAM, 16 MB flash). Same two glossy EMO eyes, same 16 states, the tall
vertically-elongated clock, the centered listening notepad (white pad, spiral on
the right, a **writing hand**), the **confirm check ✓** (the eyes morph into a
checkmark and back), and the **light feedback** (Maind X §0) whose level + tint
follow the state (idle breathe, speak pulse *in time with the eyes*, listen
brighter + cooler, confirm pulse, wake fade). The light is **EXTERNAL** on the
device (an RGB LED halo — the LCD shows only the eyes); the firmware exposes
`g_lightLevel` / `g_lightCool` for it. **Round-only**, no new design. The
render loop is paced ~30 fps and every timeline is driven from the wall clock with
per-segment cubic-bezier easing, so motion stays smooth and framerate-independent
(matching EMO's eased blinks + continuous micro-life).

Sketch folder: [`Hormy_eyes/`](Hormy_eyes/).

## What it does out of the box

Runs a **demo** that cycles through all 16 states like the web gallery
(idle → clock → speaking → listening+notepad → … → expressions), ~5–8 s each.
A host can instead call `setState(i)` to drive a specific state; the chain info
(`A2→A3`, `D1→B1`, `D2→B2`) lives in the state table for that use.

## Files

| file | role |
|------|------|
| `Hormy_eyes.ino` | `setup()`/`loop()`, frame pacing, demo state driver, `setState()` hook |
| `panel_config.h` | **the only file you must verify** — pins, timing, ST7701 init, TCA9554 expander, backlight |
| `palette.h` | EMO cyan colors, RGB565 pack/lerp, the eye gradient |
| `easing.h` | cubic-bezier solver + the exact timing-functions the CSS uses |
| `anim.h` | the 3-layer keyframe engine (`.eyes` group / `.eye` per-eye / `.dot`), sampling, `evalState()` |
| `timelines.h` | all 16 state keyframe tables, transcribed 1:1 from `state-keyframes.css` |
| `draw.h` | renderer: glossy gradient+gloss+glow eyes, tall 7-seg clock, notepad |

The engine + renderer are **hardware-agnostic** (they only draw through a
`Arduino_GFX*`). The 16 state tables, easings, durations and geometry are copied
verbatim from the web source, so the device matches the preview.

## Build

1. **Arduino IDE** with the **ESP32 board package**.
2. Install the **Arduino_GFX** library (moononournation/Arduino_GFX).
   - Its ESP32-S3 **RGB** driver needs **arduino-esp32 core 2.0.x**. Waveshare
     ships **3.0.2** with the Espressif panel stack — if you stay on 3.x, bring
     the panel up with their `ESP32_Display_Panel` example and keep `gfx` valid
     (the renderer is unchanged).
3. Board settings: **ESP32S3 Dev Module**, **PSRAM = OPI (8 MB) ENABLED**,
   Flash **16 MB**, 240 MHz. (Without PSRAM the framebuffer won't allocate.)
   Confirm Flash Mode / Partition / *USB CDC On Boot* on the Waveshare 2.8C page.
4. Open `Hormy_eyes/Hormy_eyes.ino`, finish the two TODOs in `panel_config.h`
   (below), flash.

## ⚠️ The two things you must fill in `panel_config.h`

This board routes the ST7701 **CS** and the **LCD/touch resets** through a
**TCA9554 I/O expander** (not direct GPIOs), so the bring-up can't be fully
hardcoded. Pins and RGB timing **are** confirmed and filled in; these two are not:

1. **ST7701 init-operations array** — paste your panel's init table (from the
   Waveshare 2.8C example or the matching Arduino_GFX `st7701_type*` table) into
   `ST7701_INIT` / `ST7701_INIT_LEN`. Without it the panel stays black.
2. **TCA9554 reset sequence** — `expanderInit()` drives EXIO0 = LCD-RST,
   EXIO1 = TP-RST, EXIO2 = ST7701-CS (addr `0x20` on I²C SDA 15 / SCL 7). The
   mapping is best-effort (EXIO0=LCD-RST is corroborated only by ESPHome) —
   verify against your example and adjust the bit values if the screen is dark.

If anything looks off, copy the panel construction + init + expander code
**verbatim** from the official Waveshare example; only `panel_config.h` changes.

## The clock

Tall vertically-elongated digits drawn as **rounded 7-segment** primitives (the
`font allungato verticale`), glossy cyan with a bloom, 24-hour `HH:MM`, colon
blinking at 1 Hz. **Time source is a placeholder** (free-running from `millis()`,
seeded near 10:08) — wiring the real RTC/NTP is the *next* step: just replace the
body of `getTime()` in `draw.h`, the signature stays the same.

## Hardware checklist (verify on the device)

- [ ] ST7701 init array + TCA9554 expander filled in `panel_config.h`
- [ ] PSRAM = OPI enabled (framebuffer allocates)
- [ ] RGB timing matches the example (18 MHz default; if it shears/flickers, the example's values win)
- [ ] Eyes centered & symmetric within the circle (`CX_L=150, CX_R=330, CY=240`)
- [ ] Glow halo + notepad far corner not clipped by the round bezel (inset if needed)
- [ ] Backlight on (GPIO 6)

Touch (GT911 @ `0x5D`) is **not** needed for this firmware.
