#include "SerialUtils.h"
#include "PhraserUtils.h"
#include "Adler.h"

const uint8_t magic_number_[] = { 0xC3, 0xD2, 0xE1, 0xF0 };
const uint8_t* magic_number = magic_number_;

void sendHelloSerial() {
  Serial.write(magic_number, 4);
  Serial.write(HELLO);
}

void sendStartBackupSerial() {
  Serial.write(magic_number, 4);
  Serial.write(START_BACKUP);
}

void sendBye() {
  Serial.write(magic_number, 4);
  Serial.write(BYE);
}

void sendByteSerial(uint8_t value) {
  Serial.write(value);
}

void sendUInt32Serial(uint32_t value) {
  uint8_t bytes[4];
  uInt32ToBytes(value, bytes);
  Serial.write(bytes, 4);
}

void sendBlockSerial(uint8_t backup_bank, uint32_t block_number) {
  Serial.write(magic_number, 4);
  Serial.write(BACKUP_BLOCK);
  sendByteSerial(backup_bank);
  sendUInt32Serial(block_number);

  uint32_t db_block_size = FLASH_SECTOR_SIZE;
  uint8_t dbBlock[db_block_size];

  sendUInt32Serial(db_block_size);

  readDbBlockFromFlashBank(backup_bank, block_number, (void*)dbBlock);
  uint32_t adler32Checksum = adler32(dbBlock, db_block_size);

  Serial.write(dbBlock, db_block_size);
  sendUInt32Serial(adler32Checksum);
}

void sendRestoreBlockReceivedSerial(uint8_t bank, uint32_t block_number) {
  Serial.write(magic_number, 4);
  Serial.write(RESTORE_BLOCK_RECEIVED);
  sendByteSerial(bank);
  sendUInt32Serial(block_number);
}

boolean canReadByteSerial() {
  return Serial.available() >= 1;
}

uint8_t readByteSerial() {
  uint8_t b = Serial.read();
  return b;
}

boolean canReadUInt32Serial() {
  return Serial.available() >= 4;
}

uint32_t readUInt32Serial() {
  uint8_t bytes[4];
  bytes[0] = Serial.read();
  bytes[1] = Serial.read();
  bytes[2] = Serial.read();
  bytes[3] = Serial.read();

  return bytesToUInt32(bytes);
}

boolean canReadOperationSerial() {
  return Serial.available() >= 5;
}

uint8_t readOperationSerial() {
  uint8_t checkMagicNumber[4];
  checkMagicNumber[0] = Serial.read();
  checkMagicNumber[1] = Serial.read();
  checkMagicNumber[2] = Serial.read();
  checkMagicNumber[3] = Serial.read();

  if (checkMagicNumber[0] != magic_number[0] ||
    checkMagicNumber[1] != magic_number[1] ||
    checkMagicNumber[2] != magic_number[2] ||
    checkMagicNumber[3] != magic_number[3]) {
    return DATA_CORRECTNESS_ERROR;
  }

  uint8_t op = Serial.read();
  return op;
}

void sendStartRestoreSerial() {
  Serial.write(magic_number, 4);
  Serial.write(START_RESTORE);
}

bool is_debug = false;
// printf-like functions and stdargs in C have some terminal issues forwarding arguments, 
// so I had to just copy and paste the impl pretty much
size_t serialDebugPrintf(const char *format, ...) {
  if (is_debug) {
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
            return 0;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    len = Serial.write((const uint8_t*) buffer, len);
    if (buffer != temp) {
        delete[] buffer;
    }
    return len;
  }
  return 0;
}