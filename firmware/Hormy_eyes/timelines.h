// timelines.h — the 16 state keyframe tables, transcribed 1:1 from
// state-keyframes.css / states.css. Percent values and easings match the CSS
// exactly. One-shot "action windows" come from each state's CSS comment.
#pragma once
#include "anim.h"

// ============================ A · system & power ============================

// A1_off — static, dark screen. (no tracks)

// A2_wake — one-shot 4000ms, action 0-37%, chain -> A3_idle
static const Kf a2_scale[] = {{0,.92f},{17,1},{90,1},{97,.92f},{100,.92f}};
static const Kf a2_op[]    = {{0,0},{17,1},{90,1},{97,0},{100,0}};
static const Kf a2_dot[]   = {{0,1},{21,1},{26,.08f},{28.5f,.08f},{34,1.05f},{37,1},{100,1}};
// mouth: a resting smile appears with the pop-in (no own morph — rides the group).
static const Kf a2_m_alpha[] = {{0,1}};
static const Kf a2_m_curve[] = {{0,1}};
static const Track a2_tracks[] = {
  {CH_G_SCALE, EASE_A2_GRP, 5, 4000, a2_scale},
  {CH_DOP,     EASE_A2_GRP, 5, 4000, a2_op},
  {CH_DSY,     EASE_A2_DOT, 7, 4000, a2_dot},
  {CH_MOUTH_ALPHA, EASE_LINEAR, 1, 4000, a2_m_alpha},
  {CH_MOUTH_CURVE, EASE_LINEAR, 1, 4000, a2_m_curve},
};

// A3_idle — loop. breath 4600ms + look-around drift 7200ms + blink 13000ms
// (three independent rhythms = EMO's "never perfectly still" idle).
static const Kf a3_scale[] = {{0,1},{50,1.08f},{100,1}};
static const Kf a3_drift[] = {{0,0},{18,-11},{32,-11},{50,0},{68,12},{82,12},{100,0}};
static const Kf a3_dot[]   = {{0,1},{38.5f,1},{39.06f,.07f},{39.62f,1},{86.4f,1},{86.96f,.07f},{87.52f,1},{100,1}};
// mouth: steady smile that breathes (idleSmile -1px lift) and dips toward neutral
// occasionally (web ~80% smile / 20% neutral). Horizontal drift inherited from gTx.
static const Kf a3_m_alpha[] = {{0,1}};
static const Kf a3_m_curve[] = {{0,1},{60,1},{72,.11f},{86,.11f},{100,1}};
static const Kf a3_m_cy[]    = {{0,0},{50,-1},{100,0}};
static const Track a3_tracks[] = {
  {CH_G_SCALE, EASE_IN_OUT, 3, 4600,  a3_scale},
  {CH_G_TX,    EASE_IN_OUT, 7, 7200,  a3_drift},
  {CH_DSY,     EASE_DOT_46, 8, 13000, a3_dot},
  {CH_MOUTH_ALPHA, EASE_LINEAR, 1, 9000, a3_m_alpha},
  {CH_MOUTH_CURVE, EASE_IN_OUT, 5, 9000, a3_m_curve},
  {CH_MOUTH_CY,    EASE_IN_OUT, 3, 9000, a3_m_cy},
};

// A4_clock — display (clock overlay). (no tracks)

// ======================= B · conversation turn-taking =======================

// B1_speaking — loop. group 3400ms (scale+tx+ty), blink 6100ms
static const Kf b1_scale[] = {{0,1},{7,1.055f},{13,1.02f},{21,1.075f},{28,1.03f},{36,1.06f},
                              {44,1.015f},{52,1.08f},{60,1.035f},{68,1.07f},{76,1.02f},{84,1.06f},{92,1.025f},{100,1}};
