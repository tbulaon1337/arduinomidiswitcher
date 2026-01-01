#pragma once
#include "config.h"

int     getPresetIndex(int bank, int slot);
int     getPresetAddress(int bank, int slot);

void    savePreset(int bank, int slot, const Preset &p);
Preset  loadPreset(int bank, int slot);

void    applyPresetToHardware(const Preset &p);
Preset  makePresetFromCurrentState(byte pc, byte cc, byte val);
void    saveCurrentStateToPreset(int bank, int slot, byte pc, byte cc, byte val);

// ---- Preset names (stored separately in EEPROM) ----
void    savePresetName(int bank, int slot, const char* name);
void    loadPresetName(int bank, int slot, char* outName, int outLen);
