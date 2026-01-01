// ===================== encoder.cpp =====================
#include "config.h"
#include "menu.h"

static int lastClk = HIGH;
static uint32_t lastStepMs = 0;
static const uint16_t ENC_DEBOUNCE_MS = 2; // 1–3ms is usually sweet

void setupEncoder() {
  lastClk = digitalRead(ENCODER_CLK);
}

void checkEncoder() {
  const uint32_t now = millis();
  const int clk = digitalRead(ENCODER_CLK);

  // Count only on a clean falling edge of CLK
  if (lastClk == HIGH && clk == LOW) {
    // optional tiny debounce
    if (now - lastStepMs >= ENC_DEBOUNCE_MS) {
      lastStepMs = now;

      // Direction: read DT at the moment CLK falls
      bool dirCW = (digitalRead(ENCODER_DT) == HIGH);

      if (menuState != MENU_NONE) {
        menuOnEncoder(dirCW);
      } else if (digitalRead(ENCODER_SW) == HIGH && isStompboxMode) {
        // adjust BPM only when not in menu and not pressing encoder
        currentBPM += dirCW ? 1 : -1;
        if (currentBPM < 40) currentBPM = 40;
        if (currentBPM > 300) currentBPM = 300;
      }
    }
  }

  lastClk = clk;

  // Encoder push
  if (digitalRead(ENCODER_SW) == LOW) {
    delay(150); // debounce
    if (menuState == MENU_NONE) {
      menuState = MENU_MAIN;
      menuEnter();
    } else {
      menuOnSelect();
    }
  }
}
