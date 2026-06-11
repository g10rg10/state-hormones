# EMO / Maind X Eyes — eye animation galleries (480×480)

Two preview galleries, all **pure HTML + CSS + JS**, zero dependencies,
designed to run even on embedded displays:

- **`index.html` — Appear**: the 9 animations the EMO eyes can appear with.
- **`states.html` — States**: the states from the *Maind X Healthcare* spec
  ([docs/maind-x-eyes-animation.md](docs/maind-x-eyes-animation.md)) — plus a
  clock (tall vertical digits) and a few height-only expressions — the same
  glossy EMO eyes as the Appear gallery, all expression carried by rhythm.
  **Round-only** (the hardware screen is round).
- **`firmware/` — device port**: a faithful C++ / Arduino_GFX port of the States
  gallery for the **Waveshare ESP32-S3 2.8″ 480×480 round** display. All 13
  states + the tall clock, ~30 fps eased motion. See
  [firmware/README.md](firmware/README.md).
- **[docs/animations-sounds-lights.md](docs/animations-sounds-lights.md)** —
  single reference for every **animation, sound and light** (kept in sync).

## Run

```bash
python3 -m http.server 8123
# then open http://localhost:8123 (appear) or http://localhost:8123/states.html (states)
```

---

## Gallery 1 — Appear (`index.html`)

- **Featured** stage at real size **480×480**, looping.
- **Grid** with all 9 variants, each looping. Click one to feature it.
- Controls:
  - **Appear speed** — 150–800 ms slider (duration of the appear move only).
  - **Square / Round** — rounded-rectangle or circular screen.
  - **Blink** — idle micro-blink mid-cycle to keep it alive.
  - **Replay** — resyncs all animations.

### The 9 variants

| id | effect |
|----|--------|
| `pop` | springs up from tiny with an elastic bounce |
| `eye-open` | opens from a thin horizontal slit (waking up) |
| `fade-scale` | fade + slight scale-up (the calmest) |
| `slide-in` | the two eyes slide in from the edges |
| `power-on` | LED/CRT power-on: white pinpoint → flash → full |
| `drop-bounce` | drops from above with a bounce |
| `iris-expand` | iris-style opening: wide-flat first, then full height |
| `zoom-rush` | rushes in from afar, blurred → sharp |
| `wipe-up` | bottom-to-top reveal (clip-path) |

### Structure / extracting one for firmware

- `index.html` — gallery page.
- `eyes.css` — screen + eyes look + blink + UI (the **base**).
- `keyframes.css` — the 9 animations: each one a `@keyframes ap-<id>` + `.anim-<id> .eye` selector.
- `anims.js` — variant list (id, name, description, ms).
- `eyes.js` — preview controller.

All variants share the same contract, so they can be composed and swapped:

```
DOM:   .stage > .eyes > .eye.left + .eye.right
cycle: 0% hidden → ~16% fully visible (transform:none) →
       16–84% hold → 84–92% fade out → 92–100% hidden
props: transform / opacity / filter only (clip-path only for the "wipes")
```

To use **a single one** on the device: take the eye base from `eyes.css`, the chosen
variant block from `keyframes.css`, and set the `anim-<id>` class on the `.stage`.
Speed is driven by the CSS variable `--loop` (appear duration ≈ `--loop` × 0.16).

---

## Gallery 2 — Maind X states (`states.html`)

Implements the spec [docs/maind-x-eyes-animation.md](docs/maind-x-eyes-animation.md) 1:1,
reusing the **glossy EMO eyes** from the Appear gallery (rounded-rectangle, bloom);
all expression lives in rhythm — blink, breath, size, position.

Controls: **Tempo** (global `--spd` duration multiplier), **Square/Round**,
**EMO cyan / Warm white** (tint, cyan by default), **Replay** (resyncs everything),
**Sound** (mute toggle, on by default — see below).

### The states (id = firmware clip name)

