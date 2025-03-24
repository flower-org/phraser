#include "DbBackup.h"
#include "TextAreaDialog.h"
#include "UiCommon.h"
#include "BlockStore.h"
#include "PhraserUtils.h"

const uint8_t magicNumber[] = { 0xC3, 0xD2, 0xE1, 0xF0 };

const uint8_t HELLO = 1;
const uint8_t BYE = 2;
const uint8_t START_BACKUP = 3;
const uint8_t START_BACKUP_CONFIRMATION = 4;
const uint8_t START_RESTORE = 5;
const uint8_t START_RESTORE_CONFIRMATION = 6;
const uint8_t BACKUP_BLOCK = 7;
const uint8_t BACKUP_BLOCK_RECEIVED = 8;
const uint8_t RESTORE_BLOCK = 9;
const uint8_t RESTORE_BLOCK_RECEIVED = 10;

const uint8_t DATA_CORRECTNESS_ERROR = 254;

void sendHelloSerial() {
  Serial.write(magicNumber, 4);
  Serial.write(HELLO);
}

void sendStartBackupSerial() {
  Serial.write(magicNumber, 4);
  Serial.write(START_BACKUP);
}

void sendBye() {
  Serial.write(magicNumber, 4);
  Serial.write(BYE);
}

void sendUInt32Serial(uint32_t value) {
  uint8_t bytes[4];
  bytes[0] = (value >> 24) & 0xFF;
  bytes[1] = (value >> 16) & 0xFF;
  bytes[2] = (value >> 8) & 0xFF;
  bytes[3] = value & 0xFF;
  Serial.write(bytes, 4);
}

void sendBlockSerial(uint32_t block_number) {
  Serial.write(magicNumber, 4);
  Serial.write(BACKUP_BLOCK);
  sendUInt32Serial(block_number);

  uint32_t db_block_size = FLASH_SECTOR_SIZE;
  uint8_t dbBlock[db_block_size];

  sendUInt32Serial(db_block_size);

  readDbBlockFromFlash(block_number, (void*)dbBlock);
  uint32_t adler32Checksum = adler32(dbBlock, db_block_size);

  Serial.write(dbBlock, db_block_size);
  sendUInt32Serial(adler32Checksum);
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

  uint32_t result = (static_cast<uint32_t>(bytes[0]) << 24) |
                    (static_cast<uint32_t>(bytes[1]) << 16) |
                    (static_cast<uint32_t>(bytes[2]) << 8)  |
                    (static_cast<uint32_t>(bytes[3]));

  return result;
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

  if (checkMagicNumber[0] != magicNumber[0] ||
    checkMagicNumber[1] != magicNumber[1] ||
    checkMagicNumber[2] != magicNumber[2] ||
    checkMagicNumber[3] != magicNumber[3]) {
    return DATA_CORRECTNESS_ERROR;
  }

  uint8_t op = Serial.read();
  return op;
}

uint32_t backup_block_ctr;
uint32_t backup_block_count;
bool waiting_for_block_ack;
int backup_phase;

void backupInit() {
  backup_phase = 0;
  backup_block_ctr = 0;
  backup_block_count = 0;// Will be acquired via protocol from the station
  waiting_for_block_ack = false;
  char* text = "Backup DB?";
  initTextAreaDialog(text, strlen(text), DLG_YES_NO);
}

