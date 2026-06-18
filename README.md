# Maind X — eyes & face

> Repo: **`state-hormones`**

Pure **HTML + CSS + JS** (zero dependencies) animated **face** for the **Maind X
Healthcare** offline voice companion — designed to run on a round **480×480**
embedded display — plus a faithful **C++ / Arduino_GFX** firmware port. The
browser page previews every device state (idle, listening, thinking, speaking,
confirm, clock, wink…) as two glowing eyes plus an animated mouth, and the
firmware renders the same states 1:1 on a Waveshare ESP32-S3 round LCD. The
look is **flat** (no 3D shading), and the companion is woken with the wake word
**"Hey Kai"**.

---

## Live demo

The site is deployed on Vercel and auto-updates on every push to `main`.

**→ [state-hormones.vercel.app](https://state-hormones.vercel.app)**

The root URL opens the face directly (`/` = `index.html`).

---

## Quick start

No build step and no dependencies — just serve the folder statically. From the
repo root:

```bash
python3 -m http.server 8123
# then open http://localhost:8123/
```

The page uses the **Web Speech API** (TTS) and **SpeechRecognition** (mic
wake-word detection); browser support and microphone permissions vary. Sound is
synthesized live via **Web Audio** — the sound control is disabled and labelled
"🔇 n/d" where Web Audio is unavailable.

---

## The page

`index.html` — *Maind X — face (all states)*. A large flat round stage shows the
featured state (eyes + animated mouth); a grid of cards lets you feature any
state, voicing its sound and showing its **Light** and **Sound** descriptions.
A live HH:MM clock ticks at 1 Hz. Driven by `face.js`, `state-anims.js`,
`sound.js`.

**Controls (header)**
- **Power toggle** (default "Active") — the physical activate/deactivate switch.
  **Active** wakes the face and turns the mic ON (listens for "Hey Kai");
  **Deactivated** drops to the clock and turns the mic OFF.
- **🔊 Sound** — audio on/off (also drives the speaking TTS).
- **▶ Simulate flow** — a scripted lifecycle demo: clock → switch on → wake →
  idle → "Hey Kai" → wake-word → listening → thinking → confirm → speaking →
  idle → switch off → clock, looping.
- **🎤 "Hey Kai"** — manual wake trigger, active only during the flow.

---

## Face states

The state set shown on the page (sourced 1:1 from `state-anims.js`). The wake
word is **"Hey Kai"**; `Wake word` is an attentive pop that then waits up to ~3 s
for speech before falling back to Idle.

| State | Type | Light behaviour | Sound |
|-------|------|-----------------|-------|
| Wake | one-shot | Fade-in from off, neutral light, settles to the floor | Gentle rising arpeggio + shimmer |
| Idle / waiting | loop | Slow breathe around the floor — not audio-driven (~4–5 s) | silent |
| Clock | display | Light off — a quiet, dark bedside clock (deactivated); mic also off | silent |
| Speaking / TTS | loop | Pulses with the TTS envelope (fast attack / slow release) | the spoken reply (TTS) |
| Listening | loop | Brighter floor + cooler tint — "your turn" | silent |
| Thinking / processing | loop | Idle floor, slow breathe (processing) | Sparse contemplative blips (loop) |
| Confirm | one-shot | Single brightness pulse — saved / accepted | Bright positive two-note |
| Didn't catch that | one-shot | Speak pulse, then cools toward the listening floor | Questioning upward "hm?" |
| Wake word | one-shot | Lifts and cools to the listening floor — "your turn" | Attentive rising pop |
| Idle micro-quirk | one-shot · rare | Idle floor | silent |
| Wink | one-shot | Idle floor ("playful") | Playful upward blip |

> The `Off` state exists in the data but is excluded from the grid — it is
> represented by the power switch.

---

## Animation / CSS architecture

Motion is **CSS-only**, driven by a JS **data manifest** (`state-anims.js`,
pure data — no animation logic). The CSS splits into a flat **base look** plus
two **keyframes** files:

| File | Role |
|------|------|
| `face.css` | Base look — the flat 480×480 stage, the eye DOM and the mouth DOM. |
| `state-keyframes.css` | Per-state eye / overlay / ambient-light animations (`.st-<id>`). |
| `face-keyframes.css` | Per-state **mouth** morphs + eye/group overrides (loaded last). |

The **3-layer eye contract** keeps animations from fighting over one transform:

- **`.eyes`** — group transforms (breath, drift, fade)
- **`.eye`** — per-eye offsets (tilt, wobble); carries `.left` / `.right`
- **`.dot`** — the eye itself (blink → `scaleY`, dim → `filter brightness`)

The **mouth** is two layers in the same box: a stroked SVG arc (`.smile`) and a
filled ellipse (`.open`) for the talking mouth. The whole look is **flat** — the
stage is a single uniform fill (no sphere gradient, sheen or specular
highlight); the eyes and mouth are solid fills. The only on-screen glow is the
functional ambient light-feedback ring.

### Spec ids — the spine
Each state has a stable id (`A1_off … E4_wink`) that is the data `id`, the CSS
selector `.st-<id>`, **and** the firmware clip name, all 1:1 — so a single state
can be lifted from the web into the firmware deterministically.

---

## Firmware

A faithful C++ / Arduino_GFX port that renders the same flat eyes + mouth and
the same state timelines on hardware. Full detail in
**[firmware/README.md](firmware/README.md)**.

### Hardware target

| Attribute | Value |
|-----------|-------|
| Board | Waveshare **ESP32-S3-Touch-LCD-2.8C** |
| MCU memory | 8 MB OPI PSRAM, 16 MB flash |
| Display | 2.8″, **ST7701** controller, **RGB** interface |
| Resolution | **480×480**, round |
| Backlight | GPIO 6, active high |
| Touch | GT911 @ `0x5D` (present but **not used**) |
| I/O expander | TCA9554 @ `0x20` (I²C SDA 15 / SCL 7) — routes ST7701 CS / LCD-RST / TP-RST |

> "Light feedback" is an **external** RGB LED halo on the device; the LCD shows
> only the eyes + mouth. The firmware exposes `g_lightLevel` / `g_lightCool`;
> wiring the LED itself is a TODO. Renders at **~30 fps**, all timelines
> wall-clock driven with per-segment cubic-bezier easing (framerate-independent).

### Build / flash

1. **Arduino IDE** with the **ESP32 board package**.
2. Install the **Arduino_GFX** library (`moononournation/Arduino_GFX`). Its
   ESP32-S3 **RGB** driver needs **arduino-esp32 core 2.0.x** (see the firmware
   README for the 3.x path).
3. **Board settings:** ESP32S3 Dev Module, **PSRAM = OPI (8 MB) ENABLED**, Flash
   16 MB, 240 MHz. (Without PSRAM the framebuffer won't allocate.)
4. Open `firmware/Hormy_eyes/Hormy_eyes.ino`, finish the **two TODOs in
   `panel_config.h`** (ST7701 init array + TCA9554 reset sequence), then flash.

### Firmware files

| File | Role |
|------|------|
| `Hormy_eyes.ino` | `setup()` / `loop()`, frame pacing, lifecycle/demo driver, host hooks |
| `panel_config.h` | **The only file you must verify against your board** — pins, init, expander, backlight; defines `gfx` |
| `palette.h` | colors + RGB565 pack/lerp/mul helpers, eye fill + clock gradient |
| `easing.h` | cubic-bezier solver + the named timing-functions the CSS uses |
| `anim.h` | the 3-layer keyframe engine + the mouth channels; `evalState()` → `Pose` |
| `timelines.h` | all state keyframe tables (eyes, overlays **and mouth**), transcribed 1:1 from the CSS, plus `STATES[]` |
| `draw.h` | the renderer: flat eyes, animated mouth, 7-segment clock, listening notepad, confirm check ✓, reminder bell |

---

## Docs

| File | What it covers |
|------|----------------|
| [docs/device-flow.md](docs/device-flow.md) | The end-to-end device interaction flow driven by the activate/deactivate switch. |
| [docs/animations-sounds-lights.md](docs/animations-sounds-lights.md) | The single reference for every animation, sound and light per state. |
| [docs/maind-x-eyes-animation.md](docs/maind-x-eyes-animation.md) | The Maind X Healthcare eyes-animation spec the page + firmware implement 1:1. |
| [docs/status-behaviours.md](docs/status-behaviours.md) | The per-state behaviour reference. |

---

## Project layout

```text
state-hormones/
├── index.html              # the face — eyes + animated mouth, all states (site root)
│
├── state-anims.js          # window.STATES / STATE_SECTIONS — the state data manifest
├── face.js                 # page logic (idle look-around, TTS, wake word, flow demo)
├── sound.js                # live Web Audio sound bed
│
├── face.css                # base look — flat stage + eye/mouth DOM
├── state-keyframes.css     # per-state eye / overlay / ambient-light animations
├── face-keyframes.css      # per-state mouth morphs + overrides (loaded last)
│
├── docs/
│   ├── device-flow.md
│   ├── animations-sounds-lights.md
│   ├── maind-x-eyes-animation.md
│   └── status-behaviours.md
│
└── firmware/
    ├── README.md
    └── Hormy_eyes/
        ├── Hormy_eyes.ino  # setup/loop, lifecycle driver, host hooks
        ├── panel_config.h  # board-specific pins/init (verify this)
        ├── palette.h       # colors + RGB565 helpers
        ├── easing.h        # cubic-bezier easings
        ├── anim.h          # 3-layer keyframe engine + mouth channels
        ├── timelines.h     # state keyframe tables (eyes + mouth) + STATES[]
        └── draw.h          # renderer (flat eyes + mouth)
```

---

## Repo & deploy

- **GitHub:** https://github.com/g10rg10/state-hormones
- **Vercel project:** `state-hormones` — the live public site, GitHub-linked with
  **auto-deploy on push to `main`**.
- **Live URL:** https://state-hormones.vercel.app (`/` = the face)