static const Kf b1_tx[]    = {{0,0},{7,0},{13,.5f},{21,1},{28,.5f},{36,0},{44,-.5f},{52,-1},{60,-.5f},{68,0},{76,.5f},{84,.5f},{92,0},{100,0}};
static const Kf b1_ty[]    = {{0,0},{7,-1.5f},{13,0},{21,-2},{28,-.5f},{36,-1},{44,0},{52,-2},{60,-.5f},{68,-1.5f},{76,0},{84,-1},{92,0},{100,0}};
static const Kf b1_dot[]   = {{0,1},{70,1},{71.2f,.07f},{72.4f,1},{100,1}};
// mouth: the talking mouth — smile hidden, the open mouth oscillates open/close.
// open + width transcribed 1:1 from web face-B1-open scale(sx,sy), ~5 beats / 3800ms.
// (mouthCurve stays 0; drawMouth hides the smile stroke whenever mouthOpen is high.)
static const Kf b1_m_alpha[] = {{0,1}};
static const Kf b1_m_open[]  = {{0,.20f},{9,.88f},{18,.22f},{27,1.0f},{36,.20f},
                                {45,.80f},{54,.22f},{63,.92f},{72,.20f},{81,.62f},{90,.22f},{100,.20f}};
static const Kf b1_m_width[] = {{0,.55f},{9,1.05f},{18,.55f},{27,.90f},{36,.55f},
                                {45,1.18f},{54,.55f},{63,.95f},{72,.55f},{81,.85f},{90,.55f},{100,.55f}};
static const Track b1_tracks[] = {
  {CH_G_SCALE, EASE_IN_OUT, 14, 3400, b1_scale},
  {CH_G_TX,    EASE_IN_OUT, 14, 3400, b1_tx},
  {CH_G_TY,    EASE_IN_OUT, 14, 3400, b1_ty},
  {CH_DSY,     EASE_DOT_46,  5, 6100, b1_dot},
  {CH_MOUTH_ALPHA, EASE_LINEAR, 1,  3800, b1_m_alpha},
  {CH_MOUTH_OPEN,  EASE_OUT,    12, 3800, b1_m_open},
  {CH_MOUTH_WIDTH, EASE_OUT,    12, 3800, b1_m_width},
};

// B2_listening — loop, held scale 1.12 + rare blink 9000ms + notepad overlay
static const Kf b2_dot[] = {{0,1},{54,1},{54.8f,.07f},{55.6f,1},{100,1}};
// mouth: a smaller smile (web face-B2-smile scale .78->.84) that gently bobs.
static const Kf b2_m_alpha[] = {{0,1}};
static const Kf b2_m_width[] = {{0,.80f},{50,.84f},{100,.80f}};
static const Kf b2_m_curve[] = {{0,1}};
static const Kf b2_m_cy[]    = {{0,0},{33,-2},{66,2},{100,0}};
static const Track b2_tracks[] = {
  {CH_DSY, EASE_DOT_46, 5, 9000, b2_dot},
  {CH_MOUTH_ALPHA, EASE_LINEAR, 1, 2900, b2_m_alpha},
  {CH_MOUTH_WIDTH, EASE_IN_OUT, 3, 2900, b2_m_width},
  {CH_MOUTH_CURVE, EASE_LINEAR, 1, 2900, b2_m_curve},
  {CH_MOUTH_CY,    EASE_IN_OUT, 4, 2900, b2_m_cy},
};

// B3_thinking — loop. gaze-slip group 2000ms + half-blink 3400ms
static const Kf b3_tx[]  = {{0,0},{28,10},{55,11},{78,0},{100,0}};
static const Kf b3_ty[]  = {{0,0},{28,-7},{55,-6},{78,0},{100,0}};
static const Kf b3_dot[] = {{0,1},{42,1},{47,.3f},{49,.3f},{55,1},{100,1}};
// mouth: a pursed, near-constant-radius pondering arc whose span grows/retracts.
static const Kf b3_m_alpha[] = {{0,1}};
static const Kf b3_m_curve[] = {{0,.85f}};
static const Kf b3_m_width[] = {{0,.70f},{35,1.0f},{70,.55f},{100,.70f}};
static const Track b3_tracks[] = {
  {CH_G_TX, EASE_GRP_42, 5, 2000, b3_tx},
  {CH_G_TY, EASE_GRP_42, 5, 2000, b3_ty},
  {CH_DSY,  EASE_IN_OUT, 6, 3400, b3_dot},
  {CH_MOUTH_ALPHA, EASE_LINEAR, 1, 2400, b3_m_alpha},
  {CH_MOUTH_CURVE, EASE_LINEAR, 1, 2400, b3_m_curve},
  {CH_MOUTH_WIDTH, EASE_IN_OUT, 4, 2400, b3_m_width},
};

