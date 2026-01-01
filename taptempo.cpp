// ===================== taptempo.cpp =====================
#include "config.h"
#include "taptempo.h"
#include "midi.h"
#include "leds.h"

// ---- Extern globals (defined in MIDI_Switcher.ino) ----
extern uint16_t currentBPM;
extern bool     midiClockEnabled;

// ---- Local config ----
static const uint8_t  TAP_COUNT      = 4;     // rolling average size
static const uint16_t BPM_MIN        = 40;    // matches menu/encoder limits
static const uint16_t BPM_MAX        = 300;
static const uint16_t TAP_TIMEOUT_MS = 2000;  // reset taps if idle this long

// ---- Local state ----
static uint32_t taps[TAP_COUNT] = {0,0,0,0};
static uint8_t  tapIdx          = 0;
static uint8_t  tapFilled       = 0;

static int      lastTapPin      = HIGH;
static uint32_t lastTapReadMs   = 0;

// Convert average inter-tap interval (ms) to BPM, clamped
static inline uint16_t intervalMsToBpm(uint32_t intervalMs) {
  if (intervalMs == 0) return currentBPM;
  // BPM = 60000 / ms
  uint32_t bpm = 60000UL / intervalMs;
  if (bpm < BPM_MIN) bpm = BPM_MIN;
  if (bpm > BPM_MAX) bpm = BPM_MAX;
  return (uint16_t)bpm;
}

// Register a tap from any source (poller, panel button, etc.)
void taptempoTap() {
  pulseTapLed(60);
  const uint32_t now = millis();

  // Reset sequence if we've been idle too long
  if (tapFilled > 0 && (now - taps[(tapIdx + TAP_COUNT - 1) % TAP_COUNT]) > TAP_TIMEOUT_MS) {
    tapIdx = 0;
    tapFilled = 0;
  }

  // Store this tap time
  taps[tapIdx] = now;
  tapIdx = (tapIdx + 1) % TAP_COUNT;
  if (tapFilled < TAP_COUNT) tapFilled++;

  // Need at least 2 taps to compute 1 interval, better with more
  if (tapFilled >= 2) {
    // Compute average of available intervals
    uint32_t sumIntervals = 0;
    uint8_t  intervals    = 0;

    uint8_t count = tapFilled;
    for (uint8_t i = 1; i < count; ++i) {
      uint32_t newer = taps[(tapIdx + TAP_COUNT - i) % TAP_COUNT];
      uint32_t older = taps[(tapIdx + TAP_COUNT - i - 1) % TAP_COUNT];
      uint32_t diff  = newer - older;
      // Ignore clearly bogus long gaps (idle reset above handles most cases)
      if (diff > 0 && diff <= TAP_TIMEOUT_MS) {
        sumIntervals += diff;
        intervals++;
      }
    }

    if (intervals > 0) {
      uint32_t avgMs = sumIntervals / intervals;
      currentBPM = intervalMsToBpm(avgMs);

      // If clock is enabled, (re)start it at this BPM
      // The clock tick rate will follow 'currentBPM' via updateMidiClock()
      if (midiClockEnabled) {
        midiClockStart();
      }
    }
  }
}

// Optional poller if you want this module to read the pin directly.
// In your current build, pollInputs() already reads TAP_TEMPO_PIN and calls taptempoTap(),
// so you can skip calling checkTapTempo() to avoid duplicate handling.
void checkTapTempo() {
  const uint32_t now = millis();
  int t = digitalRead(TAP_TEMPO_PIN);
  if (t != lastTapPin && (now - lastTapReadMs) > DEBOUNCE_MS) {
    lastTapReadMs = now;
    if (t == LOW) taptempoTap();
    lastTapPin = t;
  }
}
