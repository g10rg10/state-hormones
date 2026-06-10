// panel_config.h — Waveshare ESP32-S3-Touch-LCD-2.8C panel bring-up.
//
//   *** THIS IS THE ONLY FILE YOU MUST VERIFY AGAINST YOUR BOARD. ***
//
// Everything else (engine, states, drawing) is hardware-agnostic and draws
// through the `gfx` pointer defined here. The pins / timing below are the
// CONFIRMED values for this board; the two things that vary and CAN leave the
// screen black are flagged *** COPY FROM WAVESHARE EXAMPLE ***:
//   1) the ST7701 init-operations array (3-wire SPI init), and
//   2) the TCA9554 I/O-expander reset sequence (ST7701-CS / LCD-RST live on it).
//
// Requirements:
//   - Arduino_GFX library installed (moononournation/Arduino_GFX).
//   - Arduino_GFX's ESP32-S3 RGB driver needs arduino-esp32 core 2.0.x.
//     (Waveshare ships 3.0.2 with the Espressif panel stack; if you stay on 3.x,
//      use their ESP32_Display_Panel example for bring-up and keep `gfx` valid.)
//   - Board: "ESP32S3 Dev Module", PSRAM = OPI (8 MB) ENABLED, Flash 16 MB.
#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>

// ---- RGB data + sync pins (CONFIRMED) ----
#define LCD_DE     40
#define LCD_VSYNC  39
#define LCD_HSYNC  38
#define LCD_PCLK   41
#define LCD_R0     46
#define LCD_R1      3
#define LCD_R2      8
#define LCD_R3     18
#define LCD_R4     17
#define LCD_G0     14
#define LCD_G1     13
#define LCD_G2     12
#define LCD_G3     11
#define LCD_G4     10
#define LCD_G5      9
#define LCD_B0      5
#define LCD_B1     45
#define LCD_B2     48
#define LCD_B3     47
#define LCD_B4     21
#define LCD_BL      6     // backlight (active high)

// ---- 3-wire SPI used only to send the ST7701 init (CS is on the expander) ----
#define LCD_SCK     2
#define LCD_SDA     1

// ---- RGB timing (CONFIRMED official default) ----
#define PCLK_HZ     (18UL * 1000 * 1000)   // 18 MHz
#define HSYNC_PW    8
#define HSYNC_BP    10
#define HSYNC_FP    50
#define VSYNC_PW    2
#define VSYNC_BP    18
#define VSYNC_FP    8
#define PCLK_NEG    0      // PCLK active on rising edge

// ===================================================================
// *** COPY FROM WAVESHARE EXAMPLE #1: ST7701 init operations array ***
// Paste your board's ST7701 init table here (Arduino_GFX `st7701_type*_init
// _operations` or the Waveshare example's). Until then the panel won't light.
// ===================================================================
static const uint8_t* ST7701_INIT = nullptr;   // <-- replace with your init array
static const size_t   ST7701_INIT_LEN = 0;     // <-- and its length

// ---- gfx object (Arduino_GFX path) ----
static Arduino_DataBus* bus = new Arduino_SWSPI(
  GFX_NOT_DEFINED /*DC*/, GFX_NOT_DEFINED /*CS = EXIO2 on expander*/, LCD_SCK, LCD_SDA, GFX_NOT_DEFINED);

static Arduino_ESP32RGBPanel* rgbpanel = new Arduino_ESP32RGBPanel(
  LCD_DE, LCD_VSYNC, LCD_HSYNC, LCD_PCLK,
  LCD_R0, LCD_R1, LCD_R2, LCD_R3, LCD_R4,
  LCD_G0, LCD_G1, LCD_G2, LCD_G3, LCD_G4, LCD_G5,
  LCD_B0, LCD_B1, LCD_B2, LCD_B3, LCD_B4,
  1 /*hsync_polarity*/, HSYNC_FP, HSYNC_PW, HSYNC_BP,
  1 /*vsync_polarity*/, VSYNC_FP, VSYNC_PW, VSYNC_BP,
  PCLK_NEG, PCLK_HZ);

// Arduino_RGB_Display is-a Arduino_GFX. auto_flush=true => draws land in the
// scanned-out PSRAM framebuffer directly (no manual flush; matches "Model A").
Arduino_GFX* gfx = new Arduino_RGB_Display(
  480, 480, rgbpanel, 0 /*rotation*/, true /*auto_flush*/,
  bus, GFX_NOT_DEFINED /*rst on expander*/, ST7701_INIT, ST7701_INIT_LEN);

// ===================================================================
// *** COPY FROM WAVESHARE EXAMPLE #2: TCA9554 expander reset ***
// The ST7701 CS = EXIO2, LCD reset = EXIO0, touch reset = EXIO1 (addr 0x20 on
// I2C SDA=15 / SCL=7). Verify the EXIO mapping against your example — EXIO0=
// LCD-RST is corroborated only by ESPHome. Adjust the bit values if black.
// ===================================================================
static void expanderInit() {
  Wire.begin(15, 7);
  // TCA9554: reg 0x03 = config (0 => output). All EXIO as outputs.
  Wire.beginTransmission(0x20); Wire.write(0x03); Wire.write(0x00); Wire.endTransmission();
  auto setOut = [](uint8_t v) {
    Wire.beginTransmission(0x20); Wire.write(0x01); Wire.write(v); Wire.endTransmission();
  };
  setOut(0b00000000); delay(20);   // assert resets (EXIO0/1 low), CS low
  setOut(0b00000011); delay(20);   // release LCD-RST + TP-RST (EXIO0/1 high), keep CS (EXIO2) low/active
}

static void panelInit() {
  expanderInit();                  // bring ST7701 CS / resets out of the expander FIRST
  gfx->begin(PCLK_HZ);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);      // backlight on
}
