#include "Unseal.h"

#include <Thumby.h>
#include <Keyboard.h>
#include "ScreenKeyboard.h"
#include "TextAreaDialog.h"

#include "pbkdf2-sha256.h"

// ---------- Unseal ---------- 

//SHA256 of string "PhraserPasswordManager"
unsigned char HARDCODED_SALT[] = {
  0xE9, 0x8A, 0xD5, 0x84, 0x33, 0xB6, 0xE9, 0xE3,
  0x03, 0x30, 0x6F, 0x29, 0xE0, 0x94, 0x43, 0x8B,
  0x13, 0xA5, 0x52, 0x22, 0xD2, 0x89, 0x0E, 0x5F,
  0x6E, 0x0E, 0xC4, 0x29, 0xFB, 0x40, 0xE2, 0x6D
};
const int HARDCODED_SALT_LEN = 32;

//MD5 of string "PhraserPasswordManager"
unsigned char HARDCODED_IV_MASK[] = {
  0x44, 0x75, 0xBB, 0x91, 0x5E, 0xA8, 0x40, 0xDB,
  0xCE, 0x22, 0xDA, 0x4E, 0x22, 0x4B, 0x8A, 0x3C
};
const int HARDCODED_IV_MASK_LEN = 16;

const int PBKDF_INTERATIONS_COUNT = 10239;

char* aes256KeyBlockKey = NULL;
char* debug_hex_key = NULL;
int unseal_phase = 0;
bool unseal_password_mode = false;

void unsealInit(bool password_mode) {
  // Init duplex UART for Thumby to PC comms
  Keyboard.begin(KeyboardLayout_en_US);
  unseal_phase = 0;
  unseal_password_mode = password_mode;
  initOnScreenKeyboard(false, password_mode);
}

int pbkdf2_iterations_count = PBKDF_INTERATIONS_COUNT;
void unsealLoop(Thumby* thumby) {
  if (unseal_phase == 0) {
    // 0. Password Keyboard
    if (aes256KeyBlockKey == NULL) {
      char* new_password = specialKeyboardLoop(thumby);
      if (new_password == KB_B_PRESSED) {
        unseal_phase = 1;
        char* text = "Set custom #\nof PBKDF2 iterations?";
        initTextAreaDialog(text, strlen(text), DLG_YES_NO);
      } else if (new_password != NULL) {
        size_t length = strlen(new_password);

        int key_length = 32;
        aes256KeyBlockKey = (char*)malloc(key_length);

        PKCS5_PBKDF2_SHA256_HMAC((unsigned char*)new_password, length,
            HARDCODED_SALT, HARDCODED_SALT_LEN, 
            pbkdf2_iterations_count,
            key_length, (unsigned char*)aes256KeyBlockKey);

        // debug mode - show hex key
        debug_hex_key = bytesToHexString((const unsigned char*)aes256KeyBlockKey, key_length);
        unseal_phase = 789;
      }
    }
  } else if (unseal_phase == 1) {
    // 1. Dialog - custom # of PBKDF iteratins
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
    // 2. Keyboard - numbers only - PBKDF iterations
    char* new_iterations = keyboardLoop(thumby);
    if (new_iterations != NULL) {
      pbkdf2_iterations_count = atoi(new_iterations);
      unsealInit(unseal_password_mode);
    }
  } else if (unseal_phase == 789) {
    // 789. Debug mode - show password
    drawMessage(thumby, debug_hex_key);
  }
}
