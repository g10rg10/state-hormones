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
static const int EYE_W = 118, EYE_H = 150, EYE_RAD = 46;
static const int GAP = 62;                     // between the inner edges
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
static void drawNotepad() {
  int nx = SCREEN - 60 - 96;     // 324
  int ny = SCREEN - 64 - 112;    // 304
  // bob 2.6s: ty 0 -> -3 -> 0
  float bp = fmodf(millis() / 2600.0f, 1.0f);
  int bob = (int)(-3.0f * sinf(bp * (float)M_PI));   // 0 -> -3 -> 0
  ny += bob;
  // pad
  RGB pad = lerpRGB(G0, G1, 0.35f);
  gfx->fillRoundRect(nx, ny, 96, 112, 12, pack565(pad));
  // spiral binding dots
  RGB bind = lerpRGB(pad, ISH, 0.55f);
  for (int bx = nx + 14; bx < nx + 84; bx += 18) gfx->fillCircle(bx, ny + 11, 3, pack565(bind));
  // written lines
  RGB ln = lerpRGB(pad, ISH, 0.55f);
  gfx->fillRoundRect(nx + 14, ny + 30, (int)(96 * 0.64f), 6, 3, pack565(ln));   // l1
  gfx->fillRoundRect(nx + 14, ny + 46, (int)(96 * 0.52f), 6, 3, pack565(ln));   // l2
  // active write line 1.9s: width 8% -> 80%
  float wp = fmodf(millis() / 1900.0f, 1.0f);
  float frac = wp < 0.68f ? (0.08f + (0.80f - 0.08f) * (wp / 0.68f)) : 0.80f;
  RGB lnA = lerpRGB(pad, ISH, 0.8f);
  int wl = (int)(96 * frac);
  gfx->fillRoundRect(nx + 14, ny + 64, wl, 6, 3, pack565(lnA));
  // pen nib sweeping with the write line (tx 0 -> 40)
  float pp = wp < 0.68f ? (wp / 0.68f) : 1.0f;
  int penX = nx + 16 + (int)(40 * pp);
  RGB pen = lerpRGB({58, 48, 38}, GLOW, 0.2f);
  gfx->fillCircle(penX, ny + 66, 4, pack565(pen));
  gfx->drawLine(penX, ny + 66, penX - 9, ny + 50, pack565(lerpRGB({90, 74, 56}, G0, .3f)));
}

// ---------------------------------------------------------------- background
static void drawBackground() {
  gfx->fillScreen(pack565(BG));
}

// ---------------------------------------------------------------- frame
static void renderFrame(int stateIdx, uint32_t now, uint32_t startMs) {
  const StateDef* s = &STATES[stateIdx];
  drawBackground();
  if (s->dotsOff) return;                       // A1_off: dark screen
  if (s->overlay == OV_CLOCK) { drawClock(); return; }
  Pose p = evalState(s, now, startMs);
  drawEyes(p);
  if (s->overlay == OV_NOTEPAD) drawNotepad();
}