// ========================= C · feedback & confirm ===========================

// C1_confirm — one-shot: eyes fade out, a check ✓ morphs in, then eyes return.
// (the check itself is drawn in draw.h's renderFrame from the C1 progress)
static const Kf c1_op[] = {{0,1},{12,0},{80,0},{92,1},{100,1}};
static const Track c1_tracks[] = {
  {CH_DOP, EASE_IN_OUT, 5, 2400, c1_op},
};

// C2_didnt_catch — one-shot 3600ms, action 0-17%. tilt L-13 / R+13
static const Kf c2_tyL[] = {{0,0},{8,-13},{11,-13},{16.7f,0},{100,0}};
static const Kf c2_tyR[] = {{0,0},{8,13},{11,13},{16.7f,0},{100,0}};
static const Kf c2_dot[] = {{0,1},{2,1},{6.5f,.1f},{11,1},{100,1}};
// mouth: smile relaxes to a slight downturn ("hm? didn't catch that") within the
// 0-17% action window (one-shot clamps the playhead there, then holds the pose).
static const Kf c2_m_alpha[] = {{0,1}};
static const Kf c2_m_curve[] = {{0,1},{16.7f,-.4f}};
static const Track c2_tracks[] = {
  {CH_ETY_L, EASE_C2_EYE, 5, 3600, c2_tyL},
  {CH_ETY_R, EASE_C2_EYE, 5, 3600, c2_tyR},
  {CH_DSY,   EASE_IN_OUT, 5, 3600, c2_dot},
  {CH_MOUTH_ALPHA, EASE_LINEAR, 1, 3600, c2_m_alpha},
  {CH_MOUTH_CURVE, EASE_IN_OUT, 2, 3600, c2_m_curve},
};

// ========================== D · proactive moments ===========================

// D1_reminder — one-shot: eyes fade out, a bell rings, then eyes return.
// (the bell is drawn in draw.h's renderFrame from the D1 progress; chains -> B1)
static const Kf d1_op[] = {{0,1},{12,0},{82,0},{94,1},{100,1}};
static const Track d1_tracks[] = {
  {CH_DOP, EASE_IN_OUT, 5, 3200, d1_op},
};

// D2_wakeword — one-shot 3400ms, action 0-15% (holds B2 pose), chain -> B2_listening
static const Kf d2_scale[] = {{0,1},{7,1.19f},{11.8f,1.095f},{15,1.12f},{86,1.12f},{96,1},{100,1}};
static const Kf d2_bri[]   = {{0,1},{7,1.22f},{16,1},{100,1}};
// mouth: a brief surprised "o" forms within the 0-15% action window, then holds
// until the one-shot chains -> B2_listening (whose smile completes the "o -> smile").
static const Kf d2_m_alpha[] = {{0,0},{6,1},{15,1}};
static const Kf d2_m_open[]  = {{0,.6f},{8,1.0f},{15,1.0f}};
static const Kf d2_m_width[] = {{0,.40f},{8,.55f},{15,.55f}};
static const Track d2_tracks[] = {
  {CH_G_SCALE, EASE_D2_GRP, 7, 3400, d2_scale},
  {CH_DBR,     EASE_OUT,    4, 3400, d2_bri},
  {CH_MOUTH_ALPHA, EASE_OUT, 3, 3400, d2_m_alpha},
  {CH_MOUTH_OPEN,  EASE_OUT, 3, 3400, d2_m_open},
  {CH_MOUTH_WIDTH, EASE_OUT, 3, 3400, d2_m_width},
};

// ===================== E · personality / expressions ========================

// E2_quirk — one-shot 4200ms, action 0-30%. look-around + blink
static const Kf e2_tx[]  = {{0,0},{9,-17},{16,-15},{24,15},{30,0},{100,0}};
static const Kf e2_ty[]  = {{0,0},{9,2},{16,3},{24,-2},{30,0},{100,0}};
static const Kf e2_dot[] = {{0,1},{26,1},{27.7f,.07f},{29.4f,1},{100,1}};
// mouth: plain resting smile riding the look-around.
static const Kf e2_m_alpha[] = {{0,1}};
static const Kf e2_m_curve[] = {{0,1}};
static const Track e2_tracks[] = {
  {CH_G_TX, EASE_GRP_42, 6, 4200, e2_tx},
  {CH_G_TY, EASE_GRP_42, 6, 4200, e2_ty},
  {CH_DSY,  EASE_DOT_46, 5, 4200, e2_dot},
  {CH_MOUTH_ALPHA, EASE_LINEAR, 1, 4200, e2_m_alpha},
  {CH_MOUTH_CURVE, EASE_LINEAR, 1, 4200, e2_m_curve},
};

