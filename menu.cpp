// ===================== menu.cpp =====================
#include "menu.h"
#include "config.h"
#include "display.h"
#include "presets.h"
#include "relay_logic.h"
#include "midi.h"
#include "taptempo.h"
#include "config_store.h"
#include "leds.h"
#include <Adafruit_SSD1306.h>
#include <string.h>

// ---- OLED from display.cpp ----
extern Adafruit_SSD1306 display;

// ---- Menu state comes from the .ino (do NOT define here) ----
extern MenuState menuState;

// ---- active preset snapshot (defined in MIDI_Switcher.ino) ----
extern Preset activePreset;
extern bool   haveActivePreset;

// ----------------- Footswitch scanning -----------------
static const int fsPins[NUM_RELAYS] = FOOTSWITCH_PINS;

static unsigned long lastFsTime[NUM_RELAYS]  = {0,0,0,0,0};
static bool          lastFsState[NUM_RELAYS] = {0,0,0,0,0}; // store "pressed" bool

static const unsigned long DEBOUNCE_LOCAL_MS = DEBOUNCE_MS;

// Combo & other inputs
static bool     comboLatch = false;
static uint32_t comboAtMs  = 0;

static int      lastModeBtn = HIGH;
static uint32_t lastModeMs  = 0;

static int      lastTap     = HIGH;
static uint32_t lastTapMs   = 0;

// ----------------- Simple menu model -----------------
enum MenuPage {
  PAGE_MAIN = 0,
  PAGE_SET_START_MODE,
  PAGE_SET_START_BANK,
  PAGE_SET_DEFAULT_BPM,
  PAGE_PRESET_SETUP,
  PAGE_MIDI_SETTINGS,
  PAGE_CLEAR_PRESET,
  PAGE_COPY_PRESET
};

static const char* MAIN_ITEMS[] = {
  "Startup Mode",
  "Startup Bank",
  "BPM Default",
  "Preset Setup",
  "MIDI Settings",
  "Clear Preset",
  "Copy Preset"
};
static const int MAIN_COUNT = sizeof(MAIN_ITEMS)/sizeof(MAIN_ITEMS[0]);

// menu runtime
static bool     inMenu    = false;
static bool     inEdit    = false;
static MenuPage page      = PAGE_MAIN;
static int      menuIndex = 0;

// ---- MAIN MENU SCROLLING ----
static int      menuScroll   = 0;
static const int MENU_VISIBLE = 5;
static const int MENU_Y_START = 12;
static const int MENU_ROW_H   = 10;

// preset edit
static Preset   editPreset;
static int      presetField = 0;   // 0..8 now
static int      loopCursor  = 0;

// ---- PRESET SETUP SCROLLING ----
static int presetScroll = 0;
static const int PRESET_FIELD_COUNT = 9; // Loops, R5, PC, CC, Val, Name, EXP En, EXP CC, EXP Inv
static const int PRESET_VISIBLE     = 5; // show 5 rows, scroll for the rest

