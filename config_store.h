#pragma once
#include "config.h"

// ===============================================================
//  config_store.h
//  Handles saving and loading of global settings to EEPROM.
//  Stores the following fields (see config_store.cpp):
//   - startupIsStomp
//   - startupBank
//   - altBank
//   - midiChannel
//   - midiClockEnabled
//   - defaultBPM
//  Uses versioned struct + CRC32 for data integrity.
// ===============================================================

void saveGlobalConfig();
void loadGlobalConfig();
