// ===================== midi.cpp =====================
#include "config.h"
#include "midi.h"
#include <MIDI.h>
#include "leds.h"

// Create the MIDI instance (library default)
MIDI_CREATE_DEFAULT_INSTANCE();

// ---- extern globals (defined in MIDI_Switcher.ino) ----
extern byte     midiChannel;        // 1..16
extern bool     midiClockEnabled;   // on/off
extern uint16_t currentBPM;         // live BPM

// ---- local clock state ----
static bool     clockRunning = false;
static uint32_t lastTickMs   = 0;   // when we last sent a 24-PPQN tick

// Count outgoing MIDI clock ticks (24 ticks per quarter note)
static uint8_t  clockTick24  = 0;   // 0..23

// ---------------- API ----------------
void setupMIDI() {
  // Start MIDI on the chosen channel; no THRU for a cleaner output
  MIDI.begin(midiChannel);
  MIDI.turnThruOff();
}

void sendPC(byte pc) {
  MIDI.sendProgramChange(pc, midiChannel);
}

void sendCC(byte cc, byte value) {
  MIDI.sendControlChange(cc, value, midiChannel);
}

void midiClockStart() {
  if (!midiClockEnabled) return;
  clockRunning = true;
  lastTickMs   = millis();
  clockTick24  = 0;                 // keep LED in sync from the start
  MIDI.sendRealTime(midi::Start);
}

void midiClockStop() {
  if (!clockRunning) return;
  MIDI.sendRealTime(midi::Stop);
  clockRunning = false;
  clockTick24  = 0;
}

// Call this every loop()
void updateMidiClock() {
  if (!midiClockEnabled || !clockRunning) return;
  if (currentBPM < 1) return;

  // 24 PPQN. interval_ms = 60000 / (BPM * 24)
  const uint32_t interval = 60000UL / (uint32_t(currentBPM) * 24UL);
  const uint32_t now = millis();

  // Use accumulation to reduce drift on long frames
  while ((now - lastTickMs) >= interval) {
    lastTickMs += interval;
    MIDI.sendRealTime(midi::Clock);

    // Blink TAP LED once per quarter note (every 24 clock ticks)
    clockTick24++;
    if (clockTick24 >= 24) {
      clockTick24 = 0;
      pulseTapLed(40); // ms on-time; tweak 30–80ms to taste
    }
  }
}