// ---- preset name editor state ----
static char editName[PRESET_NAME_LEN + 1] = {0};
static int  nameCursor = 0;
static const char NAME_CHARS[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";
static const int  NAME_CHARS_LEN = sizeof(NAME_CHARS) - 1;

static int findCharIndex(char c) {
  for (int i = 0; i < NAME_CHARS_LEN; ++i) if (NAME_CHARS[i] == c) return i;
  return 0; // space
}

// MIDI settings edit
static int midiField = 0;

// Copy preset
static int copySrcBank = 0;
static int copySrcSlot = 0;

// ----------------- Helpers -----------------
static void recallPresetAndToast(int bank, int slot) {
  selectedSlot = slot;

  Preset p = loadPreset(bank, slot);

  // cache active preset so stomp overlay can restore it
  activePreset = p;
  haveActivePreset = true;

  applyPresetToHardware(p);
  sendPC(p.midiPC);
  sendCC(p.midiCC, p.ccValue);

  displayShowRecall(bank, slot);
}

static void recallPresetSilently(int bank, int slot) {
  selectedSlot = slot;

  Preset p = loadPreset(bank, slot);

  activePreset = p;
  haveActivePreset = true;

  applyPresetToHardware(p);
  sendPC(p.midiPC);
  sendCC(p.midiCC, p.ccValue);
}

static void handleFootswitchPress(int idx) {
  if (isStompboxMode) {
    toggleLoop(idx);
  } else {
    recallPresetAndToast(currentBank, idx);
  }
  refreshLeds();
}

// ----------------- Public: Inputs -----------------
void setupInputs() {
  for (int i = 0; i < NUM_RELAYS; ++i) pinMode(fsPins[i], INPUT_PULLUP);

  pinMode(ENCODER_CLK,   INPUT_PULLUP);
  pinMode(ENCODER_DT,    INPUT_PULLUP);
  pinMode(ENCODER_SW,    INPUT_PULLUP);
  pinMode(BTN_BACK,      INPUT_PULLUP);
  pinMode(BTN_SAVE,      INPUT_PULLUP);
  pinMode(TAP_TEMPO_PIN, INPUT_PULLUP);
  pinMode(MODE_BTN_PIN,  INPUT_PULLUP);
}

void pollInputs() {
  const uint32_t now = millis();

  // MODE toggle (Preset = main, Stompbox = temporary overlay)
  int m = digitalRead(MODE_BTN_PIN);
  if (m != lastModeBtn && (now - lastModeMs) > DEBOUNCE_LOCAL_MS) {
    lastModeMs = now;
    if (m == LOW) {

      // enter stomp overlay
      if (!isStompboxMode) {
        if (!haveActivePreset) {
          activePreset = loadPreset(currentBank, selectedSlot);
          haveActivePreset = true;
        }
        isStompboxMode = true;
        displayShowMessage("STOMPBOX", 600);
        refreshLeds();
      }
      // exit stomp overlay -> restore preset
      else {
        isStompboxMode = false;

        if (haveActivePreset) {
          applyPresetToHardware(activePreset);
          sendPC(activePreset.midiPC);
          sendCC(activePreset.midiCC, activePreset.ccValue);
        }

        displayShowMessage("PRESET", 600);
        refreshLeds();
      }
    }
    lastModeBtn = m;
  }

  // Tap tempo
  int t = digitalRead(TAP_TEMPO_PIN);
  if (t != lastTap && (now - lastTapMs) > DEBOUNCE_LOCAL_MS) {
    lastTapMs = now;
    if (t == LOW) taptempoTap();
    lastTap = t;
  }

  // Read footswitches (pressed = true)
  bool fsNow[NUM_RELAYS];
  for (int i = 0; i < NUM_RELAYS; ++i)
    fsNow[i] = (digitalRead(fsPins[i]) == LOW);

  // release clears combo latch
  bool anyPressed = false;
  for (int i = 0; i < NUM_RELAYS; ++i) anyPressed |= fsNow[i];
  if (!anyPressed) comboLatch = false;

  // ---------- Bank combos ONLY in preset mode ----------
  if (!comboLatch && !isStompboxMode) {

    // Bank -
    if (fsNow[0] && fsNow[1]) {
      currentBank = (currentBank - 1 + NUM_BANKS) % NUM_BANKS;

      // change sound silently (no Recall toast)
      recallPresetSilently(currentBank, selectedSlot);

      displayShowMessage("Bank -", 600);
      refreshLeds();
      comboLatch = true;
      comboAtMs  = now;
    }

    // Bank +
    else if (fsNow[3] && fsNow[4]) {
      currentBank = (currentBank + 1) % NUM_BANKS;

      recallPresetSilently(currentBank, selectedSlot);

      displayShowMessage("Bank +", 600);
      refreshLeds();
      comboLatch = true;
      comboAtMs  = now;
    }
  }

  // ---------- Individual FS presses ----------
  for (int i = 0; i < NUM_RELAYS; ++i) {

    // debounce edge detect
    if (fsNow[i] != lastFsState[i] &&
        (now - lastFsTime[i]) > DEBOUNCE_LOCAL_MS) {

      // ignore any FS triggers right after a combo
      if (!(comboLatch && (now - comboAtMs) < COMBO_LATCH_MS)) {
        if (fsNow[i]) handleFootswitchPress(i);
      }

      lastFsTime[i]  = now;
      lastFsState[i] = fsNow[i];
    }
  }
}

// ----------------- Menu hooks -----------------
void menuEnter() {
  inMenu = true;
  inEdit = false;
  page = PAGE_MAIN;
  menuIndex = 0;
  menuScroll = 0;
  presetField = 0;
  presetScroll = 0;
  loopCursor = 0;
  nameCursor = 0;
  menuState = MENU_MAIN;
}

void menuExit() {
  inMenu = false;
  inEdit = false;
  menuState = MENU_NONE;
}

void menuOnEncoder(bool dirCW) {
  if (!inMenu) return;
  int step = dirCW ? 1 : -1;

  // ---------- MAIN MENU ----------
  if (page == PAGE_MAIN && !inEdit) {
    menuIndex = (menuIndex + step + MAIN_COUNT) % MAIN_COUNT;

    if (menuIndex < menuScroll) menuScroll = menuIndex;
    else if (menuIndex >= menuScroll + MENU_VISIBLE)
      menuScroll = menuIndex - MENU_VISIBLE + 1;

    int maxScroll = max(0, MAIN_COUNT - MENU_VISIBLE);
    menuScroll = constrain(menuScroll, 0, maxScroll);
    return;
  }

  switch (page) {

    case PAGE_SET_START_MODE:
      startupIsStomp = !startupIsStomp;
      break;

    case PAGE_SET_START_BANK:
      startupBank = (startupBank + step + NUM_BANKS) % NUM_BANKS;
      break;

    case PAGE_SET_DEFAULT_BPM:
      currentBPM = constrain((int)currentBPM + step, 40, 300);
      break;

    case PAGE_MIDI_SETTINGS:
      if (!inEdit) {
        midiField ^= 1;
      } else if (midiField == 0) {
        midiChannel = constrain((int)midiChannel + step, 1, 16);
      } else {
        midiClockEnabled = !midiClockEnabled;
      }
      break;

    // ---------- PRESET SETUP ----------
    case PAGE_PRESET_SETUP:
      if (!inEdit) {
        // Move between fields (0..PRESET_FIELD_COUNT-1)
        presetField = (presetField + step + PRESET_FIELD_COUNT) % PRESET_FIELD_COUNT;

        // Keep selected field visible
        if (presetField < presetScroll) presetScroll = presetField;
        else if (presetField >= presetScroll + PRESET_VISIBLE)
          presetScroll = presetField - PRESET_VISIBLE + 1;

        int maxScroll = max(0, PRESET_FIELD_COUNT - PRESET_VISIBLE);
        presetScroll = constrain(presetScroll, 0, maxScroll);

      } else {
        // Edit current field
        if (presetField == 0) {
          loopCursor = (loopCursor + step + NUM_RELAYS) % NUM_RELAYS;
        }
        else if (presetField == 1) {
          editPreset.relay5Mode ^= 1;
        }
        else if (presetField == 2) {
          editPreset.midiPC = constrain((int)editPreset.midiPC + step, 0, 127);
        }
        else if (presetField == 3) {
          editPreset.midiCC = constrain((int)editPreset.midiCC + step, 0, 127);
        }
        else if (presetField == 4) {
          editPreset.ccValue = constrain((int)editPreset.ccValue + step, 0, 127);
        }
        else if (presetField == 5) {
          char &c = editName[nameCursor];
          int idx = findCharIndex(c);
          idx = (idx + step + NAME_CHARS_LEN) % NAME_CHARS_LEN;
          c = NAME_CHARS[idx];
        }
        else if (presetField == 6) {
          // EXP Enable
          editPreset.expEnable ^= 1;
        }
        else if (presetField == 7) {
          // EXP CC
          int v = (int)editPreset.expCC;
          if (v > 127) v = 11;
          v = constrain(v + step, 0, 127);
          editPreset.expCC = (uint8_t)v;
        }
        else if (presetField == 8) {
          // EXP Invert
          editPreset.expInvert ^= 1;
        }
      }
      break;

    case PAGE_COPY_PRESET:
      if (!inEdit) {
        copySrcBank = (copySrcBank + step + NUM_BANKS) % NUM_BANKS;
      } else {
        copySrcSlot = constrain(copySrcSlot + step, 0, PRESETS_PER_BANK - 1);
      }
      break;

    default:
      break;
  }
}

void menuOnSelect() {
  if (!inMenu) { menuEnter(); return; }

  if (page == PAGE_MAIN) {
    page = (MenuPage)(PAGE_SET_START_MODE + menuIndex);
    inEdit = false;

    if (page == PAGE_PRESET_SETUP) {
      editPreset  = loadPreset(currentBank, selectedSlot);
      loadPresetName(currentBank, selectedSlot, editName, sizeof(editName));

      // sensible defaults if unset
      if (editPreset.expCC > 127) editPreset.expCC = 11;

      presetField  = 0;
      presetScroll = 0;
      loopCursor   = 0;
      nameCursor   = 0;
    } else if (page == PAGE_COPY_PRESET) {
      copySrcBank = currentBank;
      copySrcSlot = selectedSlot;
    }
    return;
  }

  if (page == PAGE_PRESET_SETUP && inEdit) {
    if (presetField == 0) {
      editPreset.loopBits ^= (1 << loopCursor);
    } else if (presetField == 5) {
      nameCursor++;
      if (nameCursor >= PRESET_NAME_LEN) nameCursor = 0;
    }
    return;
  }

  inEdit = !inEdit;
}

// ----------------- Back / Save -----------------
void checkMenuButtons() {
  if (digitalRead(BTN_BACK) == LOW) {
    delay(150);
    if (!inMenu) return;
    if (inEdit) inEdit = false;
    else if (page != PAGE_MAIN) page = PAGE_MAIN;
    else menuExit();
  }

  if (digitalRead(BTN_SAVE) == LOW) {
    delay(150);
    if (!inMenu) return;

    switch (page) {
      case PAGE_SET_START_MODE:
      case PAGE_SET_START_BANK:
      case PAGE_SET_DEFAULT_BPM:
      case PAGE_MIDI_SETTINGS:
        saveGlobalConfig();
        displayShowMessage("Saved", 700);
        break;

      case PAGE_PRESET_SETUP:
        savePreset(currentBank, selectedSlot, editPreset);
        savePresetName(currentBank, selectedSlot, editName);

        // keep snapshot in sync (this affects EXP behaviour too)
        activePreset = editPreset;
        haveActivePreset = true;

        applyPresetToHardware(editPreset);
        displayShowMessage("Preset Saved", 900);
        break;

      case PAGE_CLEAR_PRESET:
        savePreset(currentBank, selectedSlot, Preset{});
        savePresetName(currentBank, selectedSlot, "");
        displayShowMessage("Cleared", 700);
        break;

      case PAGE_COPY_PRESET: {
        Preset src = loadPreset(copySrcBank, copySrcSlot);
        savePreset(currentBank, selectedSlot, src);

        char tmp[PRESET_NAME_LEN + 1];
        loadPresetName(copySrcBank, copySrcSlot, tmp, sizeof(tmp));
        savePresetName(currentBank, selectedSlot, tmp);

        applyPresetToHardware(src);
        displayShowMessage("Copied", 700);
      } break;

      default: break;
    }

    inEdit = false;
    page = PAGE_MAIN;
  }
}

// ----------------- Drawing -----------------
static void drawHeader(const char* title) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(title);
  display.drawLine(0, 10, SCREEN_WIDTH - 1, 10, SSD1306_WHITE);
}

