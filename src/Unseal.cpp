#include "Unseal.h"

#include <Thumby.h>
#include <Keyboard.h>
#include "ScreenKeyboard.h"
#include "TextAreaDialog.h"
#include "Adler.h"

#include "pbkdf2-sha256.h"
//#include "Schema_generated.h"
//#include "RootTypeFinishingMethods.h"
#include "Schema_reader.h"

// ---------- Unseal ---------- 

//SHA256 of string "PhraserPasswordManager"
uint8_t HARDCODED_SALT[] = {
  0xE9, 0x8A, 0xD5, 0x84, 0x33, 0xB6, 0xE9, 0xE3,
  0x03, 0x30, 0x6F, 0x29, 0xE0, 0x94, 0x43, 0x8B,
  0x13, 0xA5, 0x52, 0x22, 0xD2, 0x89, 0x0E, 0x5F,
  0x6E, 0x0E, 0xC4, 0x29, 0xFB, 0x40, 0xE2, 0x6D
};
const int HARDCODED_SALT_LEN = 32;

//MD5 of string "PhraserPasswordManager"
uint8_t HARDCODED_IV_MASK[] = {
  0x44, 0x75, 0xBB, 0x91, 0x5E, 0xA8, 0x40, 0xDB,
  0xCE, 0x22, 0xDA, 0x4E, 0x22, 0x4B, 0x8A, 0x3C
};
const int HARDCODED_IV_MASK_LEN = 16;

const int PBKDF_INTERATIONS_COUNT = 10239;

char* password = NULL;
uint8_t* aes256_key_block_key = NULL;
char* debug_hex_key = NULL;
int unseal_phase = 0;
bool unseal_password_mode = false;
bool ui_draw_cycle = false;
int key_block_decrypt_cursor = 0;
int max_key_blocks = 384;
uint32_t latest_found_keyblock_version = 0;

void unsealInit(bool password_mode) {
  // Init duplex UART for Thumby to PC comms
  Keyboard.begin(KeyboardLayout_en_US);
  unseal_phase = 0;
  key_block_decrypt_cursor = 0;
  unseal_password_mode = password_mode;
  password = NULL;
  aes256_key_block_key = NULL;
  debug_hex_key = NULL;
  max_key_blocks = 384;
  latest_found_keyblock_version = 0;
  initOnScreenKeyboard(false, password_mode);
}

