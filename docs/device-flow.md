# Maind X — Device flow (activate / deactivate switch)

The device is a small bedside companion (round eyes screen + a fabric speaker body)
with a **physical switch on top** — like the reference object: one position lights the
device up for interaction, the other puts it to rest as a quiet clock.

The switch has two positions:

| Switch        | Screen                | Microphone | What it does                                  |
| ------------- | --------------------- | ---------- | --------------------------------------------- |
| **Deactivated** | **Clock only** (HH:MM) | **OFF**   | Quiet bedside clock. No voice, no listening.  |
| **Activated**   | The face (eyes)        | **ON**    | Full voice assistant: wake word + interaction |

The device is always plugged in / powered. The switch is **not** a power button — it
only toggles the assistant **on the device that's already running**.

---

## 1. Plugged in — switch DEACTIVATED → Clock

- Screen shows **only the clock** (state `A4_clock`, live `HH:MM`, blinking colon).
- The **microphone is OFF**: saying "Hey Arduino" does **nothing**.
- Ambient light: calm idle floor (warm). No interaction is possible here.

```
[ Deactivated ] ──▶ Clock display  (mic OFF)  ──▶ stays here until you flip the switch
```

## 2. Flip the switch to ACTIVATED → wakes up, mic ON

- A short **wake** animation (`A2_wake`) — the face appears.
- Settles into **Idle / waiting** (`A3_idle`) — the curious face looks around.
- The **microphone turns ON** and the device starts **listening for the wake word**
  ("Hey Arduino").

```
[ Activated ] ──▶ Wake (A2) ──▶ Idle / waiting (A3)  + mic ON, listening for "Hey Arduino"
```

## 3. Voice interaction (only while Activated)

When the device hears **"Hey Arduino"**:

1. **Wake word** (`D2_wakeword`) — an attentive perk-up. It does **not** jump straight
   into the listening pose; it **waits for you to actually say something**.
2. **You speak within 3 s** → **Listening** (`B2_listening`, notepad + pen) →
   **Thinking** (`B3_thinking`) → **Speaking / reply** (`B1_speaking`) → back to **Idle**.
3. **You say nothing for more than 3 s** → it gives up and returns to **Idle**.

```
"Hey Arduino"
      │
      ▼
  Wake word (D2) ── waits up to 3 s for speech ──┐
      │                                          │
   you speak                                no speech (>3 s)
      ▼                                          ▼
 Listening (B2) ─▶ Thinking (B3) ─▶ Speaking (B1) ─▶ Idle (A3)   Idle (A3)
```

Other proactive / feedback states (Confirm, Didn't-catch, Reminder, Wink, quirks) all
live inside this **Activated** mode.

### Confirm vs Reminder — when they trigger

These two look similar (a brief morph) but mean different things:

- **Confirm** (`C1`, check ✓) — the device **saved or updated** something you asked it
  to remember or change: your **pronouns** or other important **onboarding** details, or
  when you **set up a recurring / daily event** (e.g. "remind me to take my pill at 9").
  It's the "got it, saved / updated" acknowledgement.
- **Reminder** (`D1`, ringing bell) — the **proactive reminder itself**, fired when it's
  **time to do** something you'd previously asked to be reminded of: **take the pill**,
  **apply the gel**, **do the injection**. Calm, never alarming.

So: you *ask* to be reminded → **Confirm** (saved). Later, when the time comes → **Reminder**.

## 4. Flip the switch back to DEACTIVATED → Clock

- Any interaction stops, the **microphone turns OFF**, and the screen returns to the
  **clock** — fluidly (the face fades out while the clock fades in).

```
[ Deactivated ] ──▶ mic OFF ──▶ Clock display
```

---

## State summary

| State            | Mode        | Reachable when… |
| ---------------- | ----------- | --------------- |
| `A4_clock`       | Deactivated | switch off (mic off) |
| `A2_wake`        | Activated   | on flipping the switch on |
| `A3_idle`        | Activated   | resting / waiting for the wake word |
| `D2_wakeword`    | Activated   | "Hey Arduino" heard |
| `B2_listening`   | Activated   | you started speaking |
| `B3_thinking`    | Activated   | processing |
| `B1_speaking`    | Activated   | replying (TTS) |
| `C1_confirm`     | Activated   | saved/updated something (pronouns, onboarding, recurring event) |
| `C2_didnt_catch` | Activated   | didn't understand the request |
| `D1_reminder`    | Activated   | proactive reminder fires (pill / gel / injection) |
| `E*` (quirk/wink)| Activated   | personality / idle micro-moments |

## Demo (face.html)

- The **top switch** ("Active" / "Clock") is the physical switch — flip it to
  activate (face + mic) or deactivate (clock + mic off).
- While **Active**, real speech works: say **"Hey Arduino"** (grant the mic) and then a
  request; if you stay silent for 3 s after the wake word it returns to idle.
- **▶ Simulate flow** runs the whole lifecycle hands-free: deactivated clock → activate →
  idle → "Hey Arduino" → wait → listening → thinking → reply → idle → deactivate → clock,
  looping.