void drawMenuUI() {
  if (!inMenu) return;

  switch (page) {
    case PAGE_MAIN: {
      drawHeader("Settings");

      if (menuScroll > 0) {
        display.setCursor(120, MENU_Y_START);
        display.print("^");
      }
      if (menuScroll + MENU_VISIBLE < MAIN_COUNT) {
        display.setCursor(120, MENU_Y_START + (MENU_VISIBLE - 1) * MENU_ROW_H);
        display.print("v");
      }

      for (int row = 0; row < MENU_VISIBLE; ++row) {
        int item = menuScroll + row;
        if (item >= MAIN_COUNT) break;

        display.setCursor(0, MENU_Y_START + row * MENU_ROW_H);
        if (item == menuIndex) display.print("> ");
        else                   display.print("  ");
        display.print(MAIN_ITEMS[item]);
      }

      display.display();
    } break;

    case PAGE_SET_START_MODE: {
      drawHeader("Startup Mode");
      display.setCursor(0, 14);
      display.print(startupIsStomp ? "Stompbox" : "Preset");
      display.setCursor(0, 26);
      display.print("Back=Cancel  Save=OK");
      display.display();
    } break;

    case PAGE_SET_START_BANK: {
      drawHeader("Startup Bank");
      display.setCursor(0, 14);
      display.print("Bank: "); display.print(startupBank + 1);
      display.setCursor(0, 26);
      display.print("Back=Cancel  Save=OK");
      display.display();
    } break;

    case PAGE_SET_DEFAULT_BPM: {
      drawHeader("Default BPM");
      display.setCursor(0, 14);
      display.print("BPM: "); display.print((int)currentBPM);
      display.setCursor(0, 26);
      display.print("Back=Cancel  Save=OK");
      display.display();
    } break;

    case PAGE_MIDI_SETTINGS: {
      drawHeader("MIDI Settings");

      display.setCursor(0, 14);
      if (!inEdit && midiField == 0) display.print("> ");
      else                           display.print("  ");
      display.print("Channel: "); display.print(midiChannel);

      display.setCursor(0, 26);
      if (!inEdit && midiField == 1) display.print("> ");
      else                           display.print("  ");
      display.print("Clock: "); display.print(midiClockEnabled ? "On" : "Off");

      display.setCursor(0, 38);
      display.print("Back=Cancel  Save=OK");

      display.display();
    } break;

    case PAGE_CLEAR_PRESET: {
      drawHeader("Clear Preset");
      display.setCursor(0, 14);
      display.print("B"); display.print(currentBank + 1);
      display.print(" S"); display.print(selectedSlot + 1);
      display.setCursor(0, 26);
      display.print("Back=Cancel  Save=OK");
      display.display();
    } break;

    case PAGE_COPY_PRESET: {
      drawHeader("Copy Preset");
      display.setCursor(0, 14);
      display.print("From:");

      display.setCursor(0, 26);
      display.print("Bank "); display.print(copySrcBank + 1);

      display.setCursor(64, 26);
      display.print("Slot "); display.print(copySrcSlot + 1);

      display.setCursor(0, 38);
      display.print(inEdit ? "Editing: Slot" : "Editing: Bank");

      display.setCursor(0, 50);
      display.print("Back=Cancel  Save=OK");

      display.display();
    } break;

    case PAGE_PRESET_SETUP: {
      drawHeader("Preset Setup");

      display.setCursor(0, 12);
      display.print("B"); display.print(currentBank + 1);
      display.print(" S"); display.print(selectedSlot + 1);

      const int yStart = 18;
      const int rowH   = 8;

      // scroll indicators
      if (presetScroll > 0) {
        display.setCursor(120, yStart);
        display.print("^");
      }
      if (presetScroll + PRESET_VISIBLE < PRESET_FIELD_COUNT) {
        display.setCursor(120, yStart + (PRESET_VISIBLE - 1) * rowH);
        display.print("v");
      }

      for (int row = 0; row < PRESET_VISIBLE; ++row) {
        int i = presetScroll + row;
        if (i >= PRESET_FIELD_COUNT) break;

        const int rowY = yStart + row * rowH;
        display.setCursor(0, rowY);

        if (i == presetField) {
          if (!inEdit) display.print("> ");
          else         display.print("* ");
        } else {
          display.print("  ");
        }

        switch (i) {
          case 0: // Loops
            display.print("Loops:");
            display.setCursor(64, rowY);
            for (int b = 0; b < NUM_RELAYS; ++b) {
              bool on = (editPreset.loopBits >> b) & 0x01;
              if (inEdit && presetField == 0 && b == loopCursor) display.print('(');
              display.print(on ? 'X' : '-');
              if (inEdit && presetField == 0 && b == loopCursor) display.print(')');
            }
            break;

          case 1: // Relay5
            display.print("R5:");
            display.setCursor(64, rowY);
            display.print(editPreset.relay5Mode ? "AMP" : "LOOP");
            break;

          case 2: // PC
            display.print("PC:");
            display.setCursor(64, rowY);
            display.print(editPreset.midiPC);
            break;

          case 3: // CC
            display.print("CC:");
            display.setCursor(64, rowY);
            display.print(editPreset.midiCC);
            break;

          case 4: // CC Val
            display.print("Val:");
            display.setCursor(64, rowY);
            display.print(editPreset.ccValue);
            break;

          case 5: // Name
            display.print("Name:");
            display.setCursor(64, rowY);
            for (int k = 0; k < 10 && k < PRESET_NAME_LEN; ++k) {
              char c = editName[k] ? editName[k] : ' ';
              if (inEdit && presetField == 5 && k == nameCursor) display.print('(');
              display.print(c);
              if (inEdit && presetField == 5 && k == nameCursor) display.print(')');
            }
            break;

          case 6: // EXP Enable
            display.print("EXP:");
            display.setCursor(64, rowY);
            display.print(editPreset.expEnable ? "On" : "Off");
            break;

          case 7: // EXP CC
            display.print("EXPCC:");
            display.setCursor(64, rowY);
            display.print((int)editPreset.expCC);
            break;

          case 8: // EXP Invert
            display.print("EXPInv:");
            display.setCursor(64, rowY);
            display.print(editPreset.expInvert ? "On" : "Off");
            break;
        }
      }

      display.setCursor(0, 56);
      display.print("Back=Cancel Save=OK");
      display.display();
    } break;

    default:
      break;
  }
}
