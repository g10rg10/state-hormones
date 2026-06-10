// Hormy_eyes.ino — EMO eyes + tall clock on the Waveshare ESP32-S3-Touch-LCD-
// 2.8C (480x480 round, ST7701), Arduino + Arduino_GFX. Faithful port of the web
// gallery: all 16 states, round-only, placeholder clock (RTC wired LATER).
//
// Out of the box this runs a DEMO that cycles through every state (like the web
// gallery). A host can instead call setState(i) to drive a specific state; the
// chain info (A2->A3, D1->B1, D2->B2) lives in the state table for that use.
//
// Fluidity: the render loop is paced ~30 fps and ALL timelines are driven from
// the wall clock (millis), with per-segment cubic-bezier easing — so motion is
// smooth and framerate-independent, matching EMO's eased blinks + continuous
// micro-life. See firmware/README.md.
#include "palette.h"
#include "easing.h"
#include "anim.h"
#include "timelines.h"
#include "panel_config.h"     // defines gfx (VERIFY against your board)
#include "draw.h"

float g_spd = 1.0f;            // global tempo (the web "Tempo" slider); 1.0 = as authored

// ---- demo driver: cycle through all states, gallery-style ----
static int      demoIdx   = 2;        // A3_idle
static uint32_t stateStart = 0;
static uint32_t lastFrame  = 0;

static uint32_t dwellFor(int idx) {
  const StateDef* s = &STATES[idx];
  if (s->overlay == OV_CLOCK) return 8000;     // lingers on the clock
  if (s->mode == MODE_LOOP)   return 6000;
  if (s->mode == MODE_STATIC) return 2500;
  return 4500;                                  // one-shots: play + hold a moment
}

// host hook: jump to a state by index (0..NUM_STATES-1)
void setState(int idx) {
  if (idx < 0 || idx >= NUM_STATES) return;
  demoIdx = idx;
  stateStart = millis();
}

void setup() {
  Serial.begin(115200);
  panelInit();
  gfx->fillScreen(pack565(BG));
  setState(2);
}

void loop() {
  uint32_t now = millis();

  // advance the demo (remove this block when a host drives setState())
  if (now - stateStart >= dwellFor(demoIdx)) {
    demoIdx = (demoIdx + 1) % NUM_STATES;
    stateStart = now;
  }

  renderFrame(demoIdx, now, stateStart);

  // ~30 fps pacing (smooth; timing itself is wall-clock based above)
  uint32_t dt = millis() - lastFrame;
  if (dt < 33) delay(33 - dt);
  lastFrame = millis();
}
