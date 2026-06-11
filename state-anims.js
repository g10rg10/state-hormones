/* state-anims.js — the Maind X state set (spec ids 1:1 with firmware clips,
   plus A4_clock and the E3..E5 expressions). Each state carries two feedback
   layers beyond motion: `sound` (sound.js) and `lightDesc` (the ambient light
   behavior from the Maind X spec §0 · Light states). */
window.STATE_SECTIONS = [
  { key: "A", title: "A · System & power" },
  { key: "B", title: "B · Conversation (turn-taking)" },
  { key: "C", title: "C · Feedback & confirmation" },
  { key: "D", title: "D · Proactive moments" },
  { key: "E", title: "E · Personality / easter eggs" }
];

window.STATES = [
  {
    id: "A1_off", sec: "A", name: "Off",
    light: "off", type: "static", dur: "—",
    lightDesc: "Light fully off — the only state that goes dark.",
    desc: "Device off: no dots, dark screen. No animation."
  },
  {
    id: "A2_wake", sec: "A", name: "Wake",
    light: "[WAKE]", type: "one-shot", dur: "~700 ms + blink",
    lightDesc: "Fade-in from off, neutral light, settles to the floor.",
    desc: "Fade-in from off (opacity 0→100), one slow waking blink, then settle to rest. Chains into A3."
  },
  {
    id: "A3_idle", sec: "A", name: "Idle / waiting",
    light: "[IDLE]", type: "loop", dur: "breath ~4.6 s",
    lightDesc: "Slow breathe around the floor — not audio-driven (~4–5 s).",
    desc: "Slow ±8% scale breath, a gentle look-around drift, and a natural blink every ~5–8 s — never perfectly still, like EMO. Waiting for the wake word."
  },
  {
    id: "A4_clock", sec: "A", name: "Clock",
    light: "[IDLE] · soft tick", type: "display", dur: "live",
    lightDesc: "Idle floor, slow breathe (on the shelf).",
    desc: "On the shelf: the eyes give way to a glowing digital clock (live HH:MM, colon blinking once a second)."
  },
  {
    id: "B1_speaking", sec: "B", name: "Speaking (TTS)",
    light: "[SPEAK]", type: "loop", dur: "length of speech",
    lightDesc: "Pulses with the TTS envelope — floor 0.55, fast attack / slow release, path color.",
    desc: "Breath at speech rhythm (simulated TTS envelope, irregular peaks), blink every ~6 s, tiny drift. Alive = it's talking."
  },
  {
    id: "B2_listening", sec: "B", name: "Listening",
    light: "[LISTEN]", type: "loop", dur: "seamless",
    lightDesc: "Brighter floor (0.65) + cooler tint — “your turn”.",
    desc: "Dots +12%, held still and attentive, rare blink: your turn. The stillness is the signal. A notepad + writing hand shows it's taking notes."
  },
  {
    id: "B3_thinking", sec: "B", name: "Thinking / processing",
    light: "[IDLE]", type: "loop", dur: "~2 s cycle",
    lightDesc: "Idle floor, slow breathe (processing).",
    desc: "The gaze slips away for a moment and comes back, with one slow half blink. No audio sync."
  },
  {
    id: "C1_confirm", sec: "C", name: "Confirm",
    light: "[CONFIRM]", type: "one-shot", dur: "~1.9 s",
    lightDesc: "Single brightness pulse — saved / accepted.",
    desc: "The eyes morph into a check ✓, hold, then return to eyes: choice saved / accepted."
  },
  {
    id: "C2_didnt_catch", sec: "C", name: "Didn't catch that",
    light: "[SPEAK]→[LISTEN]", type: "one-shot", dur: "~600 ms",
    lightDesc: "Speak pulse, then cools toward the listening floor.",
    desc: "Slow blink + gentle tilt (one dot up, one down): a silent “hm?”, then back."
  },
  {
    id: "D1_reminder", sec: "D", name: "Reminder",
    light: "[SPEAK] · reminder-chime", type: "one-shot → B1", dur: "~1.9 s",
    lightDesc: "Calm speak pulse for the reminder (never alarming).",
    desc: "The eyes morph into a little bell that rings, then return to eyes — a calm reminder, never alarming."
  },
  {
    id: "D2_wakeword", sec: "D", name: "Wake word",
    light: "[WAKE]→[LISTEN]", type: "one-shot", dur: "~400 ms",
    lightDesc: "Lifts and cools to the listening floor — “your turn”.",
    desc: "Attentive pop: scales up with a little overshoot and settles at the listening size (the B2 pose)."
  },
  {
    id: "E2_quirk", sec: "E", name: "Idle micro-quirk",
    light: "[IDLE]", type: "one-shot · rare", dur: "~1.3 s",
    lightDesc: "Idle floor.",
    desc: "Slowly looks around (left, then right) and back to center, with one natural blink. Alive without distracting."
  },
  {
    id: "E4_wink", sec: "E", name: "Wink",
    light: "playful", type: "one-shot", dur: "~2.6 s",
    lightDesc: "Idle floor.",
    desc: "The right eye flattens to a line and holds shut while the left does a small sympathetic squint, then both reopen."
  }
];
