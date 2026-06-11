// draw.h — rendering. Re-rasterizes the two glossy EMO eyes (gradient + gloss +
// glow) every frame, plus the tall clock and the listening notepad. Faithful to
// states.css geometry. Uses only Arduino_GFX primitives, so it works with any
// Arduino_GFX* `gfx` (here an Arduino_RGB_Display drawing into PSRAM).
#pragma once
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <math.h>
#include "palette.h"
#include "timelines.h"

extern Arduino_GFX* gfx;     // defined in panel_config.h (filled by the user)

// ----- base geometry (states.css, CONFIRMED) -----
static const int SCREEN = 480;
static const int CXC = 240, CYC = 240;        // screen center
static const int EYE_W = 124, EYE_H = 132, EYE_RAD = 44;   // rounded-square, EMO-like
static const int GAP = 58;                     // between the inner edges
static const int BASE_CX_L = 240 - GAP / 2 - EYE_W / 2;   // 150
static const int BASE_CX_R = 240 + GAP / 2 + EYE_W / 2;   // 330
static const float GLOW_REACH = 40.0f;         // halo px (scaled from CSS 72 for round-fit/perf)

// ---------------------------------------------------------------- eye glow
static void drawEyeGlow(int cx, int top, int w, int h, int rad, float alpha) {
  const int KN = 18;
  for (int k = KN; k >= 1; k--) {
    float f = (float)k / KN;               // 1 = outer, ~0 = inner
    float d = GLOW_REACH * f;               // distance from the eye edge
    int   off = (int)(d + 0.5f);
    float inten = 0.24f * (1.0f - f);       // wide weak lobe
    if (d < 18.0f) inten += 0.55f * (1.0f - d / 18.0f);   // tight strong lobe
    if (inten > 0.75f) inten = 0.75f;
    inten *= alpha;
    RGB col = lerpRGB(BG, GLOW, inten);
    gfx->fillRoundRect(cx - w / 2 - off, top - off, w + 2 * off, h + 2 * off, rad + off, pack565(col));
  }
}

// ---------------------------------------------------------------- eye body
// vertical-gradient rounded-rect with top gloss + bottom inner shadow, alpha,
// brightness and a per-row shear (for E1 rotation).
static void drawEyeBody(int cx, int top, int w, int h, int rad, float rotTan,
                        int cy, float alpha, float bright) {
  if (h < 1 || w < 1) return;
  gfx->startWrite();
  for (int oy = 0; oy < h; oy++) {
    int rowY = top + oy;
    if (rowY < 0 || rowY >= SCREEN) continue;
    // rounded-rect half-width at this row
    float hw;
    if (oy < rad) {
      float dy = rad - oy; float t = (float)rad * rad - dy * dy;
      hw = w / 2.0f - (rad - (t > 0 ? sqrtf(t) : 0));
    } else if (oy >= h - rad) {
      float dy = rad - (h - 1 - oy); float t = (float)rad * rad - dy * dy;
      hw = w / 2.0f - (rad - (t > 0 ? sqrtf(t) : 0));
    } else {
      hw = w / 2.0f;
    }
    if (hw <= 0) continue;
    // gradient + gloss
    float fy = (h > 1) ? (float)oy / (h - 1) : 0.0f;
    RGB col = eyeGradient(fy);
    if (oy < 18) { float g = 0.45f * (1.0f - fabsf(oy - 7.0f) / 14.0f); if (g > 0) col = lerpRGB(col, WHT, g); }
    int bb = h - 1 - oy;
    if (bb < 22) { float g = 0.55f * (1.0f - bb / 22.0f); if (g > 0) col = lerpRGB(col, ISH, g); }
    if (bright != 1.0f) col = mulRGB(col, bright);
    if (alpha < 1.0f)  col = lerpRGB(BG, col, alpha);
    int shear = (int)((rowY - cy) * rotTan + (rowY >= cy ? 0.5f : -0.5f));
    int x0 = (int)(cx - hw + 0.5f) + shear;
    int ww = (int)(2 * hw + 0.5f);
    if (ww > 0) gfx->writeFastHLine(x0, rowY, ww, pack565(col));
  }
  gfx->endWrite();
}

