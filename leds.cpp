#include "leds.h"
#include <Arduino.h>

// ---- externs from your runtime (defined in MIDI_Switcher.ino) ----
extern bool     isStompboxMode;
extern int      selectedSlot;
extern bool     loopStates[NUM_RELAYS];

// ---- LED pins ----
static const int loopLedPins[NUM_RELAYS] = LOOP_LED_PINS;

// ---- tap LED pulse state ----
static uint32_t tapLedOffAt = 0;

// ---- IMPORTANT: LED wiring ----
// Your symptom indicates ACTIVE-LOW LEDs (pin sinks current).
static inline void writeLed(int pin, bool on) {
  digitalWrite(pin, on ? LOW : HIGH);
}

void setupLeds() {
  // Loop/Preset LEDs
  for (int i = 0; i < NUM_RELAYS; ++i) {
    pinMode(loopLedPins[i], OUTPUT);
    writeLed(loopLedPins[i], false);
  }

  // Status LEDs
  pinMode(TAP_LED_PIN, OUTPUT);
  pinMode(MODE_LED_PIN, OUTPUT);
  writeLed(TAP_LED_PIN, false);
  writeLed(MODE_LED_PIN, false);
}

void pulseTapLed(uint16_t ms) {
  writeLed(TAP_LED_PIN, true);
  tapLedOffAt = millis() + ms;
}

void tickLeds() {
  if (tapLedOffAt != 0 && millis() >= tapLedOffAt) {
    writeLed(TAP_LED_PIN, false);
    tapLedOffAt = 0;
  }
}

void refreshLeds() {
  // Mode LED: ON = stompbox overlay, OFF = preset mode
  writeLed(MODE_LED_PIN, isStompboxMode);

  if (isStompboxMode) {
    // In stomp mode show loop on/off
    for (int i = 0; i < NUM_RELAYS; ++i) {
      writeLed(loopLedPins[i], loopStates[i]);
    }
  } else {
    // In preset mode show active slot (one-hot)
    for (int i = 0; i < NUM_RELAYS; ++i) {
      writeLed(loopLedPins[i], (i == selectedSlot));
    }
  }
}
