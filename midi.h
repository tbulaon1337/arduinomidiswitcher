#pragma once
#include "config.h"

// ===============================================================
//  midi.h
//  Handles outbound MIDI for PC, CC, and MIDI Clock
//  - Sends Program Change and Control Change
//  - Sends 24 PPQN MIDI Clock when enabled
//  - Controlled by midiClockEnabled + midiClockStart()/Stop()
// ===============================================================

// Setup and core I/O
void setupMIDI();
void sendPC(byte pc);
void sendCC(byte cc, byte value);

// MIDI Clock controls
void midiClockStart();
void midiClockStop();
void updateMidiClock();

