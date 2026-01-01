// ===================== config_store.cpp =====================
#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"

// -------- extern globals defined in MIDI_Switcher.ino --------
extern bool     startupIsStomp;
extern int      startupBank;
extern int      altBank;
extern byte     midiChannel;
extern bool     midiClockEnabled;
extern uint16_t defaultBPM;

// ================= EEPROM layout for Global Config =================
// We place the config blob at address 0. Presets are handled separately
// by presets.cpp and should NOT overlap.

static const uint16_t CFG_ADDR    = 0;       // start of EEPROM for config
static const uint8_t  CFG_VERSION = 0x03;    // bump when fields change

// Packed config payload (no pointers, POD only)
struct GlobalConfigBlob {
  // header
  uint32_t magic;       // 'MSW0' marker
  uint8_t  version;     // CFG_VERSION
  uint8_t  _pad0;       // alignment
  uint16_t len;         // bytes from 'payloadStart' to end-of-payload (excl. crc)

  // ---- payloadStart (offset recorded in len) ----
  // BASIC SETTINGS
  uint8_t  startupIsStomp_u8;  // 0/1
  uint8_t  startupBank_u8;     // 0..(NUM_BANKS-1)
  uint8_t  altBank_u8;         // 0..(NUM_BANKS-1)
  uint8_t  midiChannel_u8;     // 1..16
  uint8_t  midiClockEnabled_u8;// 0/1
  uint8_t  _pad1;              // keep payload aligned
  uint16_t defaultBPM_u16;     // e.g., 120

  // ---- end of payload; CRC follows in EEPROM ----
  // We store CRC32 immediately after the payload when writing/reading.
};

// CRC32 (polynomial 0xEDB88320, same as ZIP)
static uint32_t crc32_update(uint32_t crc, const uint8_t* data, size_t len) {
  crc = ~crc;
  while (len--) {
    crc ^= *data++;
    for (uint8_t k = 0; k < 8; k++) {
      crc = (crc >> 1) ^ (0xEDB88320UL & (-(int32_t)(crc & 1)));
    }
  }
  return ~crc;
}

static void applyDefaults() {
  startupIsStomp   = false;
  startupBank      = 0;
  altBank          = 0;
  midiChannel      = 1;
  midiClockEnabled = false;
  defaultBPM       = 120;
}

static bool blobIsSane(const GlobalConfigBlob& b) {
  if (b.magic   != 0x3057534DUL) return false; // 'M''S''W''0' in LE: 0x3057534D
  if (b.version != CFG_VERSION) return false;
  if (b.len     != sizeof(GlobalConfigBlob) - offsetof(GlobalConfigBlob, startupIsStomp_u8)) return false;

  // field sanity
  if (b.startupBank_u8 >= NUM_BANKS) return false;
  if (b.altBank_u8     >= NUM_BANKS) return false;
  if (b.midiChannel_u8 < 1 || b.midiChannel_u8 > 16) return false;
  if (b.defaultBPM_u16 < 30 || b.defaultBPM_u16 > 300) return false;

  return true;
}

static void writeBlobAndCRC(uint16_t addr, const GlobalConfigBlob& b) {
  // payload size
  const uint16_t payloadOffset = offsetof(GlobalConfigBlob, startupIsStomp_u8);
  const uint16_t payloadLen    = sizeof(GlobalConfigBlob) - payloadOffset;

  // Write the blob
  EEPROM.put(addr, b);

  // Compute CRC over payload region only
  uint8_t tmp[payloadLen];
  for (uint16_t i = 0; i < payloadLen; ++i) {
    tmp[i] = EEPROM.read(addr + payloadOffset + i);
  }
  const uint32_t crc = crc32_update(0, tmp, payloadLen);

  // Store CRC right after the blob payload (not inside struct)
  const uint16_t crcAddr = addr + payloadOffset + payloadLen;
  EEPROM.put(crcAddr, crc);
}

static bool readBlobAndCheckCRC(uint16_t addr, GlobalConfigBlob& out) {
  // Read header first to know lengths
  EEPROM.get(addr, out);

  // Quick header sanity before CRC check
  if (out.magic != 0x3057534DUL) return false;
  if (out.version != CFG_VERSION) return false;

  const uint16_t payloadOffset = offsetof(GlobalConfigBlob, startupIsStomp_u8);
  if (out.len != (sizeof(GlobalConfigBlob) - payloadOffset)) return false;

  // Compute CRC over payload in EEPROM
  const uint16_t payloadLen = out.len;
  uint8_t tmp[payloadLen];
  for (uint16_t i = 0; i < payloadLen; ++i) {
    tmp[i] = EEPROM.read(addr + payloadOffset + i);
  }
  const uint32_t crcCalc = crc32_update(0, tmp, payloadLen);

  // Read stored CRC
  uint32_t crcStored = 0;
  const uint16_t crcAddr = addr + payloadOffset + payloadLen;
  EEPROM.get(crcAddr, crcStored);

  return (crcCalc == crcStored) && blobIsSane(out);
}

// ================= Public API =================
void saveGlobalConfig() {
  GlobalConfigBlob b{};
  b.magic   = 0x3057534DUL; // 'MSW0'
  b.version = CFG_VERSION;
  b.len     = sizeof(GlobalConfigBlob) - offsetof(GlobalConfigBlob, startupIsStomp_u8);

  // Clamp/coerce before writing
  uint8_t sb = (startupBank  < 0) ? 0 : (startupBank >= NUM_BANKS ? (NUM_BANKS - 1) : startupBank);
  uint8_t ab = (altBank      < 0) ? 0 : (altBank     >= NUM_BANKS ? (NUM_BANKS - 1) : altBank);
  uint8_t ch = (midiChannel  < 1) ? 1 : (midiChannel > 16 ? 16 : midiChannel);
  uint16_t dbpm = defaultBPM;
  if (dbpm < 30) dbpm = 30;
  if (dbpm > 300) dbpm = 300;

  b.startupIsStomp_u8    = startupIsStomp ? 1 : 0;
  b.startupBank_u8       = sb;
  b.altBank_u8           = ab;
  b.midiChannel_u8       = ch;
  b.midiClockEnabled_u8  = midiClockEnabled ? 1 : 0;
  b._pad1                = 0;
  b.defaultBPM_u16       = dbpm;

  writeBlobAndCRC(CFG_ADDR, b);
}

void loadGlobalConfig() {
  GlobalConfigBlob b{};
  if (!readBlobAndCheckCRC(CFG_ADDR, b)) {
    // no valid config → defaults, then persist
    applyDefaults();
    saveGlobalConfig();
    return;
  }

  // Apply to globals
  startupIsStomp   = (b.startupIsStomp_u8 != 0);
  startupBank      = b.startupBank_u8;
  altBank          = b.altBank_u8;
  midiChannel      = b.midiChannel_u8;
  midiClockEnabled = (b.midiClockEnabled_u8 != 0);
  defaultBPM       = b.defaultBPM_u16;
}
