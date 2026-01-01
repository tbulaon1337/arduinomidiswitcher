#pragma once
#include "config.h"

// ===============================================================
//  encoder.h
//  Handles rotary encoder input for menu navigation and BPM control.
//  - Rotation while menu is open → navigates menu items
//  - Rotation while menu is closed + stompbox mode active → adjusts BPM
//  - Press to enter/confirm menu
// ===============================================================

void setupEncoder();
void checkEncoder();
