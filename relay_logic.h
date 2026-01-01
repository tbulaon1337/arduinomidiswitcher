#pragma once
#include "config.h"

// ===============================================================
//  relay_logic.h
//  Low-level control for the 5 relay loops.
//  - setupRelays(): configure pins and set all relays OFF
//  - setLoop(i, state): set a single loop ON/OFF (0-based index)
//  - toggleLoop(i): convenience toggle
//  - updateRelays(): push loopStates[] to hardware
//  - encode/decode helpers: pack/unpack loopStates into a byte
// ===============================================================

void setupRelays();
void updateRelays();

void setLoop(int loopNum, bool state);
void toggleLoop(int loopNum);

byte encodeLoopStates(bool loops[NUM_RELAYS]);
void decodeLoopStates(byte encoded, bool loops[NUM_RELAYS]);
