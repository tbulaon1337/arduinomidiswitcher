#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MIDI.h>

// ================= Project sizes =================
#define NUM_RELAYS        5
#define PRESETS_PER_BANK  5
#define NUM_BANKS         35
#define BYTES_PER_PRESET  11   // UPDATED: now 11 bytes per preset (Mega EEPROM is fine)

// Name Saves
#define PRESET_NAME_LEN 12     // 12 chars max (OLED-friendly)

// ================= OLED =================
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_ADDR      0x3C

// ================= Pin map =================

// ---------- Relays ----------
#define RELAY_PINS        {22, 23, 24, 25, 26}

// ---------- Footswitches ----------
#define FOOTSWITCH_PINS   {30, 31, 32, 33, 34}

// ---------- Rotary encoder ----------
#define ENCODER_CLK       A0
#define ENCODER_DT        A1
#define ENCODER_SW        A2

// ---------- Panel tactiles ----------
#define BTN_BACK          37
#define BTN_SAVE          38

// ---------- Dedicated switches ----------
#define TAP_TEMPO_PIN     35
#define MODE_BTN_PIN      36

// ---------- Loop/Preset LEDs ----------
#define LOOP_LED_PINS     {41, 42, 43, 44, 45}

// ---------- Status LEDs ----------
#define TAP_LED_PIN       46
#define MODE_LED_PIN      47

// ---------- Expression pedal ----------
#define EXP_PEDAL_PIN     A5

// ================= Timing constants =================
#define DEBOUNCE_MS       35
#define COMBO_LATCH_MS    150

// ================= Preset structure =================
// EXACTLY 11 bytes now
struct Preset {
  uint8_t loopBits;     // bit0..4 = loops 1..5
  uint8_t relay5Mode;   // 0 = loop, 1 = amp channel
  uint8_t midiPC;       // 0–127
  uint8_t midiCC;       // 0–127
  uint8_t ccValue;      // 0–127

  // --- NEW: per-preset expression settings ---
  uint8_t expEnable;    // 0/1
  uint8_t expCC;        // 0–127 (default suggestion: 11)
  uint8_t expInvert;    // 0/1

  // spare
  uint8_t rsv1;
  uint8_t rsv2;
  uint8_t rsv3;
};

static_assert(sizeof(Preset) == BYTES_PER_PRESET, "Preset must be 11 bytes");

// ================= Global state (defined in MIDI_Switcher.ino) =================
extern bool        loopStates[NUM_RELAYS];
extern bool        relay5_isAmpSwitch;

extern bool        isStompboxMode;
extern int         currentBank;
extern int         selectedSlot;

extern int         altBank;
extern bool        startupIsStomp;
extern int         startupBank;
extern uint16_t    defaultBPM;

extern byte        midiChannel;
extern bool        midiClockEnabled;
extern uint16_t    currentBPM;

// ---- NEW: active preset snapshot (for stomp overlay + expression logic) ----
extern Preset      activePreset;
extern bool        haveActivePreset;

// ================= Menu / UI =================
enum MenuState { MENU_NONE, MENU_MAIN };
extern MenuState menuState;

// ================= Module interfaces =================

// Display
void setupDisplay();
void drawMainUI();
void drawMenuUI();
void displayShowMessage(const char* msg, uint16_t ms);
void displayShowRecall(int bank, int slot);

// Inputs
void setupInputs();
void pollInputs();
void checkMenuButtons();

// Encoder
void setupEncoder();
void checkEncoder();

// Relays
void setupRelays();
void setLoop(int loopNum, bool state);
void toggleLoop(int loopNum);
byte encodeLoopStates(bool loops[NUM_RELAYS]);
void decodeLoopStates(byte encoded, bool loops[NUM_RELAYS]);

// Presets
int  getPresetIndex(int bank, int slot);
int  getPresetAddress(int bank, int slot);
void savePreset(int bank, int slot, const Preset &p);
Preset loadPreset(int bank, int slot);
void applyPresetToHardware(const Preset &p);

// MIDI
void setupMIDI();
void sendPC(byte pc);
void sendCC(byte cc, byte value);
void midiClockStart();
void midiClockStop();
void updateMidiClock();

// Tap tempo
void checkTapTempo();
void taptempoTap();

// Config store
void saveGlobalConfig();
void loadGlobalConfig();
