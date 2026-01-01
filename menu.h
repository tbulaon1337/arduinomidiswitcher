#pragma once
#include "config.h"

// ===============================================================
//  menu.h
//  Input scanning + on-screen settings menu
//  - Scans FS1–FS5, TAP_TEMPO_PIN, MODE_BTN_PIN
//  - FS combos: FS1+FS2 = Bank-, FS4+FS5 = Bank+
//  - Menu navigation via encoder (rotate/select) + Back/Save tactiles
// ===============================================================

// Open/close menu and react to encoder actions
void menuEnter();
void menuExit();
void menuOnEncoder(bool dirCW);
void menuOnSelect();

// Input setup & polling
void setupInputs();      // pinMode() for FS, Tap, Mode, Encoder, Back/Save
void pollInputs();       // handles FS presses, combos, Tap, Mode toggle (persists)
void checkMenuButtons(); // Back/Save handlers

// Drawing (implemented in display/menu modules)
void drawMenuUI();
