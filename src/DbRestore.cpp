#include "DbRestore.h"
#include "TextAreaDialog.h"
#include "UiCommon.h"
#include "PhraserUtils.h"
#include "SerialUtils.h"
#include "Adler.h"

uint32_t restore_block_ctr;
uint8_t restore_bank;
uint32_t restore_block_count;
int restore_phase;

void restoreInit() {
  restore_phase = 0;
  restore_block_ctr = 0;
  restore_bank = 0;// Will be acquired via protocol from the station
  restore_block_count = 0;// Will be acquired via protocol from the station
  char* text = "Restore DB?\nExisting data\nwill be lost!";
  initTextAreaDialog(text, strlen(text), DLG_YES_NO);
}

void restoreLoop(Thumby* thumby) {
  if (restore_phase == 0) { // init YesNo dialog
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_YES) {
      restore_phase = 1;
      char* text = "Sending HELLO periodically to the station.";
      initTextAreaDialog(text, strlen(text), TEXT_AREA);

      Serial.begin(115200);
    } else if (result == DLG_RES_NO) {
      switchToStartupScreen();
    }
  } else if (restore_phase == 1) { // YesNo dialog result Yes, start serial sequence 
    textAreaLoop(thumby);

    // Spam Hello's, since initial messages are not guaranteed to be receieved by the station.
    sendHelloSerial();
    delay(100);

    if (canReadOperationSerial()) {
      uint8_t op = readOperationSerial();
      if (op == DATA_CORRECTNESS_ERROR) {
        char* text = "Protocol error - data correctness.";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        restore_phase = -1;
      } else {
        if (op == HELLO) {
          // Send START_RESTORE
          sendStartRestoreSerial();
          restore_phase = 2;

          char* text = "START_RESTORE sent to the station.";
          initTextAreaDialog(text, strlen(text), TEXT_AREA);
        } else {
          char* text = "Protocol error - Unexpected operation.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          restore_phase = -1;
          }
      }
    }
  } else if (restore_phase == 2) {
    // START_RESTORE was sent, wait for START_RESTORE_CONFIRMATION
    textAreaLoop(thumby);
    
    if (canReadOperationSerial()) {
      uint8_t op = readOperationSerial();
      if (op == DATA_CORRECTNESS_ERROR) {
        char* text = "Protocol error - data correctness.";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        restore_phase = -1;
      } else {
        if (op == START_RESTORE_CONFIRMATION) {
          // block count to restore acquired from START_RESTORE_CONFIRMATION
          while (!canReadByteSerial()) {}
          restore_bank = readByteSerial();
          while (!canReadUInt32Serial()) {}
          restore_block_count = readUInt32Serial();

          if (restore_bank != 1 && restore_bank != 2 && restore_bank != 3) {
            char text[50];
            sprintf(text, "Unknown Bank# %d", restore_bank);
            initTextAreaDialog(text, strlen(text), DLG_OK);
            restore_phase = -1;
          } else if ((restore_bank == 1 && restore_block_count > 384) 
            || (restore_bank == 2 && restore_block_count > 256) 
            || (restore_bank == 3 && restore_block_count > 128)) {
            char text[50];
            sprintf(text, "Too many blocks: Bank# %d; block count: %d", restore_bank, restore_block_count);
            initTextAreaDialog(text, strlen(text), DLG_OK);
            restore_phase = -1;
          } else {
            char* text = "START_RESTORE_CONFIRMATION received from the station.";
            initTextAreaDialog(text, strlen(text), TEXT_AREA);
            restore_phase = 3;
          }
        } else {
          char* text = "Protocol error - Unexpected operation.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          restore_phase = -1;
        }
      }
    }
  } else if (restore_phase == 3) {
    // Receive and save `restore_block_count` RESTORE_BLOCKs
    textAreaLoop(thumby);
    
    if (restore_block_ctr >= restore_block_count) {
      restore_phase = 4;
      char* text = "Awaiting for BYE from the station.";
      initTextAreaDialog(text, strlen(text), TEXT_AREA);
    } else {
      if (canReadOperationSerial()) {
        uint8_t op = readOperationSerial();
        if (op == DATA_CORRECTNESS_ERROR) {
          char* text = "Protocol error - RESTORE_BLOCK data correctness.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          restore_phase = -1;
        } else {
          if (op == RESTORE_BLOCK) {
            // get bank
            while (!canReadByteSerial()) {}
            uint8_t rcv_bank = readByteSerial();
            // get block number
            while (!canReadUInt32Serial()) {}
            uint32_t rcv_block_number = readUInt32Serial();

            if (rcv_bank != restore_bank) {
              char* text = "Protocol error - Block bank.";
              initTextAreaDialog(text, strlen(text), DLG_OK);
              restore_phase = -1;
            } else if (rcv_block_number != restore_block_ctr) {
              char* text = "Protocol error - Block order.";
              initTextAreaDialog(text, strlen(text), DLG_OK);
              restore_phase = -1;
            } else {
              // get data length
              while (!canReadUInt32Serial()) {}
              uint32_t data_length = readUInt32Serial();

              // TODO: wait for data to become available
              // get data block
              uint8_t data[data_length];
              Serial.readBytes(data, data_length);

              // get adler32
              while (!canReadUInt32Serial()) {}
              uint32_t adler32_checksum = readUInt32Serial();

              uint32_t adler32_calc = adler32(data, data_length);

              if (adler32_calc != adler32_checksum) {
                char* text = "Protocol error - Adler checksum.";
                initTextAreaDialog(text, strlen(text), DLG_OK);
                restore_phase = -1;
              } else {
                char text[50];
                sprintf(text, "Restoring block# %d/%d", restore_block_ctr, restore_block_count);
                initTextAreaDialog(text, strlen(text), TEXT_AREA);

                // write block to flash
                writeDbBlockToFlashBank(restore_bank, restore_block_ctr, data);

                sendRestoreBlockReceivedSerial(restore_bank, restore_block_ctr);
                restore_block_ctr++;
              }
            }
          } else {
            char* text = "Protocol error - Unexpected operation.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
            restore_phase = -1;
          }
        }
      }
    }
  } else if (restore_phase == 4) {
    // last block was received, waiting for BYE from the station
    textAreaLoop(thumby);
    
    if (canReadOperationSerial()) {
      uint8_t op = readOperationSerial();
      if (op == DATA_CORRECTNESS_ERROR) {
        char* text = "Protocol error - data correctness.";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        restore_phase = -1;
      } else {
        if (op == BYE) {
          restore_phase = 5;
          sendBye();

          char text[100];
          sprintf(text, "DB Restore successfully finished.\nBank# %d; block count: %d", restore_bank, restore_block_count);
          initTextAreaDialog(text, strlen(text), DLG_OK);
        } else {
          char* text = "Protocol error - Unexpected operation.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          restore_phase = -1;
          }
      }
    }
  } else if (restore_phase == 5) {
    // Show message about operation success, with transition to final screen
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      restore_phase = 222;
    }
  } else if (restore_phase == 222) {
    // Final screen
    Serial.end();
    drawTurnOffMessage(thumby);
  } else if (restore_phase == -1) {
    // Error
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      restore_phase = 222;
    }
  }
}
