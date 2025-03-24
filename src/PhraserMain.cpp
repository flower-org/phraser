#include <Thumby.h>
#include <Keyboard.h>

#include "ThumbyUtils.h"
#include "PhraserUtils.h"
#include "ScreenKeyboard.h"
#include "ScreenList.h"
#include "SpecialSymbolDrawer.h"
#include "TextField.h"
#include "Registry.h"
#include "TextAreaDialog.h"
#include "UiCommon.h"
#include "DbBackup.h"

#include "Schema_generated.h"
#include "RootTypeFinishingMethods.h"
#include "pbkdf2-sha256.h"

using namespace phraser; // FlatBuf

unsigned char MAGIC_NUMBER[] = {0xC3, 0xD2, 0xE1, 0xF0};

void startupScreenInit();

// -------------------------------------------------------------------------

// TODO: FlatBuf usage example, for now keep it for the reference, remove later
void playWithWords() {
  flatbuffers::FlatBufferBuilder builder(1024);
  auto name = builder.CreateString("worrd_name");
  auto word = builder.CreateString("worrd_tmpl");
  short word_id = 3;

  //auto name2 = builder.CreateString("worrd_tmpl_2");
  //short word_id2 = 23;

  auto word1 = CreateWord(builder, word_id, 0, name, word, 15, phraser::Icon_GTTriangle);

  builder.Finish(word1);

  uint8_t *buf = builder.GetBufferPointer();
  int size = builder.GetSize();

  Serial.printf("serialized address %d\n", buf);
  Serial.printf("serialized size %d\n\n", size);

  const phraser::Word* word_r = GetWord(buf);

  const flatbuffers::String* wordName = word_r->word();
  const char* wordCstr = wordName->c_str();

  Serial.printf("deserialized template_id %d\n", word_r->word_template_id());
  Serial.printf("deserialized word %s\n", wordCstr);
}

// ---------- Common ---------- 

const int PBKDF_INTERATIONS_COUNT = 10000;


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

Thumby* thumby = new Thumby();

// ---------- Undefined ---------- 

void undefinedLoop() {
  symbolLoop(thumby);
}

// ---------- Test Keyboard ---------- 

void testKeyboardInit() {
  // Init duplex UART for Thumby to PC comms
  Keyboard.begin(KeyboardLayout_en_US);

  initOnScreenKeyboard(true, false);
}

void testKeyboardLoop() {
  keyboardLoop(thumby);
}

// ---------- Unseal ---------- 

char* aes256KeyBlockKey = NULL;
char* password = NULL;
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
void unsealLoop() {
  if (unseal_phase == 0) {
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
  
        // TODO: remove output, proceed with store init and AES decryption
        password = bytesToHexString((const unsigned char*)aes256KeyBlockKey, key_length);
      }
    } else {
      drawMessage(thumby, password);
    }
  } else if (unseal_phase == 1) {
    DialogResult result = textAreaLoop(thumby);
    Serial.printf("Result: %d\r\n", (int)result);
    if (result == DLG_RES_NO) {
      unseal_phase = 0;
    } else if (result == DLG_RES_YES) {
      unseal_phase = 2;
      char buffer[20]; 
      itoa(pbkdf2_iterations_count, buffer, 10);
      initOnScreenKeyboard(buffer, strlen(buffer), false, false, true);
    }
  } else if (unseal_phase == 2) {
    // Keyboard - numbers only
    char* new_iterations = keyboardLoop(thumby);
    if (new_iterations != NULL) {
      pbkdf2_iterations_count = atoi(new_iterations);
      unsealInit(unseal_password_mode);
    }
  }
}

// ---------- DB Backup ---------- 

// ---------- New DB ---------- 

int create_new_db_phase = 0;
void createNewDbInit() {
  create_new_db_phase = 0;
  char* text = "Create DB?\nExisting data\nwill be lost!";
  initTextAreaDialog(text, strlen(text), DLG_YES_NO);
}

void createNewDbLoop() {
  if (create_new_db_phase == 0) {
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_YES) {
      create_new_db_phase = 1;
      char* text = "Create DB\nnot implemented";
      initTextAreaDialog(text, strlen(text), DLG_OK);
    } else if (result == DLG_RES_NO) {
      switchToStartupScreen();
    }
  } else if (create_new_db_phase == 1) {
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      create_new_db_phase = 2;
    }
  } else if (create_new_db_phase == 2) {
    drawTurnOffMessage(thumby);
  }
}

// ---------- DB Restore ---------- 

int restore_phase = 0;
void restoreInit() {
  restore_phase = 0;
  char* text = "Restore DB?\nExisting data\nwill be lost!";
  initTextAreaDialog(text, strlen(text), DLG_YES_NO);
}

void restoreLoop() {
  if (restore_phase == 0) {
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_YES) {
      restore_phase = 1;
      char* text = "Restore DB\nnot implemented";
      initTextAreaDialog(text, strlen(text), DLG_OK);
    } else if (result == DLG_RES_NO) {
      switchToStartupScreen();
    }
  } else if (restore_phase == 1) {
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      restore_phase = 2;
    }
  } else if (restore_phase == 2) {
    drawTurnOffMessage(thumby);
  }

  // Make sure RX buffer is empty
  //removeRxBytes();

  //Receive and display a message from link
  //receive(thumby);

  //Serial.begin(115200);
  //delay(1000);

  //Serial.printf("PSM-1 (Phraser)\n");
  //Serial.printf("Thumby (Pi Pico) USB Password Manager\n\n");
}

// ---------- Startup Screen ---------- 


void startupScreenLoop() {
  ListItem* chosenItem = listLoop(thumby);
  if (chosenItem != NULL) {
    currentMode = UNDEFINED;
    if (chosenItem == startup_screen_items[0]) { currentMode = UNSEAL; } 
    else if (chosenItem == startup_screen_items[1]) { currentMode = BACKUP; } 
    else if (chosenItem == startup_screen_items[2]) { currentMode = RESTORE; } 
    else if (chosenItem == startup_screen_items[3]) { currentMode = TEST_KEYBOARD; } 
    else if (chosenItem == startup_screen_items[4]) { currentMode = UNSEAL_SHOW_PASS; } 
    else if (chosenItem == startup_screen_items[5]) { currentMode = CREATE_NEW_DB; }

    switch (currentMode) {
      case UNSEAL: unsealInit(true); break;
      case BACKUP: backupInit(); break;
      case RESTORE: restoreInit(); break;
      case TEST_KEYBOARD: testKeyboardInit(); break;
      case UNSEAL_SHOW_PASS: unsealInit(false); break;
      case CREATE_NEW_DB: createNewDbInit(); break;
    }
  }
}

// ---------- Main logic ---------- 


// Entry point - setup
void setup() {
  Serial.begin(115200);
  // Sets up buttons, audio, link pins, and screen
  thumby->begin();

  switchToStartupScreen();

  playStartupSound(thumby);

//  play_with_words();
//  loadRegistryFromFlash();
}

// Entry point - loop
void loop() {
  // Clear the screen to black
  thumby->clear();

  switch (currentMode) {
    case UNDEFINED: undefinedLoop(); break;
    case STARTUP_SCREEN: startupScreenLoop(); break;
    case UNSEAL:
    case UNSEAL_SHOW_PASS: unsealLoop(); break;
    case BACKUP: backupLoop(thumby); break;
    case RESTORE: restoreLoop(); break;
    case TEST_KEYBOARD: testKeyboardLoop(); break;
    case CREATE_NEW_DB: createNewDbLoop(); break;
  }

  // Update the screen
  thumby->writeBuffer(thumby->getBuffer(), thumby->getBufferSize());
}