int pbkdf2_iterations_count = PBKDF_INTERATIONS_COUNT;
void unsealLoop(Thumby* thumby) {
  if (unseal_phase == 0) {
    // PHASE 0. Password Keyboard
    if (aes256_key_block_key == NULL) {
      char* new_password = specialKeyboardLoop(thumby);
      if (new_password == KB_B_PRESSED) {
        unseal_phase = 1;
        char* text = "Set custom #\nof PBKDF2 iterations?";
        initTextAreaDialog(text, strlen(text), DLG_YES_NO);
      } else if (new_password != NULL) {
        password = new_password;
        ui_draw_cycle = true;
        unseal_phase = 3;
        char* text = "Calculating\nkey...";
        initTextAreaDialog(text, strlen(text), TEXT_AREA);
      }
    }
  } else if (unseal_phase == 1) {
    // PHASE 1. Dialog - custom # of PBKDF iteratins
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_NO) {
      unseal_phase = 0;
    } else if (result == DLG_RES_YES) {
      unseal_phase = 2;
      char buffer[20]; 
      itoa(pbkdf2_iterations_count, buffer, 10);
      initOnScreenKeyboard(buffer, strlen(buffer), false, false, true);
    }
  } else if (unseal_phase == 2) {
    // PHASE 2. Keyboard - numbers only - PBKDF iterations
    char* new_iterations = keyboardLoop(thumby);
    if (new_iterations != NULL) {
      pbkdf2_iterations_count = atoi(new_iterations);
      unsealInit(unseal_password_mode);
    }
  } else if (unseal_phase == 3) {
    // PHASE 3. Calculate PBKDF2
    textAreaLoop(thumby);
    if (ui_draw_cycle) { ui_draw_cycle = false; return; }

    size_t length = strlen(password);

    int aes256_key_length = 32;
    aes256_key_block_key = (uint8_t*)malloc(aes256_key_length);

    PKCS5_PBKDF2_SHA256_HMAC((unsigned char*)password, length,
        HARDCODED_SALT, HARDCODED_SALT_LEN, 
        pbkdf2_iterations_count,
        aes256_key_length, (unsigned char*)aes256_key_block_key);

    char* text = "Trying to find\nand decrypt\nKeyBlock...";
    initTextAreaDialog(text, strlen(text), TEXT_AREA);
    ui_draw_cycle = true;
    unseal_phase = 4;

    // TODO: debug code
    // debug mode - show hex key
    //debug_hex_key = bytesToHexString((const unsigned char*)aes256_key_block_key, aes256_key_length);
    //unseal_phase = 789;
  } else if (unseal_phase == 4) {
    // PHASE 4. Try to decrypt KeyBlock (go up to 384 blocks)
    textAreaLoop(thumby);
    if (ui_draw_cycle) { ui_draw_cycle = false; return; }

    if (key_block_decrypt_cursor >= max_key_blocks) {
      //if key_block_decrypt_cursor >= 384, go to fail screen
      // TODO: AND key block not found
      char text[50];
      sprintf(text, "max key blocks %d version %d", max_key_blocks, latest_found_keyblock_version);
      //TODO: restore fail message
//      char* text = "Failed to find KeyBlock.";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      unseal_phase = -1;
    } else {
      //0. output progress
      char progress[50];
      sprintf(progress, "Looking for a\nKeyBlock\n# %d/%d", key_block_decrypt_cursor, max_key_blocks);
      initTextAreaDialog(progress, strlen(progress), TEXT_AREA);

      //1. Read block at key_block_decrypt_cursor
      uint32_t db_block_size = FLASH_SECTOR_SIZE;
      uint8_t db_block[db_block_size];
      readDbBlockFromFlash(key_block_decrypt_cursor, (void*)db_block);
    
      //2. Decrypt using aes256_key_block_key and HARDCODED_IV_MASK
      uint8_t* iv = xorByteArrays(HARDCODED_IV_MASK, db_block+(db_block_size - HARDCODED_IV_MASK_LEN), HARDCODED_IV_MASK_LEN);
      inPlaceDecryptBlock4096(aes256_key_block_key, iv, db_block);
      free(iv);

      //3. validate Adler32 checksum
      uint32_t length_without_adler = db_block_size - HARDCODED_IV_MASK_LEN - 4;
      uint32_t expected_adler = bytesToUInt32(db_block+length_without_adler);
      uint32_t adler32_checksum = adler32(db_block, length_without_adler);
      if (expected_adler == adler32_checksum) {
        //4. reverse decrypted block
        reverseInPlace(db_block, length_without_adler);

        //5. validate KEY_BLOCK block type (1st byte in a buffer is BlockType)
        if (db_block[0] == phraser_BlockType_KeyBlock) {
          //6. read version of KEY_BLOCK
          //uint16_t size = bytesToUInt16(db_block+1);
          phraser_KeyBlock_table_t key_block;
          if (!(key_block = phraser_KeyBlock_as_root(db_block+3))) {
            return;
          }

          phraser_StoreBlock_struct_t keyblock_storeblock;
          keyblock_storeblock = phraser_KeyBlock_block(key_block);

          uint32_t keyblock_version = phraser_StoreBlock_version(keyblock_storeblock);

          //7. if latest version of KeyBlock found (so far), update `max_key_blocks`
          if (keyblock_version > latest_found_keyblock_version) {
            latest_found_keyblock_version = keyblock_version;

            uint32_t block_count = phraser_KeyBlock_block_count(key_block);
            max_key_blocks = block_count;
          }
        }
      }

      key_block_decrypt_cursor++;
    }
  } else if (unseal_phase == -1) {
    // PHASE -1. Error
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_OK) {
      unseal_phase = -2;
    }
  } else if (unseal_phase == -2) {
    // PHASE -2. Final screen after error
    drawTurnOffMessage(thumby);
  } else if (unseal_phase == 789) {
    // PHASE 789. Debug mode - show password
    drawMessage(thumby, debug_hex_key);
  }
}
