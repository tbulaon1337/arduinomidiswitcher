// ===================== presets.cpp =====================
#include "config.h"
#include "presets.h"
#include "relay_logic.h"
#include <EEPROM.h>
#include <string.h>

// Compile-time safety check
static_assert(sizeof(Preset) == BYTES_PER_PRESET, "BYTES_PER_PRESET does not match sizeof(Preset)!");

// ------- extern globals (defined in MIDI_Switcher.ino) -------
extern bool loopStates[NUM_RELAYS];
extern bool relay5_isAmpSwitch;

// ------- EEPROM layout for presets -------
// Leave room at the start for the global config blob + CRC.
static const int PRESET_BASE_ADDR = 64;

// ------- EEPROM layout for preset names -------
// Names live AFTER the preset blob region, so we don't change preset addresses.
static const int PRESET_COUNT      = (NUM_BANKS * PRESETS_PER_BANK);
static const int PRESET_DATA_BYTES = (PRESET_COUNT * BYTES_PER_PRESET);

// Each name stored as fixed (PRESET_NAME_LEN + 1) bytes (includes NUL)
static const int NAME_STRIDE           = (PRESET_NAME_LEN + 1);
static const int PRESET_NAME_BASE_ADDR = PRESET_BASE_ADDR + PRESET_DATA_BYTES;

// ------- internal helpers -------
static inline Preset defaultPreset() {
  Preset p{};
  p.loopBits   = 0;
  p.relay5Mode = 0;
  p.midiPC     = 0;
  p.midiCC     = 0;
  p.ccValue    = 0;

  // NEW defaults
  p.expEnable  = 0;     // off by default
  p.expCC      = 11;    // common default
  p.expInvert  = 0;

  return p;
}

static inline Preset normalisePreset(const Preset &in) {
  Preset p = in;

  // Only keep the lowest NUM_RELAYS bits
  p.loopBits &= ((1 << NUM_RELAYS) - 1);

  // Relay5 mode 0/1
  p.relay5Mode = (p.relay5Mode ? 1 : 0);

  // Clamp MIDI domains
  if (p.midiPC  > 127) p.midiPC  = 127;
  if (p.midiCC  > 127) p.midiCC  = 127;
  if (p.ccValue > 127) p.ccValue = 127;

  // NEW: exp fields
  p.expEnable = (p.expEnable ? 1 : 0);
  if (p.expCC > 127) p.expCC = 127;
  p.expInvert = (p.expInvert ? 1 : 0);

  // If exp is enabled but CC is 0xFF-ish or nonsense, give a sane default
  if (p.expCC == 0xFF) p.expCC = 11;

  return p;
}

int getPresetIndex(int bank, int slot) {
  if (bank < 0) bank = 0;
  if (bank >= NUM_BANKS) bank = NUM_BANKS - 1;
  if (slot < 0) slot = 0;
  if (slot >= PRESETS_PER_BANK) slot = PRESETS_PER_BANK - 1;
  return bank * PRESETS_PER_BANK + slot;
}

int getPresetAddress(int bank, int slot) {
  const int idx = getPresetIndex(bank, slot);
  return PRESET_BASE_ADDR + idx * BYTES_PER_PRESET;
}

static int getPresetNameAddress(int bank, int slot) {
  const int idx = getPresetIndex(bank, slot);
  return PRESET_NAME_BASE_ADDR + (idx * NAME_STRIDE);
}

// ================= Public API =================
void savePreset(int bank, int slot, const Preset &pIn) {
  const Preset p = normalisePreset(pIn);
  const int addr = getPresetAddress(bank, slot);
  EEPROM.put(addr, p);
}

Preset loadPreset(int bank, int slot) {
  Preset p{};
  const int addr = getPresetAddress(bank, slot);
  EEPROM.get(addr, p);

  // If EEPROM is “blank” (all 0xFF for the struct), return defaults.
  const uint8_t *raw = (const uint8_t*)&p;
  bool looksBlank = true;
  for (int i = 0; i < (int)sizeof(Preset); ++i) {
    if (raw[i] != 0xFF) { looksBlank = false; break; }
  }

  if (looksBlank) return defaultPreset();

  // If only the new EXP bytes are blank (older saved preset), patch in defaults
  if (p.expEnable == 0xFF && p.expCC == 0xFF && p.expInvert == 0xFF) {
    p.expEnable = 0;
    p.expCC     = 11;
    p.expInvert = 0;
  }

  return normalisePreset(p);
}

void applyPresetToHardware(const Preset &pIn) {
  const Preset p = normalisePreset(pIn);

  // Decode loop bits into loopStates and push to relays
  for (int i = 0; i < NUM_RELAYS; ++i) {
    bool on = (p.loopBits >> i) & 0x01;
    setLoop(i, on);
    loopStates[i] = on;
  }

  // Relay 5 mode (amp switch vs loop)
  relay5_isAmpSwitch = (p.relay5Mode != 0);
}

Preset makePresetFromCurrentState(byte pc, byte cc, byte val) {
  Preset p = defaultPreset();

  // Build loop bits from current loopStates
  p.loopBits = 0;
  for (int i = 0; i < NUM_RELAYS; ++i) {
    if (loopStates[i]) p.loopBits |= (1 << i);
  }

  p.relay5Mode = relay5_isAmpSwitch ? 1 : 0;
  p.midiPC     = (pc  > 127) ? 127 : pc;
  p.midiCC     = (cc  > 127) ? 127 : cc;
  p.ccValue    = (val > 127) ? 127 : val;

  // EXP fields left at defaults here (they’re edited/saved via preset setup)

  return normalisePreset(p);
}

void saveCurrentStateToPreset(int bank, int slot, byte pc, byte cc, byte val) {
  Preset p = makePresetFromCurrentState(pc, cc, val);
  savePreset(bank, slot, p);
}

// ================= Preset Name API =================
void savePresetName(int bank, int slot, const char* name) {
  const int lastByteNeeded =
    getPresetNameAddress(NUM_BANKS - 1, PRESETS_PER_BANK - 1) + NAME_STRIDE;
  if (lastByteNeeded > (int)EEPROM.length()) return;

  char buf[PRESET_NAME_LEN + 1];
  memset(buf, 0, sizeof(buf));

  if (name && *name) {
    strncpy(buf, name, PRESET_NAME_LEN);
    buf[PRESET_NAME_LEN] = '\0';
  }

  const int addr = getPresetNameAddress(bank, slot);
  for (int i = 0; i < (int)sizeof(buf); ++i) {
    EEPROM.update(addr + i, (uint8_t)buf[i]);
  }
}

void loadPresetName(int bank, int slot, char* outName, int outLen) {
  if (!outName || outLen <= 0) return;

  outName[0] = '\0';

  const int lastByteNeeded =
    getPresetNameAddress(NUM_BANKS - 1, PRESETS_PER_BANK - 1) + NAME_STRIDE;
  if (lastByteNeeded > (int)EEPROM.length()) return;

  char buf[PRESET_NAME_LEN + 1];
  const int addr = getPresetNameAddress(bank, slot);

  for (int i = 0; i < (int)sizeof(buf); ++i) {
    buf[i] = (char)EEPROM.read(addr + i);
  }
  buf[PRESET_NAME_LEN] = '\0';

  // detect blank EEPROM (0xFF)
  bool blank = true;
  for (int i = 0; i < PRESET_NAME_LEN; ++i) {
    if ((uint8_t)buf[i] != 0xFF) { blank = false; break; }
  }
  if (blank) return;

  if (buf[0] == '\0') return;

  strncpy(outName, buf, outLen - 1);
  outName[outLen - 1] = '\0';
}
