// ===================== display.cpp =====================
#include "config.h"
#include "presets.h"
#include <Adafruit_SSD1306.h>

// ===== OLED instance (defined here, extern elsewhere) =====
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Extern globals from MIDI_Switcher.ino =====
extern bool     isStompboxMode;
extern int      currentBank;     // 0-based
extern int      selectedSlot;    // 0-based
extern bool     loopStates[NUM_RELAYS];

extern bool     midiClockEnabled;
extern uint16_t currentBPM;

// ---- active preset snapshot ----
extern Preset   activePreset;
extern bool     haveActivePreset;

// ===== Simple toast/message buffer =====
static uint32_t msgSinceMs = 0;
static uint16_t msgDurMs   = 0;
static char     msgText[18] = {0};

// Recall toast (mutually exclusive with msgText)
static int      toastBank = -1;
static int      toastSlot = -1;

// ===== Preset name cache =====
static int  lastNameBank = -1;
static int  lastNameSlot = -1;
static char cachedName[PRESET_NAME_LEN + 1] = {0};

static void refreshNameCacheIfNeeded() {
  if (isStompboxMode) return;
  if (currentBank == lastNameBank && selectedSlot == lastNameSlot) return;

  loadPresetName(currentBank, selectedSlot, cachedName, sizeof(cachedName));
  lastNameBank = currentBank;
  lastNameSlot = selectedSlot;
}

// ===== Local helpers =====
static void drawHeader(const char* left, const char* right) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(left);

  if (right && *right) {
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(right, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(SCREEN_WIDTH - (int)w - 1, 0);
    display.print(right);
  }

  display.drawLine(0, 10, SCREEN_WIDTH - 1, 10, SSD1306_WHITE);
}

static void drawLoopsRow(int y, const bool loops[NUM_RELAYS]) {
  display.setCursor(0, y);
  display.print("Loops: ");
  for (int i = 0; i < NUM_RELAYS; ++i) {
    display.print(loops[i] ? 'X' : '-');
    if (i < NUM_RELAYS - 1) display.print(' ');
  }
}

static void drawBpmRow(int y) {
  display.setCursor(0, y);
  display.print("Clock: ");
  if (midiClockEnabled) {
    display.print("On BPM ");
    display.print((int)currentBPM);
  } else {
    display.print("Off");
  }
}

static void drawExpRow(int y) {
  if (!haveActivePreset) return;

  display.setCursor(0, y);

  if (!activePreset.expEnable) {
    display.print("EXP: Off");
    return;
  }

  display.print("EXP: On  CC:");
  display.print((int)activePreset.expCC);

  display.print(" Inv:");
  display.print(activePreset.expInvert ? "On" : "-");
}

static void drawToastIfAny() {
  const uint32_t now = millis();

  // Message toast
  if (msgText[0] != '\0' && msgDurMs && (now - msgSinceMs) < msgDurMs) {
    const char* s = msgText;
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
    int x = (SCREEN_WIDTH  - (int)w) / 2 - 4;
    int y = (SCREEN_HEIGHT - 10) / 2 - 3;
    if (x < 0) x = 0;
    if (y < 12) y = 12;

    display.fillRect(x, y, w + 8, 14, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(x + 4, y + 3);
    display.print(s);
    display.setTextColor(SSD1306_WHITE);
    return;
  }

  // Recall toast
  if (toastBank >= 0 && toastSlot >= 0 && msgDurMs && (now - msgSinceMs) < msgDurMs) {
    char buf[20];
    snprintf(buf, sizeof(buf), "Recall B%d S%d", toastBank + 1, toastSlot + 1);

    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    int x = (SCREEN_WIDTH  - (int)w) / 2 - 4;
    int y = (SCREEN_HEIGHT - 10) / 2 - 3;
    if (x < 0) x = 0;
    if (y < 12) y = 12;

    display.fillRect(x, y, w + 8, 14, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(x + 4, y + 3);
    display.print(buf);
    display.setTextColor(SSD1306_WHITE);
  }
}

// ===== Public API =====
void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("MIDI Switcher");
  display.setCursor(0, 12);
  display.print("Booting...");
  display.display();
  delay(400);
}

void drawMainUI() {
  char left[16];
  snprintf(left, sizeof(left), "%s", isStompboxMode ? "STOMPBOX" : "PRESET");

  char right[16] = {0};
  if (!isStompboxMode) {
    snprintf(right, sizeof(right), "B%d S%d", currentBank + 1, selectedSlot + 1);
  }

  drawHeader(left, right);

  // Row 1: Loops
  drawLoopsRow(16, loopStates);

  // Row 2: Clock/BPM
  drawBpmRow(28);

  // Row 3: Preset Name
  if (!isStompboxMode) {
    refreshNameCacheIfNeeded();
    display.setCursor(0, 40);
    display.print("Name: ");
    display.print(cachedName);

    // Row 4: EXP status
    drawExpRow(52);
  }

  drawToastIfAny();
  display.display();
}

// ===== Toast helpers =====
void displayShowMessage(const char* msg, uint16_t ms) {
  if (!msg) return;
  strncpy(msgText, msg, sizeof(msgText) - 1);
  msgText[sizeof(msgText) - 1] = '\0';

  toastBank = -1;
  toastSlot = -1;
  msgSinceMs = millis();
  msgDurMs   = ms ? ms : 1000;
}

void displayShowRecall(int bank, int slot) {
  toastBank = bank;
  toastSlot = slot;
  msgText[0] = '\0';
  msgSinceMs = millis();
  msgDurMs   = 1200;
}
