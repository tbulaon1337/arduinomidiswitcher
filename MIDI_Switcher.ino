// ===================== MIDI_Switcher.ino =====================
#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "config_store.h"
#include "display.h"
#include "encoder.h"
#include "menu.h"
#include "midi.h"
#include "presets.h"
#include "relay_logic.h"
#include "taptempo.h"
#include "leds.h"


// ====== Global state (define once here; declared extern in headers) ======
bool        isStompboxMode      = false;   // runtime mode
int         currentBank          = 0;       // 0..NUM_BANKS-1
int         selectedSlot         = 0;       // 0..PRESETS_PER_BANK-1

bool        loopStates[NUM_RELAYS] = { false, false, false, false, false };
bool        relay5_isAmpSwitch     = false; // set by preset.relay5Mode
Preset      activePreset;
bool        haveActivePreset = false;

// Global config (persisted via config_store.*)
int         altBank            = 0;         // kept for menu compatibility
bool        startupIsStomp     = false;     // persisted startup mode
int         startupBank        = 0;         // persisted startup bank
uint16_t    defaultBPM         = 120;       // optional default BPM

// MIDI globals
byte        midiChannel        = 1;         // 1..16
bool        midiClockEnabled   = false;     // on/off
uint16_t    currentBPM         = 120;       // live BPM for clock/tap

// Menu state lives here (referenced by menu.cpp)
MenuState   menuState          = MENU_NONE;

// ============================== Expression pedal ==============================
// Per-preset EXP settings live inside activePreset (loaded on recall / save)
// EXP sends CC only when activePreset.expEnable == 1

// Calibration / runtime state (you can later persist calibration if you want)
static uint16_t expMinRaw   = 0;
static uint16_t expMaxRaw   = 1023;

static uint16_t expSmoothed = 0;
static byte     expLastCC   = 255;   // 255 = "unset"
static uint32_t expLastSend = 0;     // ms throttle

static inline void setupExpression() {
  pinMode(EXP_PEDAL_PIN, INPUT);
  expSmoothed = analogRead(EXP_PEDAL_PIN);
  expLastCC   = 255;
  expLastSend = 0;
}

static inline void pollExpression() {
  // Need a valid active preset first
  if (!haveActivePreset) return;

  // Only send if this preset enables EXP
  if (activePreset.expEnable == 0) return;

  // CC number (safe fallback)
  byte ccNum = activePreset.expCC;
  if (ccNum > 127) ccNum = 11;
  if (ccNum == 0)  ccNum = 11; // optional: treat 0 as "unset"

  // Read raw (0..1023)
  uint16_t raw = analogRead(EXP_PEDAL_PIN);

  // Smooth: expSmoothed = 0.75*old + 0.25*new
  expSmoothed = (expSmoothed * 3 + raw) >> 2;

  // Clamp to calibration window
  uint16_t lo = (expMinRaw < expMaxRaw) ? expMinRaw : expMaxRaw;
  uint16_t hi = (expMinRaw < expMaxRaw) ? expMaxRaw : expMinRaw;
  uint16_t clamped = expSmoothed;
  if (clamped < lo) clamped = lo;
  if (clamped > hi) clamped = hi;

  // Map to 0..127
  uint16_t span = (hi > lo) ? (hi - lo) : 1;
  uint16_t pos  = (uint32_t)(clamped - lo) * 127UL / span;
  byte ccVal    = (byte)pos;

  // Optional invert (per preset)
  if (activePreset.expInvert) ccVal = 127 - ccVal;

  // Send only when changed, throttle a little
  uint32_t now = millis();
  if (ccVal != expLastCC && (now - expLastSend) > 8) {
    sendCC(ccNum, ccVal);
    expLastCC   = ccVal;
    expLastSend = now;
  }
}


// ============================== Forward decls ==============================
void setupDisplay();
void drawMainUI();      // display.cpp
void drawMenuUI();      // menu.cpp

void setupRelays();     // relay_logic.cpp
void updateRelays();

void setupMIDI();       // midi.cpp
void updateMidiClock();

void setupEncoder();    // encoder.cpp
void checkEncoder();

void setupInputs();     // menu.cpp (FS1..FS5, TAP_TEMPO_PIN, MODE_BTN_PIN)
void pollInputs();      // includes FS1+FS2 / FS4+FS5 bank combos
void checkMenuButtons(); // BTN_BACK/BTN_SAVE

void loadGlobalConfig();  // config_store.cpp
void saveGlobalConfig();

// ============================== SETUP ==============================
void setup() {
  // Serial.begin(115200);  // (optional for debugging)
  Wire.begin();

  setupDisplay();
  setupRelays();
  setupMIDI();

  //Set up LEDS
  setupLeds();
  refreshLeds();

  // Load persisted configuration (startup mode/bank, MIDI, altBank, etc.)
  loadGlobalConfig();

  // Apply startup choices to runtime
  isStompboxMode = startupIsStomp;
  refreshLeds();
  // DEBUG: force stompbox mode on boot
  //isStompboxMode = true;
  //refreshLeds();
  currentBank    = constrain(startupBank, 0, NUM_BANKS - 1);
  selectedSlot   = constrain(selectedSlot, 0, PRESETS_PER_BANK - 1);

  // Controls
  setupInputs();      // 5 FS + TAP + MODE (no panel bank buttons)
  setupEncoder();     // rotary + push
  setupExpression();  // expression pedal analog in

  // Optionally boot directly into the current preset’s sound
 if (!isStompboxMode) {
  Preset p = loadPreset(currentBank, selectedSlot);
  activePreset = p;
  haveActivePreset = true;
  applyPresetToHardware(p);
}

  drawMainUI();
}

// =============================== LOOP ==============================
void loop() {
  // Inputs
  pollInputs();         // FS1..FS5, Tap, Mode (+ FS combos for bank up/down)
  checkEncoder();       // menu navigation
  checkMenuButtons();   // Back/Save

  // Continuous controls
  pollExpression();     // reads EXP_PEDAL_PIN and sends CC

  // Time-based systems
  updateMidiClock();    // 24 PPQN, gated by midiClockEnabled
  
  //Set up tap tempo LED
  tickLeds();

  // UI
  if (menuState == MENU_NONE) {
    drawMainUI();
  } else {
    drawMenuUI();
  }
}
