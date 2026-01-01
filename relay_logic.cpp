// ===================== relay_logic.cpp =====================
#include "relay_logic.h"

// Relay pins from config
static const int relayPins[NUM_RELAYS] = RELAY_PINS;

// Extern globals (defined in MIDI_Switcher.ino)
extern bool loopStates[NUM_RELAYS];
extern bool relay5_isAmpSwitch; // mode flag (amp/loop) – hardware drive is same

// If your wiring needs inverted logic (LOW = ON), set this to 0.
#ifndef RELAY_ACTIVE_HIGH
#define RELAY_ACTIVE_HIGH 1
#endif

static inline void writeRelayPin(int pin, bool on) {
#if RELAY_ACTIVE_HIGH
  digitalWrite(pin, on ? HIGH : LOW);
#else
  digitalWrite(pin, on ? LOW : HIGH);
#endif
}

void setupRelays() {
  for (int i = 0; i < NUM_RELAYS; ++i) {
    pinMode(relayPins[i], OUTPUT);
    writeRelayPin(relayPins[i], false); // all OFF
    loopStates[i] = false;
  }
  relay5_isAmpSwitch = false;
}

void updateRelays() {
  for (int i = 0; i < NUM_RELAYS; ++i) {
    writeRelayPin(relayPins[i], loopStates[i]);
  }
}

void setLoop(int loopNum, bool state) {
  if (loopNum < 0 || loopNum >= NUM_RELAYS) return;
  loopStates[loopNum] = state;
  writeRelayPin(relayPins[loopNum], state);
}

void toggleLoop(int loopNum) {
  if (loopNum < 0 || loopNum >= NUM_RELAYS) return;
  setLoop(loopNum, !loopStates[loopNum]);
}

byte encodeLoopStates(bool loops[NUM_RELAYS]) {
  byte b = 0;
  for (int i = 0; i < NUM_RELAYS; ++i) {
    if (loops[i]) b |= (1 << i);
  }
  return b;
}

void decodeLoopStates(byte encoded, bool loops[NUM_RELAYS]) {
  for (int i = 0; i < NUM_RELAYS; ++i) {
    loops[i] = (encoded >> i) & 0x01;
  }
}
