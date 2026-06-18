# Maind X Healthcare — Eyes / face animation spec (hardware)

The face is **two glowing dots only** — no mouth, no brows. All expression comes from
**rhythm**: blink, breath, size, position, movement. Calm presence; the eyes never fake
emotion or perform feelings — they signal state.

**Coherence rule:** the eyes always match the light state (hardware doc §1). They are an
intentional *redundant* turn-taking signal: if the ambient light washes out in a bright
room, the eyes still say whose turn it is. Speaking vs Listening must be unmistakable.

**Anti-hyperfocus:** never hypnotic, never "fun to watch on a loop." Subtle and quiet.

---

## The dots — base spec (starting values, tweakable)
- **Shape:** soft-edged glowing dots (slight bloom). Blink = scaleY collapses to a thin
  line, not a hard cut.
- **Size:** base diameter = 1 unit; spacing (center-to-center) ≈ 2.5 units.
- **Color:** constant warm-neutral white for the eyes (keep them readable); the **ambient
  light**, not the eyes, carries the path color. (Decide: if you want eyes path-tinted,
  keep it very subtle so expression stays legible.)
- **Position:** centered at rest; vertical offset used for "tilt" (one up / one down).
- **Frame:** design loops to be seamless; one-shots define clean in/out for chaining.

---

## A · System & power

| ID | Use case | Trigger (flow) | Eyes (two dots) | Light | Type | ~Duration |
|---|---|---|---|---|---|---|
| A1 | Off | device off | dots fully off (dark) | off | static | — |
| A2 | Wake | press on/off · boot | fade-in opacity 0→100, then one slow "waking" blink, settle to rest | `[WAKE]` | one-shot | ~700ms + blink |
| A3 | Idle | waiting for wake word | rest size, slow breath (±8% scale, ~4–5s cycle), occasional natural blink (every ~5–8s) | `[IDLE]` | loop | seamless |
| A4 | Going to sleep | end of flow / closing | gentle fade toward standby; either fully off or one dim dot breathing very slowly | `[IDLE]` · `(closing-tone)` | one-shot | ~800ms |

---

## B · Conversation turn-taking (the core)

| ID | Use case | Trigger (flow) | Eyes (two dots) | Light | Type | ~Duration |
|---|---|---|---|---|---|---|
| B1 | Speaking | device talks (TTS) | breath synced to the TTS amplitude envelope (gentle rise on stressed syllables), blink every ~6s, tiny natural drift | `[SPEAK]` | loop (length of speech) | variable |
| B2 | Listening | mic open, your turn | dots slightly larger (+10–15% scale), **held still**, blink rare or none, stable centered position | `[LISTEN]` | loop | seamless |
| B3 | Thinking / processing | parsing, between turns | slow blink or small drift (eyes look briefly "away" then back), no audio sync | `[IDLE]` | loop (short) | ~1–2s cycle |

> B1↔B2 is the most important pair to nail: **alive/breathing = it's talking; larger,
> still, attentive = your turn.** Make the difference obvious at a glance.

---

## C · Feedback & confirmation

| ID | Use case | Trigger (flow) | Eyes (two dots) | Light | Type | ~Duration |
|---|---|---|---|---|---|---|
| C1 | Confirm | a choice saved/accepted (reminder, edit, mark-as-important) | quick **double blink** (~2 blinks in ~300ms) | `[CONFIRM]` | one-shot | ~300ms |
| C2 | Didn't catch that | fallback, ambiguous input | slow blink + gentle **tilt** (one dot up, one down) suggesting a soft "hm?", then return | `[SPEAK]`→`[LISTEN]` | one-shot | ~600ms |

---

## D · Proactive moments

| ID | Use case | Trigger (flow) | Eyes (two dots) | Light | Type | ~Duration |
|---|---|---|---|---|---|---|
| D1 | Reminder fires | scheduled reminder (device only) | gentle "perk up" (quick scale-up), one soft blink to draw attention — calm, not alarming | `[SPEAK]` · `(reminder-chime)` | one-shot → into B1 | ~500ms |
| D2 | Heard the wake word | "Hey Kai!" | brief attentive pop (scale-up + settle) as it shifts into listening | `[WAKE]`→`[LISTEN]` | one-shot | ~400ms |

---

## E · Personality / easter eggs

| ID | Use case | Trigger | Eyes (two dots) | Light | Type | ~Duration |
|---|---|---|---|---|---|---|
| E1 | Shaken (dizzy) | device picked up / shaken | dizzy wobble — dots swirl / zig-zag, briefly destabilized, then recover | playful, brief | one-shot | ~1–1.5s |
| E2 | Idle micro-quirks | rare, during long idle | occasional natural double-blink or a slow "look around" and back — keeps it alive without distracting | `[IDLE]` | one-shot (rare) | ~1s |

---

## Production notes
- **Loops vs one-shots:** A3, B1, B2, B3 are loops (seamless); everything else is a
  one-shot with clean in/out so states can chain (e.g. A2 → A3, D1 → B1).
- **Tool:** Rive is ideal — build a state machine matching the light states, so eyes +
  light are driven by the same triggers. For presentation clips, export loopable
  videos from Rive/Jitter. Text-to-video (Veo, Runway, Kling) is unreliable for precise
  loopable signals like double-blinks — don't use it for these.
- **Frame rate:** 30 or 60 fps; keep blinks crisp (~120–150ms close+open).
- **Clip naming:** match the IDs above (A2_wake, B1_speaking, B2_listening, C1_confirm…)
  so the firmware/state machine can reference them 1:1.
- **Audio sync (B1, D1):** drive the eye breath from the same TTS amplitude envelope as
  the light, so the two channels move together.

---

*Julie Alme, Giorgio Gaudio, Mattia Piatti, A. M. Luz Vanella — Maind X Healthcare*
