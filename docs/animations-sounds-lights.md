# EMO / Maind X — Animations, Sounds & Lights

Single reference for **every animation, sound and light** in the project. Keep it
**in sync** with the code whenever any of them changes:
- States + firmware: `state-keyframes.css`, `state-anims.js`, `sound.js`, `firmware/Hormy_eyes/`
- Appear gallery: `keyframes.css`, `anims.js`

Each state carries up to three feedback layers:
- **Motion** — the two glossy rounded-square EMO eyes; expression by rhythm + height (`scaleY`), never a nose.
- **Sound** — `sound.js`, synthesized live (Web Audio, no files), warm **D-pentatonic**, calm by design.
- **Light** — Maind X spec §0: an **EXTERNAL** halo around the device (on the firmware it drives an RGB LED via `g_lightLevel` / `g_lightCool`); inside the screen there are only the eyes.

---

## Device lifecycle (logic)

```
Power on ──[A2_wake]──▶ Idle / face (A3_idle) ──5 min idle──▶ Clock (A4_clock)
   │                          ▲                                    │
   │                          └─────────── wake word ◀────────────┘
   └─ (wake word) ─▶ Wake word (D2) ─▶ Listening (B2) ─▶ Idle / face
```

- **Power on** → `A2_wake` → settles into **`A3_idle`** (face mode).
- **5 min of inactivity** in idle → **`A4_clock`** (clock "on the shelf").
- **Wake word** ("Hey Arduino"), from clock *or* idle → `D2_wakeword` →
  `B2_listening` → back to **`A3_idle`** (face), resetting the 5-min timer.

The **States page** has a **▶ Simulate flow** button that runs this end-to-end —
timings compressed (5 min ≈ a few seconds); press **🎤 "Hey Arduino"** to wake,
or the clock auto-wakes so the loop repeats. The firmware runs the same logic in
`Hormy_eyes.ino` (real 5-min timeout; the wake word is a hook for the mic).

## States gallery (`states.html` + `firmware/`) — 13 states

| ID | Animation | Sound | Light |
|----|-----------|-------|-------|
| `A1_off` | dark screen, dots off (static) | soft descending power-down | **off** (dark) |
| `A2_wake` | fade-in + one slow waking blink → chains to A3 | gentle rising arpeggio (D→F#→A) + shimmer | **wake**: fade-in, settle to floor |
| `A3_idle` | ±8% breath (~4.6 s) + look-around drift (~7.2 s) + natural blink (~5–8 s) | — *(silent)* | **idle**: slow breathe |
| `A4_clock` | eyes give way to a tall rounded-digit clock `HH:MM`, colon blinks 1 Hz | soft tick / tock ~1 Hz (loop) | **idle** breathe |
| `B1_speaking` | breath at speech rhythm (~3.4 s) + blink (~6 s) + tiny drift | — *(silent)* | **speak**: pulses *in time with the eyes* |
| `B2_listening` | dots +12 %, held still, rare blink (~9 s) + **centered notepad with a writing hand** | — *(silent)* | **listen**: brighter floor + cooler tint |
| `B3_thinking` | gaze slips away and back (~2 s) + slow half-blink | sparse contemplative blips (loop) | **idle** breathe |
| `C1_confirm` | the eyes **morph into a check ✓**, hold, then return to eyes | bright positive two-note (A→D) | **confirm**: single pulse |
| `C2_didnt_catch` | slow blink + gentle tilt (one dot up, one down) | questioning upward "hm?" (E→A↗B) | **speak** pulse → cools toward listen |
| `D2_wakeword` | attentive pop, settles at the listening size (B2 pose) | attentive rising pop (D→A→D) | **listen** (cooler) |
| `E2_quirk` | looks around (left, right) and back + one natural blink | — *(silent)* | **idle** |
| `E4_wink` | right eye flattens to a line and holds, left squints, both reopen | playful upward blip | **idle** |

**Overlays / morphs:** `A4_clock` clock face · `B2_listening` notepad + writing hand · `C1_confirm` check ✓.

---

## Light feedback (Maind X spec §0 · Light states)

External halo; brightness follows the audio envelope (floor never 0); guardrail = *never bouncy* (anti-hyperfocus).

| Light | States | Behavior |
|-------|--------|----------|
| **idle** | A3, A4, B3, E2, E4 | slow breathe around a low floor (not audio-driven) |
| **speak** | B1, C2 | pulses with the speech envelope — floor 0.55, fast attack / slow release; B1 is timed to the eye breath |
| **listen** | B2, D2 | brighter floor (0.65) + cooler tint → "your turn" |
| **wake** | A2 | fade-in from off, settle to floor |
| **confirm** | C1 | single brightness pulse |
| **off** | A1 | dark (the only state with light fully off) |

---

## Sound (`sound.js`)

Synthesized live with the Web Audio API — no audio files, no dependencies (runs on
the same embedded target). One timbral family: soft sine/triangle voices, warm
**D-pentatonic** palette, gentle lowpass + a short reverb tail. Plays on
select / replay (a user gesture is required to start audio in browsers); loop
beds (`A4_clock`, `B3_thinking`) keep going until you leave the state.

**Silent states (no sound):** `A3_idle`, `B1_speaking`, `B2_listening`, `E2_quirk`.

---

## Appear gallery (`index.html`) — 9 appear animations

How the eyes can first appear (each ~150–800 ms, set by the speed slider).

| ID | Animation |
|----|-----------|
| `pop` | springs up from tiny with an elastic bounce |
| `eye-open` | opens from a thin horizontal slit (waking up) |
| `fade-scale` | fade + slight scale-up (the calmest) |
| `slide-in` | the two eyes slide in from the edges |
| `power-on` | LED/CRT power-on: white pinpoint → flash → full |
| `drop-bounce` | drops from above with a bounce |
| `iris-expand` | iris-style opening: wide-flat first, then full height |
| `zoom-rush` | rushes in from afar, blurred → sharp |
| `wipe-up` | bottom-to-top reveal (clip-path) |
