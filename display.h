#pragma once
#include "config.h"
#include <Adafruit_SSD1306.h>

// ===============================================================
//  display.h
//  Handles OLED UI for both main screen and menus.
//  Provides toast messages (displayShowMessage / displayShowRecall).
// ===============================================================

// The global OLED object (defined in display.cpp)
extern Adafruit_SSD1306 display;

// ===== Core functions =====
void setupDisplay();

// Draws the main screen (stomp/preset info, loops, bank, BPM, etc.)
void drawMainUI();

// Draws the settings / edit menu (implemented in menu.cpp)
void drawMenuUI();

// ===== Toast helpers =====

// Displays a short message popup for `ms` milliseconds (centered on screen).
// Example: displayShowMessage("Saved", 800);
void displayShowMessage(const char* msg, uint16_t ms);

// Displays a short "Recall Bx Sy" popup when recalling presets.
void displayShowRecall(int bank, int slot);