// ---------------------------------------------------------------- both eyes
static void drawEyes(const Pose& p) {
  float ang = p.gRot * (float)M_PI / 180.0f;
  float ca = cosf(ang), sa = sinf(ang);
  float rotTan = tanf(ang);
  for (int side = 0; side < 2; side++) {
    bool L = (side == 0);
    float baseCx = L ? BASE_CX_L : BASE_CX_R;
    float dx0 = (baseCx - CXC) * p.gScale;           // baseCy == CYC -> dy0 = 0
    float rx = dx0 * ca, ry = dx0 * sa;              // rotate the eye center about screen center
    float ety = L ? p.etyL : p.etyR;
    int cx = (int)(CXC + rx + p.gTx + 0.5f);
    int cy = (int)(CYC + ry + p.gTy + ety + 0.5f);
    float dSy = L ? p.dSyL : p.dSyR;
    bool pivotB = L ? p.pivotBL : p.pivotBR;
    int w = (int)(EYE_W * p.gScale + 0.5f);
    int h = (int)(EYE_H * p.gScale * dSy + 0.5f);
    if (h < 2) h = 2;
    int rad = (int)(EYE_RAD * p.gScale); if (rad > h / 2) rad = h / 2; if (rad < 1) rad = 1;
    int top = pivotB ? (int)(cy + EYE_H * p.gScale / 2.0f - h + 0.5f)
                     : (int)(cy - h / 2.0f + 0.5f);
    if (p.dOpacity > 0.01f) {
      drawEyeGlow(cx, top, w, h, rad, p.dOpacity);
      drawEyeBody(cx, top, w, h, rad, rotTan, cy, p.dOpacity, p.dBright);
    }
  }
}

