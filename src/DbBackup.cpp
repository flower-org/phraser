#include "DbBackup.h"
#include "TextAreaDialog.h"
#include "UiCommon.h"
#include "PhraserUtils.h"
#include "SerialUtils.h"

uint32_t backup_block_ctr;
uint8_t backup_bank;
uint32_t backup_block_count;
bool waiting_for_block_ack;
int backup_phase;

void backupInit() {
  backup_phase = 0;
  backup_block_ctr = 0;
  backup_bank = 0;// Will be acquired via protocol from the station
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
      char* text = "Sending HELLO\nperiodically\nto the station.";
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
          // block count to backup acquired from START_BACKUP_CONFIRMATION - DB is encrypted!
          while (!canReadByteSerial()) {}
          backup_bank = readByteSerial();
          while (!canReadUInt32Serial()) {}
          backup_block_count = readUInt32Serial();

          if (backup_bank != 1 && backup_bank != 2 && backup_bank != 3) {
            char text[50];
            sprintf(text, "Unknown Bank# %d", backup_bank);
            initTextAreaDialog(text, strlen(text), DLG_OK);
            backup_phase = -1;
          } else if ((backup_bank == 1 && backup_block_count > 384) 
            || (backup_bank == 2 && backup_block_count > 256) 
            || (backup_bank == 3 && backup_block_count > 128)) {
            char text[50];
            sprintf(text, "Too many blocks: Bank# %d; block count: %d", backup_bank, backup_block_count);
            initTextAreaDialog(text, strlen(text), DLG_OK);
            backup_phase = -1;
          } else {
            char* text = "START_BACKUP_CONFIRMATION received from the station.";
            initTextAreaDialog(text, strlen(text), TEXT_AREA);
            backup_phase = 3;
          }
        } else {
          char* text = "Protocol error - Unexpected operation.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          backup_phase = -1;
        }
      }
    }
  } else if (backup_phase == 3) {
    // Send `backup_block_count` BACKUP_BLOCKs
    textAreaLoop(thumby);
    
    if (backup_block_ctr >= backup_block_count) {
      sendBye();
      backup_phase = 4;
      char* text = "BYE sent to the station.";
      initTextAreaDialog(text, strlen(text), TEXT_AREA);
    } else if (!waiting_for_block_ack) {
      sendBlockSerial(backup_bank, backup_block_ctr);
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
            // get bank number to verify
            while (!canReadByteSerial()) {}
            uint32_t check_bank = readByteSerial();
            // get block number to verify
            while (!canReadUInt32Serial()) {}
            uint32_t check_block_number = readUInt32Serial();

            if (check_bank != backup_bank) {
              char* text = "Protocol error - Block bank.";
              initTextAreaDialog(text, strlen(text), DLG_OK);
              backup_phase = -1;
            } else if (check_block_number != backup_block_ctr) {
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

          char text[100];
          sprintf(text, "DB Backup successfully finished.\nBank# %d; block count: %d", backup_bank, backup_block_count);
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
