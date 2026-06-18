// anim.h — the animation engine: 3-layer keyframe model faithful to the web
// gallery (.eyes group / .eye per-eye / .dot), keyframe sampling, and per-state
// evaluation into a Pose. Data tables live in timelines.h.
#pragma once
#include <Arduino.h>
#include "easing.h"

// ---- animation channels: exactly what a track drives ----
enum {
  CH_G_SCALE = 0,   // .eyes  uniform scale (about screen center)
  CH_G_TX,          // .eyes  translateX (px)
  CH_G_TY,          // .eyes  translateY (px)
  CH_G_ROT,         // .eyes  rotate (deg)   — E1 only
  CH_ETY_L,         // .eye.left  translateY (px)
  CH_ETY_R,         // .eye.right translateY (px)
  CH_DSY,           // .dot   scaleY, BOTH eyes
  CH_DSY_L,         // .dot   scaleY, left only   — E4
  CH_DSY_R,         // .dot   scaleY, right only  — E4
  CH_DOP,           // .dot   opacity, both       — A2 fade-in
  CH_DBR,           // .dot   brightness, both    — D2 flash
  // --- mouth channels (faithful port of the web .smile / .open layers) ---
  CH_MOUTH_ALPHA,   // overall mouth opacity 0..1 (0 = hidden)
  CH_MOUTH_WIDTH,   // horizontal span scale (1.0 = resting-smile span)
  CH_MOUTH_CURVE,   // signed arc depth: +1 smile, 0 flat, -1 frown
  CH_MOUTH_OPEN,    // 0 = stroked curve, 1 = filled open mouth
  CH_MOUTH_CY,      // mouth-specific vertical offset (px) — idle / B2 bob
};

enum { MODE_STATIC = 0, MODE_LOOP, MODE_ONESHOT, MODE_DISPLAY };
enum { OV_NONE = 0, OV_CLOCK, OV_NOTEPAD };

struct Kf  { float pct; float v; };                  // keyframe: percent 0..100 -> value
struct Track {
  uint8_t  ch;       // CH_*
  uint8_t  ease;     // EASE_*  (this track's timing-function)
  uint8_t  nKf;
  uint32_t durMs;    // this track's OWN period (loops free-run on it)
  const Kf* kf;
};
struct StateDef {
  const char* id;
  uint8_t  mode;          // MODE_*
  uint32_t baseDurMs;     // cycle length used for the one-shot action window
  float    actionEndPct;  // one-shot: play 0..this, then hold / chain
  uint8_t  chainTo;       // state index to enter after a one-shot (0xFF = stay)
  uint8_t  nTracks;
  const Track* tracks;
  float    staticScale;   // held group scale (B2 = 1.12; else 1.0)
  uint8_t  overlay;       // OV_*
  uint8_t  pivotBottom;   // bit0 = left dot, bit1 = right dot squash from the BOTTOM
  bool     dotsOff;       // A1 = dark screen
};

// resolved transform for one frame
struct Pose {
  float gScale = 1, gTx = 0, gTy = 0, gRot = 0;
  float etyL = 0, etyR = 0;
  float dSyL = 1, dSyR = 1;
  float dOpacity = 1, dBright = 1;
  bool  pivotBL = false, pivotBR = false;
  // mouth (faithful port). mouthAlpha defaults to 0 -> hidden unless a track drives it.
  float mouthAlpha = 0, mouthWidth = 1, mouthCurve = 0, mouthOpen = 0, mouthCy = 0;
};

extern float g_spd;   // global tempo multiplier (1.0). Defined in the .ino.

// playhead in percent (0..100) for a track, given mode.
static inline float trackPlayhead(uint32_t now, uint32_t startMs, uint32_t durMs,
                                  uint8_t mode, float actionEndPct) {
  uint32_t durScaled = (uint32_t)(durMs * g_spd);
  if (durScaled == 0) return 0;
  uint32_t el = now - startMs;
  if (mode == MODE_LOOP) {
    float p = fmodf((float)el / durScaled, 1.0f) * 100.0f;
    return p;
  }
  // one-shot: advance to actionEndPct, then hold there
  float p = (float)el / durScaled * 100.0f;
  if (p > actionEndPct) p = actionEndPct;
  return p;
}

// sample one track at a given percent, with per-segment easing (CSS semantics).
static inline float sampleTrack(const Track* t, float pct) {
  if (t->nKf == 1) return t->kf[0].v;
  int i = 0;
  while (i < t->nKf - 1 && t->kf[i + 1].pct <= pct) i++;
  const Kf& a = t->kf[i];
  const Kf& b = t->kf[(i + 1 < t->nKf) ? i + 1 : i];
  if (b.pct <= a.pct) return b.v;
  float local = (pct - a.pct) / (b.pct - a.pct);
  if (local < 0) local = 0; else if (local > 1) local = 1;
  float e = easeSample(t->ease, local);
  return a.v + (b.v - a.v) * e;
}

// evaluate a whole state into a Pose.
static inline Pose evalState(const StateDef* s, uint32_t now, uint32_t startMs) {
  Pose p;
  p.gScale  = (s->staticScale > 0) ? s->staticScale : 1.0f;
  p.pivotBL = (s->pivotBottom & 1) != 0;
  p.pivotBR = (s->pivotBottom & 2) != 0;
  for (uint8_t k = 0; k < s->nTracks; k++) {
    const Track* t = &s->tracks[k];
    float pct = trackPlayhead(now, startMs, t->durMs, s->mode, s->actionEndPct);
    float v   = sampleTrack(t, pct);
    switch (t->ch) {
      case CH_G_SCALE: p.gScale = v; break;
      case CH_G_TX:    p.gTx = v; break;
      case CH_G_TY:    p.gTy = v; break;
      case CH_G_ROT:   p.gRot = v; break;
      case CH_ETY_L:   p.etyL = v; break;
      case CH_ETY_R:   p.etyR = v; break;
      case CH_DSY:     p.dSyL = v; p.dSyR = v; break;
      case CH_DSY_L:   p.dSyL = v; break;
      case CH_DSY_R:   p.dSyR = v; break;
      case CH_DOP:     p.dOpacity = v; break;
      case CH_DBR:     p.dBright = v; break;
      case CH_MOUTH_ALPHA: p.mouthAlpha = v; break;
      case CH_MOUTH_WIDTH: p.mouthWidth = v; break;
      case CH_MOUTH_CURVE: p.mouthCurve = v; break;
      case CH_MOUTH_OPEN:  p.mouthOpen  = v; break;
      case CH_MOUTH_CY:    p.mouthCy    = v; break;
    }
  }
  return p;
}

// is a one-shot past its action window? (used by the demo driver / host chaining)
static inline bool oneShotDone(const StateDef* s, uint32_t now, uint32_t startMs) {
  if (s->mode != MODE_ONESHOT) return false;
  uint32_t durScaled = (uint32_t)(s->baseDurMs * g_spd);
  float p = (float)(now - startMs) / durScaled * 100.0f;
  return p >= s->actionEndPct;
}
