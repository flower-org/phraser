#include "Unseal.h"

#include <Thumby.h>
#include <Keyboard.h>
#include "ScreenKeyboard.h"
#include "TextAreaDialog.h"
#include "Adler.h"
#include "BlockCache.h"
#include "ScreenList.h"

#include "pbkdf2-sha256.h"
#include "Schema_reader.h"

// ---------- Unseal ---------- 

char* password = NULL;
char* debug_hex_key = NULL;
int unseal_phase = 0;
bool unseal_password_mode = false;
bool ui_draw_cycle = false;
int key_block_decrypt_cursor = 0;
uint8_t unseal_bank = 1;
int max_key_blocks = 128;
int pbkdf2_iterations_count = PBKDF_INTERATIONS_COUNT;

void unsealInit(bool password_mode) {
  // Init duplex UART for Thumby to PC comms
  Keyboard.begin(KeyboardLayout_en_US);
  unseal_phase = 0;
  key_block_decrypt_cursor = 0;
  unseal_password_mode = password_mode;

  setLoginData(NULL, 0);

  debug_hex_key = NULL;
  unseal_bank = 1;
  max_key_blocks = 128;
  pbkdf2_iterations_count = PBKDF_INTERATIONS_COUNT;
  initOnScreenKeyboard(false, password_mode);
}

