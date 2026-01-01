#pragma once
#include "config.h"

// Setup LED pins as outputs
void setupLeds();

// Call often (each loop) to service tap LED pulse timing
void tickLeds();

// Update loop LEDs + mode LED based on current runtime state
void refreshLeds();

// Pulse the tap LED briefly (non-blocking)
void pulseTapLed(uint16_t ms = 60);