void backupLoop(Thumby* thumby) {
  if (backup_phase == 0) { // init YesNo dialog
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_YES) {
      backup_phase = 1;
      char* text = "Sending HELLO periodically to the station.";
      initTextAreaDialog(text, strlen(text), TEXT_AREA);

      Serial.begin(115200);
    } else if (result == DLG_RES_NO) {
      switchToStartupScreen();
    }
  } else if (backup_phase == 1) { // YesNo dialog result Yes, start serial sequence 
    textAreaLoop(thumby);

    // Spam Hello's, since initial messages are not guaranteed to be receieved by the station.
    sendHelloSerial();
    delay(100);

    if (canReadOperationSerial()) {
      uint8_t op = readOperationSerial();
      if (op == DATA_CORRECTNESS_ERROR) {
        char* text = "Protocol error - data correctness.";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        backup_phase = -1;
      } else {
        if (op == HELLO) {
          // Send START_BACKUP
          sendStartBackupSerial();
          backup_phase = 2;

          char* text = "START_BACKUP sent to the station.";
          initTextAreaDialog(text, strlen(text), TEXT_AREA);
        } else {
          char* text = "Protocol error - Unexpected operation.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          backup_phase = -1;
          }
      }
    }
  } else if (backup_phase == 2) {
    // START_BACKUP was sent, wait for START_BACKUP_CONFIRMATION
    textAreaLoop(thumby);
    
    if (canReadOperationSerial()) {
      uint8_t op = readOperationSerial();
      if (op == DATA_CORRECTNESS_ERROR) {
        char* text = "Protocol error - data correctness.";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        backup_phase = -1;
      } else {
        if (op == START_BACKUP_CONFIRMATION) {
          backup_phase = 3;
          // block count to backup acquired from START_BACKUP_CONFIRMATION - DB is encrypted!
          backup_block_count = readUInt32Serial();

          char* text = "START_BACKUP_CONFIRMATION received from the station.";
          initTextAreaDialog(text, strlen(text), TEXT_AREA);
        } else {
          char* text = "Protocol error - Unexpected operation.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          backup_phase = -1;
          }
      }
    }
  } else if (backup_phase == 3) {
    // Send 256 BACKUP_BLOCKs followed by BYE
    textAreaLoop(thumby);
    
    if (backup_block_ctr > backup_block_count) {
      sendBye();
      backup_phase = 4;
      char* text = "BYE sent to the station.";
      initTextAreaDialog(text, strlen(text), TEXT_AREA);
    } else if (!waiting_for_block_ack) {
      sendBlockSerial(backup_block_ctr);
      waiting_for_block_ack = true;

      char text[50];
      sprintf(text, "Backing up block# %d/%d", backup_block_ctr, backup_block_count);

      initTextAreaDialog(text, strlen(text), TEXT_AREA);
    } else {
      if (canReadOperationSerial()) {
        uint8_t op = readOperationSerial();
        if (op == DATA_CORRECTNESS_ERROR) {
          char* text = "Protocol error - data correctness.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          backup_phase = -1;
        } else {
          if (op == BACKUP_BLOCK_RECEIVED) {
            while (!canReadUInt32Serial()) {}
            // get block number to verify
            uint32_t check_block_number = readUInt32Serial();
            if (check_block_number != backup_block_ctr) {
              char* text = "Protocol error - Block order.";
              initTextAreaDialog(text, strlen(text), DLG_OK);
              backup_phase = -1;
            }

            backup_block_ctr++;
            waiting_for_block_ack = false;
          } else {
            char* text = "Protocol error - Unexpected operation.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
            backup_phase = -1;
          }
        }
      }
    }
  } else if (backup_phase == 4) {
    // BYE was sent, wait for BYE from the station
    textAreaLoop(thumby);
    
    if (canReadOperationSerial()) {
      uint8_t op = readOperationSerial();
      if (op == DATA_CORRECTNESS_ERROR) {
        char* text = "Protocol error - data correctness.";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        backup_phase = -1;
      } else {
        if (op == BYE) {
          backup_phase = 5;

          char* text = "DB Backup successfully finished.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
        } else {
          char* text = "Protocol error - Unexpected operation.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          backup_phase = -1;
          }
      }
    }
  } else if (backup_phase == 5) {
    // Show message about operation success, with transition to final screen
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      backup_phase = 222;
    }
  } else if (backup_phase == 222) {
    // Final screen
    Serial.end();
    drawTurnOffMessage(thumby);
  } else if (backup_phase == -1) {
    // Error
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      backup_phase = 222;
    }
  }
}
