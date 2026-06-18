# Maind X — Status behaviours

How the device behaves in each status (state). Every status is one of three kinds:

- **display** — a screen the device sits on (clock).
- **loop** — a continuous living state it stays in (idle, listening, speaking, thinking).
- **one-shot** — a brief reaction that plays once, then chains back to a loop.

Each status has three layers of feedback: **eyes/face motion**, **sound**, and **external
light** (an ambient halo around the device; level 0 = light off). The device is gated by
a physical **activate / deactivate switch** (see `device-flow.md`):

- **Deactivated** → only the **Clock** (`A4_clock`); microphone OFF, light OFF.
- **Activated** → the **face** is live; everything below from `A2_wake` onward is reachable.

---

## A · System & power

### `A1_off` — Off (unplugged / powered down)
- **When:** the device is fully off (not plugged in / hard power-off). *Not* the same as
  Deactivated — deactivating only drops to the Clock, it does not power the device down.
- **Behaviour:** dark screen, no dots, no motion (static).
- **Sound:** a soft descending power-down chime as it goes off.
- **Light:** fully off — the only state that goes truly dark besides the clock.

### `A2_wake` — Wake (one-shot)
- **When:** the device is **activated** (switch on) or powered up.
- **Behaviour:** the face fades in from nothing (opacity 0→100), gives one slow waking
  blink, then settles. Chains straight into **Idle**.
- **Sound:** a gentle rising arpeggio (D→F♯→A) with a soft shimmer.
- **Light:** fades in from off, neutral, settling to the idle floor.

### `A3_idle` — Idle / waiting (loop)
- **When:** activated and at rest — **waiting for the wake word** ("Hey Kai").
- **Behaviour:** a curious face that is never perfectly still — slow ±8% breath (~4.6 s),
  a gentle look-around drift, and a natural blink every ~5–8 s. Occasionally a micro-quirk
  (`E2`) or, rarely, a wink (`E4`) breaks the loop.
- **Sound:** silent.
- **Light:** slow breathe around a low floor (not audio-driven).

### `A4_clock` — Clock (display · **deactivated**)
- **When:** the switch is **deactivated** — the quiet bedside clock.
- **Behaviour:** the eyes give way to a tall digital clock (live `HH:MM`, colon blinking
  once a second). The face fades out and the clock fades in (fluid hand-off).
- **Sound:** silent.
- **Light:** **off** — a quiet, dark clock. **Microphone is also off**: the wake word does
  nothing here.

---

## B · Conversation (turn-taking)

### `B2_listening` — Listening (loop)
- **When:** after the wake word, once **you start speaking**. It's the device's "your turn".
- **Behaviour:** the dots grow +12% and hold **still and attentive** — the stillness is the
  signal that it's listening. Rare blink (~9 s). A notepad + writing pen shows it's taking
  notes / capturing the request.
- **Sound:** silent.
- **Light:** brighter floor (0.65) + a cooler tint — "your turn".

### `B3_thinking` — Thinking / processing (loop)
- **When:** it has heard you and is **processing** the request.
- **Behaviour:** the gaze slips away for a moment and comes back, with one slow half-blink.
  No audio sync — it's "elsewhere", thinking.
- **Sound:** sparse, contemplative blips (loop).
- **Light:** idle floor, slow breathe.

### `B1_speaking` — Speaking / TTS (loop)
- **When:** it is **replying** out loud.
- **Behaviour:** the mouth moves at a speech rhythm (talking), the head bobs gently and
  occasionally turns a little left/right, with a blink every ~6 s. Alive = it's talking.
- **Sound:** the spoken reply (TTS).
- **Light:** pulses with the speech envelope — floor 0.55, fast attack / slow release,
  timed to the eyes.

---

## C · Feedback & confirmation

### `C1_confirm` — Confirm (one-shot)
- **When:** the device **saved or updated** something you asked it to remember or change —
  e.g. your **pronouns** or other important **onboarding** details.
- **Behaviour:** the eyes morph into a **check ✓**, hold, then return to the eyes — a clear
  "got it, saved / updated".
- **Sound:** a bright, positive two-note (A→D).
- **Light:** a single brightness pulse.

### `C2_didnt_catch` — Didn't catch that (one-shot)
- **When:** it **didn't understand** the request.
- **Behaviour:** a slow blink + a gentle tilt (one eye up, one down) — a silent, puzzled
  "hm?" — then back. (No words; just the sound + face.)
- **Sound:** a questioning, upward "hm?" (E→A↗B).
- **Light:** a speak pulse that then cools toward the listening floor (inviting you to
  try again).

---

## D · Proactive moments

### `D2_wakeword` — Wake word (one-shot)
- **When:** it **hears "Hey Kai"** (only while activated, mic on).
- **Behaviour:** an attentive pop — scales up with a little overshoot and settles at the
  listening size. It does **not** jump straight into listening: it then **waits up to 3 s**
  for you to actually say something. If you stay silent, it returns to Idle.
- **Sound:** an attentive rising pop (D→A→D).
- **Light:** lifts and cools to the listening floor — "your turn".

---

## E · Personality

### `E2_quirk` — Idle micro-quirk (rare one-shot)
- **When:** rarely, while idle — a spontaneous moment of life.
- **Behaviour:** slowly looks around (left, then right) and back to center, with one natural
  blink. Alive without distracting.
- **Sound:** silent.
- **Light:** idle floor.

### `E4_wink` — Wink (one-shot)
- **When:** occasionally, a playful beat while idle.
- **Behaviour:** the right eye flattens to a line and holds shut while the left does a small
  sympathetic squint, then both reopen. The wink sound fires exactly when the eye closes.
- **Sound:** a playful upward blip.
- **Light:** idle floor.

---

## Behaviour summary

| Status | Kind | When | Eyes / face | Sound | Light |
| ------ | ---- | ---- | ----------- | ----- | ----- |
| `A1_off` | static | unplugged / off | dark, no dots | descending power-down | off |
| `A2_wake` | one-shot | activated / powered up | fade-in + waking blink → idle | rising arpeggio + shimmer | fade-in to floor |
| `A3_idle` | loop | activated, waiting for wake word | breath + look-around + blink | silent | slow breathe |
| `A4_clock` | display | **deactivated** | clock `HH:MM`, colon 1 Hz | silent | **off** (mic off too) |
| `B2_listening` | loop | you're speaking | dots +12%, still, notepad | silent | brighter + cooler |
| `B3_thinking` | loop | processing | gaze slips away & back | sparse blips | slow breathe |
| `B1_speaking` | loop | replying (TTS) | talking mouth + gentle bob/turn | the reply | pulses with speech |
| `C1_confirm` | one-shot | saved/updated (pronouns, onboarding) | eyes → check ✓ → eyes | positive two-note | single pulse |
| `C2_didnt_catch` | one-shot | didn't understand | slow blink + tilt ("hm?") | questioning "hm?" | speak → cools to listen |
| `D2_wakeword` | one-shot | "Hey Kai" heard | attentive pop, then waits ≤3 s | rising pop | lifts + cools |
| `E2_quirk` | rare one-shot | idle micro-moment | look around + blink | silent | idle floor |
| `E4_wink` | one-shot | playful idle beat | right eye winks shut | playful blip (on close) | idle floor |