void unsealLoop(Thumby* thumby) {
  if (unseal_phase == 0) {
    // PHASE 0. Password Keyboard
    char* new_password = specialKeyboardLoop(thumby);
    if (new_password == KB_B_PRESSED) {
      unseal_phase = 1;
      char* text = "Set custom #\nof PBKDF2 iterations?";
      initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    } else if (new_password == KB_A_PRESSED) {
      unseal_phase = 111;
      char* text = "Select bank?";
      initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    } else if (new_password != NULL) {
      password = new_password;
      ui_draw_cycle = true;
      unseal_phase = 3;
      char* text = "Calculating\nPBKDF2 key...";
      initTextAreaDialog(text, strlen(text), TEXT_AREA);
    }
  } else if (unseal_phase == 111) {
    // PHASE 111. init banks
    int count = 4;
    ListItem** bank_items = (ListItem**)malloc(count * sizeof(ListItem*));
    bank_items[0] = createListItem("BANK 1", phraser_Icon_Ledger);
    bank_items[1] = createListItem("BANK 2", phraser_Icon_Ledger);
    bank_items[2] = createListItem("BANK 3", phraser_Icon_Ledger);
    bank_items[3] = createListItem("Cancel", phraser_Icon_X);

    initList(bank_items, count, unseal_bank-1);
    freeItemList(bank_items, count);

    unseal_phase = 222;
  } else if (unseal_phase == 222) {
    // PHASE 222. banks list
    int chosenItem = listLoop(thumby);
    if (chosenItem >= 0 && chosenItem <= 2) {
      unseal_bank = chosenItem + 1;
      initOnScreenKeyboard(false, unseal_password_mode);
      unseal_phase = 0;
    } else if (chosenItem == 3) {
      // Cancel
      initOnScreenKeyboard(false, unseal_password_mode);
      unseal_phase = 0;
    }
  } else if (unseal_phase == 1) {
    // PHASE 1. Dialog - custom # of PBKDF iterations
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_NO) {
      initOnScreenKeyboard(false, unseal_password_mode);
      unseal_phase = 0;
    } else if (result == DLG_RES_YES) {
      char buffer[20]; 
      itoa(pbkdf2_iterations_count, buffer, 10);
      initOnScreenKeyboard(buffer, strlen(buffer), false, false, true);
      unseal_phase = 2;
    }
  } else if (unseal_phase == 2) {
    // PHASE 2. Keyboard - numbers only - PBKDF iterations
    char* new_iterations = keyboardLoop(thumby);
    if (new_iterations != NULL) {
      pbkdf2_iterations_count = atoi(new_iterations);
      initOnScreenKeyboard(false, unseal_password_mode);
      unseal_phase = 0;
    }
  } else if (unseal_phase == 3) {
    // PHASE 3. Calculate PBKDF2
    textAreaLoop(thumby);
    if (ui_draw_cycle) { ui_draw_cycle = false; return; }

    size_t length = strlen(password);

    int aes256_key_length = 32;
    uint8_t aes256_key_block_key[aes256_key_length];

    PKCS5_PBKDF2_SHA256_HMAC((unsigned char*)password, length,
        HARDCODED_SALT, HARDCODED_SALT_LEN, 
        pbkdf2_iterations_count,
        aes256_key_length, (unsigned char*)aes256_key_block_key);

    setLoginData(aes256_key_block_key, aes256_key_length);

    char* text = "Trying to find\nand decrypt\nKeyBlock...";
    initTextAreaDialog(text, strlen(text), TEXT_AREA);
    ui_draw_cycle = true;
    startBlockCacheInit();
    unseal_phase = 4;
  } else if (unseal_phase == 555) {
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_NO) {
      //Unseal failed
      char* text = "Failed to find KeyBlock.";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      unseal_phase = -1;
    } else if (result == DLG_RES_YES) {
      if (unseal_bank == 1) {
        max_key_blocks = 384;
      } else if (unseal_bank == 2) {
        max_key_blocks = 256;
      }
      unseal_phase = 4;
    }
  } else if (unseal_phase == 4) {
    // PHASE 4. Try to decrypt KeyBlock (go up to 384 blocks)
    textAreaLoop(thumby);
    if (ui_draw_cycle) { ui_draw_cycle = false; return; }

    if (key_block_decrypt_cursor >= max_key_blocks) {
      if (main_key == NULL) {//i.e. KeyBlock not loaded
        if (max_key_blocks == 128 && (unseal_bank == 1 || unseal_bank == 2)) {
          char* text = "Not found.\nSearch extended range?";
          initTextAreaDialog(text, strlen(text), DLG_YES_NO);
          unseal_phase = 555;
        } else {
          //Unseal failed
          char* text = "Failed to find KeyBlock.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          unseal_phase = -1;
        }
      } else {
        char* text = "Decrypting DB...";
        initTextAreaDialog(text, strlen(text), TEXT_AREA);
        ui_draw_cycle = true;
        key_block_decrypt_cursor = 0;
        unseal_phase = 5;
      }
    } else {
      //0. output progress
      char progress[50];
      sprintf(progress, "Looking for a\nKeyBlock\n# %d/%d", key_block_decrypt_cursor, max_key_blocks);
      initTextAreaDialog(progress, strlen(progress), TEXT_AREA);

      //1. Read block at key_block_decrypt_cursor
      uint32_t db_block_size = FLASH_SECTOR_SIZE;
      uint8_t db_block[db_block_size];
      readDbBlockFromFlashBank(unseal_bank, key_block_decrypt_cursor, (void*)db_block);
    
      //2. Decrypt using aes256_key_block_key and HARDCODED_IV_MASK
      uint8_t* iv = xorByteArrays(HARDCODED_IV_MASK, db_block+(db_block_size - IV_MASK_LEN), IV_MASK_LEN);
      inPlaceDecryptBlock4096(key_block_key, iv, db_block);
      free(iv);

      //3. validate Adler32 checksum
      uint32_t length_without_adler = db_block_size - IV_MASK_LEN - 4;
      uint32_t expected_adler = bytesToUInt32(db_block+length_without_adler);
      uint32_t adler32_checksum = adler32(db_block, length_without_adler);
      if (expected_adler == adler32_checksum) {
        //4. reverse decrypted block
        reverseInPlace(db_block, length_without_adler);

        //5. validate KEY_BLOCK block type (1st byte in a buffer is BlockType)
        if (db_block[0] == phraser_BlockType_KeyBlock) {
          //6. update `max_key_blocks` if needed
          registerBlockInBlockCache(db_block, key_block_decrypt_cursor);
          if (max_block_count > 0) {
            max_key_blocks = max_block_count;
          }
        }
      }

      key_block_decrypt_cursor++;
    }
  } else if (unseal_phase == 5) {
    // PHASE 5. KeyBlock found, decrypt the database
    textAreaLoop(thumby);
    if (ui_draw_cycle) { ui_draw_cycle = false; return; }

    if (key_block_decrypt_cursor >= max_key_blocks) {
      char* text = "DB decryption done";
      initTextAreaDialog(text, strlen(text), TEXT_AREA);

      finalizeBlockCacheInit();

      unseal_phase = 6;
    } else {
      //0. output progress
      char progress[50];
      sprintf(progress, "Decrypting DB\nblocks\n# %d/%d", key_block_decrypt_cursor, max_key_blocks);
      initTextAreaDialog(progress, strlen(progress), TEXT_AREA);

      //1. Read block at key_block_decrypt_cursor
      uint32_t db_block_size = FLASH_SECTOR_SIZE;
      uint8_t db_block[db_block_size];
      readDbBlockFromFlashBank(unseal_bank, key_block_decrypt_cursor, (void*)db_block);
    
      //2. Decrypt using aes256_key_block_key and HARDCODED_IV_MASK
      uint8_t* iv = xorByteArrays(main_iv_mask, db_block+(db_block_size - IV_MASK_LEN), IV_MASK_LEN);
      inPlaceDecryptBlock4096(main_key, iv, db_block);
      free(iv);

      //3. validate Adler32 checksum
      uint32_t length_without_adler = db_block_size - IV_MASK_LEN - 4;
      uint32_t expected_adler = bytesToUInt32(db_block+length_without_adler);
      uint32_t adler32_checksum = adler32(db_block, length_without_adler);
      if (expected_adler == adler32_checksum) {
        //4. reverse decrypted block
        reverseInPlace(db_block, length_without_adler);

        //5. validate KEY_BLOCK block type (1st byte in a buffer is BlockType)
        registerBlockInBlockCache(db_block, key_block_decrypt_cursor);
      }

      key_block_decrypt_cursor++;
    }
  } else if (unseal_phase == 6) {
    // PHASE 6. Database decrypted, run main operation

    textAreaLoop(thumby);
    //TODO: implement
  } else if (unseal_phase == -1) {
    // PHASE -1. Error
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      unseal_phase = -2;
    }
  } else if (unseal_phase == -2) {
    // PHASE -2. Final screen after error
    drawTurnOffMessage(thumby);
  }
}
