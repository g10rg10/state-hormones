// Hormy_eyes.ino — EMO eyes + tall clock on the Waveshare ESP32-S3-Touch-LCD-
// 2.8C (480x480 round, ST7701), Arduino + Arduino_GFX.
//
// Device lifecycle (logic): power on -> Wake -> Idle (face). After 5 min of
// inactivity -> Clock. The wake word ("Hey Arduino") -> Wake word -> Listening
// -> back to the face. Call wakeWord() from the mic/host. The States web page
// simulates this with its "Simulate flow" button.
//   setStateIdx(i) pins any single state (host control / testing);
//   resumeLifecycle() hands control back to the lifecycle.
//
// Fluidity: ~30 fps loop, all timelines wall-clock driven (millis) with
// per-segment cubic-bezier easing. See firmware/README.md and
// docs/animations-sounds-lights.md.
#include "palette.h"
#include "easing.h"
#include "anim.h"
#include "timelines.h"
#include "panel_config.h"     // defines gfx (VERIFY against your board)
#include "draw.h"

float g_spd = 1.0f;            // global tempo (the web "Tempo" slider); 1.0 = as authored

// ---- device lifecycle ----
enum { PH_WAKE, PH_IDLE, PH_CLOCK, PH_WAKEWORD, PH_LISTEN };
// state indices into STATES[] used by the lifecycle:
enum { S_A2_WAKE = 1, S_A3_IDLE = 2, S_A4_CLOCK = 3, S_B2_LISTEN = 5, S_D2_WAKEWORD = 10 };

static int      g_phase = PH_WAKE;
static uint32_t g_phaseStart = 0;
static int      g_override = -1;       // host pins a single state (-1 = run the lifecycle)
static uint32_t g_overrideStart = 0;
static uint32_t lastFrame = 0;

const uint32_t IDLE_TO_CLOCK_MS  = 5UL * 60 * 1000;  // 5 min of inactivity -> clock
const uint32_t CLOCK_AUTOWAKE_MS = 15000;            // demo: auto-wake (no mic yet); set 0 for real mic

static void setPhase(int p) { g_phase = p; g_phaseStart = millis(); }

// ---- hooks for the mic / host ----
void wakeWord()         { g_override = -1; setPhase(PH_WAKEWORD); }   // "Hey Arduino"
void resumeLifecycle()  { g_override = -1; setPhase(PH_IDLE); }
void setStateIdx(int i) { if (i >= 0 && i < NUM_STATES) { g_override = i; g_overrideStart = millis(); } }

void setup() {
  Serial.begin(115200);
  panelInit();
  gfx->fillScreen(pack565(BG));
  setPhase(PH_WAKE);
}

void loop() {
  uint32_t now = millis();
  int idx; uint32_t start;

  if (g_override >= 0) {                 // host pinned a single state
    idx = g_override; start = g_overrideStart;
  } else {
    switch (g_phase) {
      case PH_WAKE:                                  // A2_wake -> idle
        idx = S_A2_WAKE; start = g_phaseStart;
        if (oneShotDone(&STATES[S_A2_WAKE], now, g_phaseStart)) setPhase(PH_IDLE);
        break;
      case PH_IDLE:                                  // A3_idle -> clock after 5 min idle
        idx = S_A3_IDLE; start = g_phaseStart;
        if (now - g_phaseStart >= IDLE_TO_CLOCK_MS) setPhase(PH_CLOCK);
        break;
      case PH_CLOCK:                                 // A4_clock -> (wake word) face
        idx = S_A4_CLOCK; start = g_phaseStart;
        if (CLOCK_AUTOWAKE_MS && now - g_phaseStart >= CLOCK_AUTOWAKE_MS) wakeWord();
        break;
      case PH_WAKEWORD:                              // D2_wakeword -> listening
        idx = S_D2_WAKEWORD; start = g_phaseStart;
        if (oneShotDone(&STATES[S_D2_WAKEWORD], now, g_phaseStart)) setPhase(PH_LISTEN);
        break;
      default:                                       // PH_LISTEN: B2_listening -> face
        idx = S_B2_LISTEN; start = g_phaseStart;
        if (now - g_phaseStart >= 3000) setPhase(PH_IDLE);
        break;
    }
  }

  renderFrame(idx, now, start);

  uint32_t dt = millis() - lastFrame;          // ~30 fps pacing
  if (dt < 33) delay(33 - dt);
  lastFrame = millis();
}