// E4_wink — one-shot 2600ms, action 0-56%. right shuts, left squints from bottom
static const Kf e4_dotR[] = {{0,1},{14,1},{20,.07f},{46,.07f},{54,1},{100,1}};
static const Kf e4_dotL[] = {{0,1},{18,1},{30,.78f},{46,.78f},{56,1},{100,1}};
// mouth: a playful smile that lifts (web scale 1.18) during the wink, within the
// 0-56% action window. (web skewY(-5deg) shear is omitted — the lift is dominant.)
static const Kf e4_m_alpha[] = {{0,1}};
static const Kf e4_m_curve[] = {{0,1},{14,1},{20,1.18f},{46,1.18f},{56,1}};
static const Kf e4_m_width[] = {{0,1},{14,1},{20,1.18f},{46,1.18f},{56,1}};
static const Track e4_tracks[] = {
  {CH_DSY_R, EASE_E4_DOTR, 6, 2600, e4_dotR},
  {CH_DSY_L, EASE_IN_OUT,  6, 2600, e4_dotL},
  {CH_MOUTH_ALPHA, EASE_LINEAR, 1, 2600, e4_m_alpha},
  {CH_MOUTH_CURVE, EASE_OUT,    5, 2600, e4_m_curve},
  {CH_MOUTH_WIDTH, EASE_OUT,    5, 2600, e4_m_width},
};

// ============================== state table =================================
// index order matches states.js. chainTo uses these indices:
//   A3_idle = 2, B1_speaking = 4, B2_listening = 5
#define NUM_STATES 13
static const StateDef STATES[NUM_STATES] = {
//  id              mode          baseDur action chain nTr tracks       sScale ovl  pivot dotsOff
  { "A1_off",       MODE_STATIC,     0,    0,   0xFF, 0,  nullptr,     0,    OV_NONE,    0, true  },
  { "A2_wake",      MODE_ONESHOT, 4000,   37,    2,   5,  a2_tracks,   0,    OV_NONE,    0, false },
  { "A3_idle",      MODE_LOOP,    4600,  100,   0xFF, 6,  a3_tracks,   0,    OV_NONE,    0, false },
  { "A4_clock",     MODE_DISPLAY,    0,    0,   0xFF, 0,  nullptr,     0,    OV_CLOCK,   0, false },
  { "B1_speaking",  MODE_LOOP,    3400,  100,   0xFF, 7,  b1_tracks,   0,    OV_NONE,    0, false },
  { "B2_listening", MODE_LOOP,    9000,  100,   0xFF, 5,  b2_tracks,   1.12f, OV_NOTEPAD, 0, false },
  { "B3_thinking",  MODE_LOOP,    2000,  100,   0xFF, 6,  b3_tracks,   0,    OV_NONE,    0, false },
  { "C1_confirm",   MODE_ONESHOT, 2400,  92,   0xFF, 1,  c1_tracks,   0,    OV_NONE,    0, false },
  { "C2_didnt_catch", MODE_ONESHOT, 3600, 17,  0xFF, 5,  c2_tracks,   0,    OV_NONE,    0, false },
  { "D1_reminder",  MODE_ONESHOT, 3200,   94,    4,   1,  d1_tracks,   0,    OV_NONE,    0, false },
  { "D2_wakeword",  MODE_ONESHOT, 3400,   15,    5,   5,  d2_tracks,   1.12f, OV_NONE,   0, false },
  { "E2_quirk",     MODE_ONESHOT, 4200,   30,   0xFF, 5,  e2_tracks,   0,    OV_NONE,    0, false },
  { "E4_wink",      MODE_ONESHOT, 2600,   56,   0xFF, 5,  e4_tracks,   0,    OV_NONE,    1, false },
};