| id | type | what it does |
|----|------|--------------|
| `A1_off` | static | dark screen, dots off |
| `A2_wake` | one-shot ~700 ms | fade-in + one slow waking blink |
| `A3_idle` | loop | ±8% breath (~4.6 s) + natural blink every ~5–8 s |
| `A4_clock` | display | the eyes give way to a live glowing digital clock (HH:MM, blinking colon) |
| `B1_speaking` | loop | breath at speech rhythm (simulated TTS envelope) + blink ~6 s |
| `B2_listening` | loop | dots +12%, held still, rare blink — your turn; a centered notepad + hand is always writing (overlay, eyes untouched) |
| `B3_thinking` | loop ~2 s | gaze slips away and back + one slow half blink |
| `C1_confirm` | one-shot ~1.9 s | the eyes morph into a check ✓, hold, then return to eyes |
| `C2_didnt_catch` | one-shot ~600 ms | slow blink + tilt (one dot up, one down), then back |
| `D1_reminder` | one-shot ~1.9 s | the eyes morph into a little bell that rings, then return to eyes |
| `D2_wakeword` | one-shot ~400 ms | attentive pop, settles at the listening size (B2) |
| `E2_quirk` | one-shot ~1 s | looks around and back, natural double blink |
| `E4_wink` | one-shot ~2.6 s | right eye flattens to a line and holds shut, left squints, then both reopen |
> The personality states kept are **E2 quirk** (look-around) and **E4 wink** —
> life from motion + height only, no extra parts. The overlays that add an element
> are the **centered notepad** (listening), the **clock**, the **confirm check ✓**
> and the **reminder bell** — each in its own region, never a "nose".

### States contract (3 composable layers)

```
DOM:    .stage.st-<ID> > .eyes > .eye.left/.right > .dot
layers: .eyes = group (breath, drift, swirl, fade)
        .eye  = per-eye offsets (tilt, left/right out-of-phase wobble) + bloom
        .dot  = the glossy EMO eye: blink (scaleY → thin line), dim/brightness
loops:  A3, B1, B2, B3 are seamless (0% == 100%)
one-shots: the action sits at the start of the cycle (window noted in the
        comments of state-keyframes.css), then hold at rest and reset — on
        the device use only the action window and chain (A2→A3, D1→B1, D2→B2…)
props:  transform / opacity / filter only; durations scale with --spd
```

Files: `states.html` (page) · `states.css` (eye base + UI) ·
`state-keyframes.css` (the states) · `state-anims.js` (metadata) ·
`sound.js` (sound design) · `states.js` (controller).

### Sound design (`sound.js`)

A coherent audio identity for every state, **synthesized live with the Web Audio
API** — no audio files, no dependencies, so it can run on the same embedded target
as the eyes. One timbral family for all states: soft sine/triangle voices, a warm
**D-pentatonic** palette, a gentle lowpass + a short reverb tail. Calm by design
(spec: *never alarming, never hypnotic*). It plays on **select / replay** (a user
gesture is required to start audio in browsers); loop states keep a low bed going
until you leave them.

| state | sound |
|-------|-------|
| `A1_off` | soft descending power-down |
| `A2_wake` | gentle rising arpeggio (D→F#→A) + shimmer |
| `A3_idle` | (silent — no sound) |
| `A4_clock` | **loop**: soft tick / tock at ~1 Hz |
| `B1_speaking` | (silent — no sound) |
| `B2_listening` | (silent — no sound) |
| `B3_thinking` | **loop**: sparse contemplative blips |
| `C1_confirm` | bright positive two-note (A→D) |
| `C2_didnt_catch` | questioning upward "hm?" (E→A↗B) |
| `D1_reminder` | gentle alarm-bell ring (soft, repeated strikes) |
| `D2_wakeword` | attentive rising pop (D→A→D) |
| `E2_quirk` | (silent — no sound) |
| `E4_wink` | playful upward blip |
> On the device, the same amplitude envelope drives **eyes + light + audio**
> together (spec §production notes); here each state restarts on select/replay so
> motion and sound stay in sync.

### Light feedback (Maind X spec §0 · Light states)

A third feedback layer beyond motion and sound: an **external light halo** around
the device — a "shadow of light" (inside the screen there are only the eyes) —
whose **level + tint** follow the state. Brightness mimics the audio-envelope floor
(never 0); the guardrail is *never bouncy* (anti-hyperfocus). The **speaking** light
pulses **in time with the eyes**. Per state (`lightDesc` in `state-anims.js`, the
`.stage` outer glow in the CSS; on the device it drives an external RGB LED —
`g_lightLevel` in the firmware):

| light | states | behavior |
|-------|--------|----------|
| **idle** | A3, A4, B3, E2, E4 | slow breathe around a low floor (not audio-driven) |
| **speak** | B1, C2, D1 | pulses with the speech envelope — floor 0.55, fast attack / slow release |
| **listen** | B2, D2 | brighter floor (0.65) + **cooler tint** → "your turn" |
| **wake** | A2 | fade-in from off, settle to floor |
| **confirm** | C1 | single brightness pulse |
| **off** | A1 | dark (the only state with light fully off) |

The **listening** overlay also shows a **centered** white **notepad** (spiral
binding on the right) with a **hand writing** on it — "I'm taking notes" — without
touching the eyes. And **confirm** (C1) morphs the eyes into a **check ✓** and back.
Both mirrored in the firmware (`drawNotepad`, `drawCheck`).
