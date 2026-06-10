/* state-anims.js — the Maind X state set (spec ids 1:1 with firmware clips,
   plus A4_clock and the E3..E5 expressions). */
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
    desc: "Device off: no dots, dark screen. No animation."
  },
  {
    id: "A2_wake", sec: "A", name: "Wake",
    light: "[WAKE]", type: "one-shot", dur: "~700 ms + blink",
    desc: "Fade-in from off (opacity 0→100), one slow waking blink, then settle to rest. Chains into A3."
  },
  {
    id: "A3_idle", sec: "A", name: "Idle / waiting",
    light: "[IDLE]", type: "loop", dur: "breath ~4.6 s",
    desc: "Slow ±8% scale breath, natural blink every ~5–8 s. Waiting for the wake word."
  },
  {
    id: "A4_clock", sec: "A", name: "Clock",
    light: "[IDLE] · soft tick", type: "display", dur: "live",
    desc: "On the shelf: the eyes give way to a glowing digital clock (live HH:MM, colon blinking once a second)."
  },
  {
    id: "B1_speaking", sec: "B", name: "Speaking (TTS)",
    light: "[SPEAK]", type: "loop", dur: "length of speech",
    desc: "Breath at speech rhythm (simulated TTS envelope, irregular peaks), blink every ~6 s, tiny drift. Alive = it's talking."
  },
  {
    id: "B2_listening", sec: "B", name: "Listening",
    light: "[LISTEN]", type: "loop", dur: "seamless",
    desc: "Dots +12%, held still and attentive, rare blink: your turn. The stillness is the signal."
  },
  {
    id: "B3_thinking", sec: "B", name: "Thinking / processing",
    light: "[IDLE]", type: "loop", dur: "~2 s cycle",
    desc: "The gaze slips away for a moment and comes back, with one slow half blink. No audio sync."
  },
  {
    id: "C1_confirm", sec: "C", name: "Confirm",
    light: "[CONFIRM]", type: "one-shot", dur: "~300 ms",
    desc: "Quick double blink (2 blinks in ~300 ms): choice saved / accepted."
  },
  {
    id: "C2_didnt_catch", sec: "C", name: "Didn't catch that",
    light: "[SPEAK]→[LISTEN]", type: "one-shot", dur: "~600 ms",
    desc: "Slow blink + gentle tilt (one dot up, one down): a silent “hm?”, then back."
  },
  {
    id: "D1_reminder", sec: "D", name: "Reminder",
    light: "[SPEAK] · reminder-chime", type: "one-shot → B1", dur: "~500 ms",
    desc: "“Perk up”: a small scale pop + one soft blink to draw attention — calm, not alarming."
  },
  {
    id: "D2_wakeword", sec: "D", name: "Wake word",
    light: "[WAKE]→[LISTEN]", type: "one-shot", dur: "~400 ms",
    desc: "Attentive pop: scales up with a little overshoot and settles at the listening size (the B2 pose)."
  },
  {
    id: "E1_dizzy", sec: "E", name: "Shaken (dizzy)",
    light: "playful · brief", type: "one-shot", dur: "~1–1.5 s",
    desc: "Dizzy wobble: zig-zag out of phase between the two dots, briefly destabilized, then recovers."
  },
  {
    id: "E2_quirk", sec: "E", name: "Idle micro-quirk",
    light: "[IDLE]", type: "one-shot · rare", dur: "~1.3 s",
    desc: "Slowly looks around (left, then right) and back to center, with one natural blink. Alive without distracting."
  },
  {
    id: "E3_happy", sec: "E", name: "Happy",
    light: "playful", type: "one-shot", dur: "~3.6 s",
    desc: "Expression by shape only: the eyes squish down into short rounded bars — a smile made of eyes — with a small upward lift. No nose."
  },
  {
    id: "E4_wink", sec: "E", name: "Wink",
    light: "playful", type: "one-shot", dur: "~2.6 s",
    desc: "The right eye flattens to a line and holds shut while the left does a small sympathetic squint, then both reopen."
  },
  {
    id: "E5_content", sec: "E", name: "Content",
    light: "calm", type: "one-shot", dur: "~4.2 s",
    desc: "Both eyes slowly lower to half-lidded rounded bars (calm / satisfied) and hold, then reopen. Height-only expression."
  }
];
