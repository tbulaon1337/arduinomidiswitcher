#pragma once
#include "config.h"
#include "leds.h"

// ===============================================================
//  taptempo.h
//  Tap-tempo handling for BPM + MIDI clock
//  - taptempoTap(): register a tap from any source (switch, code)
//  - checkTapTempo(): optional poller for TAP_TEMPO_PIN (active-LOW)
//    (If pollInputs() already calls taptempoTap(), you can skip this.)
// ===============================================================

void taptempoTap();
void checkTapTempo();  // optional; safe to leave unused