// ---------------------------------------------------------------- clock
static const uint8_t SEG7[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

// placeholder time source (RTC wired LATER — keep this signature).
static void getTime(uint8_t& hh, uint8_t& mm) {
  uint32_t base = 10UL * 3600 + 8UL * 60;        // start near 10:08 so digits are non-zero
  uint32_t secs = base + millis() / 1000;
  mm = (secs / 60) % 60;
  hh = (secs / 3600) % 24;
}

// one glossy glowing segment (rounded capsule) with a small bloom
static void drawSeg(int x, int y, int w, int h, int r, RGB col) {
  const int offs[3] = {6, 4, 2};
  const float ins[3] = {0.18f, 0.30f, 0.45f};
  for (int i = 0; i < 3; i++) {
    RGB g = lerpRGB(BG, GLOW, ins[i]);
    gfx->fillRoundRect(x - offs[i], y - offs[i], w + 2 * offs[i], h + 2 * offs[i], r + offs[i], pack565(g));
  }
  gfx->fillRoundRect(x, y, w, h, r, pack565(col));
}

static void drawDigit(int x, int top, int w, int h, int val) {
  if (val < 0 || val > 9) return;
  uint8_t segs = SEG7[val];
  int s = 14;                          // stroke
  int vlen = (h - 3 * s) / 2;          // vertical segment length
  int r = s / 2;
  RGB cTop = eyeGradient(0.15f), cMid = eyeGradient(0.5f), cBot = eyeGradient(0.85f);
  // horizontals (a,g,d)
  if (segs & 0x01) drawSeg(x + s, top, w - 2 * s, s, r, cTop);                       // a
  if (segs & 0x40) drawSeg(x + s, top + s + vlen, w - 2 * s, s, r, cMid);            // g
  if (segs & 0x08) drawSeg(x + s, top + 2 * (s + vlen), w - 2 * s, s, r, cBot);      // d
  // verticals (f,b top ; e,c bottom)
  if (segs & 0x20) drawSeg(x, top + s, s, vlen, r, cTop);                            // f
  if (segs & 0x02) drawSeg(x + w - s, top + s, s, vlen, r, cTop);                    // b
  if (segs & 0x10) drawSeg(x, top + 2 * s + vlen, s, vlen, r, cBot);                 // e
  if (segs & 0x04) drawSeg(x + w - s, top + 2 * s + vlen, s, vlen, r, cBot);         // c
}

static void drawClock() {
  uint8_t hh, mm; getTime(hh, mm);
  int dW = 60, dH = 155;
  int top = CYC - dH / 2;                          // 162
  // x layout: d0 d1 : d2 d3  (centered, total ~320, start 80)
  int x0 = 80, x1 = 152, xc = 228, x2 = 268, x3 = 340;
  drawDigit(x0, top, dW, dH, hh / 10);
  drawDigit(x1, top, dW, dH, hh % 10);
  drawDigit(x2, top, dW, dH, mm / 10);
  drawDigit(x3, top, dW, dH, mm % 10);
  // colon: blinks at 1 Hz (steps), dim (not gone) when off — matches clkColon
  bool on = (millis() % 1000) < 550;
  RGB cc = on ? eyeGradient(0.3f) : lerpRGB(BG, eyeGradient(0.3f), 0.12f);
  int cxx = xc + 6, dotR = 11;
  int cyA = top + 52, cyB = top + dH - 52;
  drawSeg(cxx, cyA - dotR, 2 * dotR, 2 * dotR, dotR, cc);
  drawSeg(cxx, cyB - dotR, 2 * dotR, 2 * dotR, dotR, cc);
}

// ---------------------------------------------------------------- notepad (B2)
// bottom-right corner overlay, all timings FIXED seconds (not spd-scaled).
// notepad (white, spiral on the RIGHT) + a writing hand. Fixed-time (not spd).
static void drawNotepad() {
  int w = 120, h = 100;
  int nx = (SCREEN - w) / 2;   // 180 — centered below the eyes
  int ny = SCREEN - 96 - h;    // 284
  float bp = fmodf(millis() / 2600.0f, 1.0f);
  ny += (int)(-3.0f * sinf(bp * (float)M_PI));        // bob
  RGB white = {235, 252, 249};
  RGB pad = lerpRGB(white, G1, 0.10f);
  gfx->fillRoundRect(nx, ny, w, h, 13, pack565(pad));
  // spiral binding loops on the RIGHT edge
  int ry[4] = {14, 36, 58, 78};
  for (int i = 0; i < 4; i++)
    gfx->fillRoundRect(nx + w - 8, ny + ry[i], 16, 7, 4, pack565(G2));
  // already-written lines
  RGB ln = lerpRGB(pad, ISH, 0.5f);
  gfx->fillRoundRect(nx + 16, ny + 20, (int)(w * 0.54f), 6, 3, pack565(ln));
  gfx->fillRoundRect(nx + 16, ny + 38, (int)(w * 0.66f), 6, 3, pack565(ln));
  // active write line (1.9s, 8% -> 74%)
  float wp = fmodf(millis() / 1900.0f, 1.0f);
  float frac = wp < 0.64f ? (0.08f + (0.74f - 0.08f) * (wp / 0.64f)) : 0.74f;
  gfx->fillRoundRect(nx + 16, ny + 56, (int)(w * frac), 6, 3, pack565(lerpRGB(pad, ISH, 0.72f)));
  // writing hand (white mitten + pen) sweeping along the line
  float hp = wp < 0.7f ? (wp / 0.7f) : 1.0f;
  int hx = nx - 8 + (int)(24 * hp);
  int hy = ny + h + 2;
  RGB nibC = {36, 31, 27};
  RGB pen = lerpRGB(G1, nibC, 0.45f);
  for (int o = -1; o <= 1; o++) gfx->drawLine(hx + 22 + o, hy - 8, hx + 38 + o, hy - 46, pack565(pen));
  gfx->fillCircle(hx + 38, hy - 46, 3, pack565(nibC));               // nib
  gfx->fillRoundRect(hx + 2, hy - 28, 40, 30, 14, pack565(white));   // palm
  gfx->fillCircle(hx, hy - 16, 8, pack565(white));                   // thumb
}

// ---------------------------------------------------------------- light feedback
// Maind X spec §0 · Light states. The light is EXTERNAL on the device (an RGB LED
// halo around it) — the LCD shows ONLY the eyes. So we do NOT draw it on screen;
// instead we expose the current level + tint for the LED (TODO: wire the LED).
// Speak tracks the eye breath; listen runs cooler.
enum { LUM_OFF, LUM_IDLE, LUM_SPEAK, LUM_LISTEN, LUM_WAKE, LUM_CONFIRM };
float g_lightLevel = 0.0f;   // 0..1 — drive an external RGB LED with this
bool  g_lightCool  = false;  // listen = cooler tint

static uint8_t lightModeFor(int idx) {
  switch (idx) {
    case 0:  return LUM_OFF;      // A1_off
    case 1:  return LUM_WAKE;     // A2_wake
    case 4:                       // B1_speaking
    case 8:                       // C2_didnt_catch
    case 9:  return LUM_SPEAK;    // D1_reminder
    case 5:                       // B2_listening
    case 10: return LUM_LISTEN;   // D2_wakeword
    case 7:  return LUM_CONFIRM;  // C1_confirm
    default: return LUM_IDLE;     // A3,A4,B3,E1..E5
  }
}

// brightness envelope per light mode (floor never 0 except OFF)
static float lightLevel(uint8_t mode, uint32_t now) {
  float p;
  switch (mode) {
    case LUM_OFF:     return 0.0f;
    case LUM_IDLE:    p = fmodf(now / 4600.0f, 1.0f); return 0.16f + 0.24f * (0.5f - 0.5f * cosf(p * 2 * (float)M_PI));
    case LUM_LISTEN:  p = fmodf(now / 3000.0f, 1.0f); return 0.50f + 0.10f * (0.5f - 0.5f * cosf(p * 2 * (float)M_PI));
    case LUM_SPEAK:   p = fmodf(now / 3400.0f, 1.0f); { float v = 0.32f + 0.42f * fabsf(sinf(p * (float)M_PI * 3.0f)); return v > 0.76f ? 0.76f : v; }
    case LUM_WAKE:    p = fmodf(now / 4000.0f, 1.0f); return p < 0.28f ? (p / 0.28f * 0.52f) : 0.30f;
    case LUM_CONFIRM: p = fmodf(now / 2400.0f, 1.0f);
                      if (p < 0.09f) return 0.28f + (0.80f - 0.28f) * (p / 0.09f);
                      if (p < 0.26f) return 0.80f - (0.80f - 0.30f) * ((p - 0.09f) / 0.17f);
                      return 0.30f;
  }
  return 0.2f;
}

// speak light tracks the eye breath (group scale) — for the external LED
static float speakLightFromScale(float sc) {
  float v = 0.20f + (sc - 1.0f) * 4.5f;
  return v < 0.20f ? 0.20f : (v > 0.56f ? 0.56f : v);
}

// confirm: the eyes morph into a check. drawn 0..1 = how much of the stroke is
// drawn; alpha 0..1 = its opacity. Two passes: a soft glow, then the bright line.
static void drawCheck(float drawn, float alpha) {
  if (alpha <= 0.01f) return;
  RGB col = lerpRGB(BG, G0, alpha);
  RGB glo = lerpRGB(BG, GLOW, alpha * 0.5f);
  const float ax = 173, ay = 246, bx = 223, by = 296, cx = 313, cy = 190;
  const float L1 = 70.7f, L2 = 139.0f, T = L1 + L2;
  float prog = drawn * T; if (prog > T) prog = T;
  for (int pass = 0; pass < 2; pass++) {        // glow pass, then bright pass
    int rad = pass == 0 ? 15 : 9;
    uint16_t c = pack565(pass == 0 ? glo : col);
    for (float s = 0; s <= L1 && s <= prog; s += 5) {
      float t = s / L1;
      gfx->fillCircle((int)(ax + (bx - ax) * t), (int)(ay + (by - ay) * t), rad, c);
    }
    if (prog > L1)
      for (float s = 0; s <= L2 && s <= (prog - L1); s += 5) {
        float t = s / L2;
        gfx->fillCircle((int)(bx + (cx - bx) * t), (int)(by + (cy - by) * t), rad, c);
      }
  }
}

// reminder: the eyes morph into a bell that rings. swing -1..1 = the bell tilt.
static void drawBell(float swing, float alpha) {
  if (alpha <= 0.01f) return;
  RGB col = lerpRGB(BG, G0, alpha);
  RGB glo = lerpRGB(BG, GLOW, alpha * 0.45f);
  int sx = (int)(swing * 24);
  int cx = CXC + sx, cy = 214;
  gfx->fillCircle(cx, cy + 6, 84, pack565(glo));                    // glow halo
  gfx->fillCircle(cx, cy - 60, 11, pack565(col));                   // top handle
  gfx->fillCircle(cx, cy - 4, 54, pack565(col));                    // dome
  gfx->fillRoundRect(cx - 66, cy + 28, 132, 36, 16, pack565(col));  // flared rim
  gfx->fillRoundRect(cx - 44, cy + 40, 88, 12, 6, pack565(BG));     // rim separation
  gfx->fillCircle(cx + (int)(swing * 9), cy + 80, 13, pack565(col));// clapper
}

// ---------------------------------------------------------------- background
static void drawBackground() {
  gfx->fillScreen(pack565(BG));
}

// ---------------------------------------------------------------- frame
static void renderFrame(int stateIdx, uint32_t now, uint32_t startMs) {
  const StateDef* s = &STATES[stateIdx];
  drawBackground();
  uint8_t lm = lightModeFor(stateIdx);

  if (s->dotsOff) { g_lightLevel = 0; g_lightCool = false; return; }   // A1: dark + light off
  if (s->overlay == OV_CLOCK) { g_lightLevel = lightLevel(lm, now); g_lightCool = false; drawClock(); return; }

  Pose p = evalState(s, now, startMs);

  // external light feedback (for the device LED — NOT drawn on the LCD). Speak
  // tracks the eye breath, so the halo pulses in time with the eyes.
  g_lightCool  = (lm == LUM_LISTEN);
  g_lightLevel = (lm == LUM_SPEAK) ? speakLightFromScale(p.gScale) : lightLevel(lm, now);

  if (stateIdx == 7) {                  // C1_confirm: eyes fade → check ✓ → eyes
    drawEyes(p);
    uint32_t dur = (uint32_t)(s->baseDurMs * g_spd);
    float pr = (float)(now - startMs) / dur; if (pr > 0.92f) pr = 0.92f;
    float alpha = pr < 0.12f ? pr / 0.12f
                : (pr < 0.70f ? 1.0f
                : (pr < 0.82f ? 1.0f - (pr - 0.70f) / 0.12f : 0.0f));
    float drawn = pr < 0.12f ? 0.0f : (pr < 0.40f ? (pr - 0.12f) / 0.28f : 1.0f);
    drawCheck(drawn, alpha);
    return;
  }

  if (stateIdx == 9) {                  // D1_reminder: eyes fade → bell rings → eyes
    drawEyes(p);
    uint32_t dur = (uint32_t)(s->baseDurMs * g_spd);
    float pr = (float)(now - startMs) / dur; if (pr > 0.94f) pr = 0.94f;
    float alpha = pr < 0.10f ? pr / 0.10f
                : (pr < 0.82f ? 1.0f
                : (pr < 0.92f ? 1.0f - (pr - 0.82f) / 0.10f : 0.0f));
    float swing = 0.0f;
    if (pr >= 0.10f && pr < 0.58f) {
      float u = (pr - 0.10f) / 0.48f;
      swing = cosf(u * (float)M_PI * 5.0f) * (1.0f - u);   // damped ring
    }
    drawBell(swing, alpha);
    return;
  }

  drawEyes(p);
  if (s->overlay == OV_NOTEPAD) drawNotepad();
}
