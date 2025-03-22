#include <Thumby.h>
#include <Keyboard.h>

#include "ThumbyUtils.h"
#include "PhraserUtils.h"
#include "ScreenKeyboard.h"
#include "ScreenList.h"
#include "SpecialSymbolDrawer.h"
#include "TextField.h"
#include "Registry.h"

#include "Schema_generated.h"
#include "RootTypeFinishingMethods.h"
#include "pbkdf2-sha256.h"

using namespace phraser; // FlatBuf

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

typedef enum {
  UNDEFINED,//If something bad happens, fall back here
  STARTUP_SCREEN,
  UNSEAL,
  BACKUP,
  RESTORE,
  TEST_KEYBOARD,
  UNSEAL_SHOW_PASS,
  CREATE_NEW_DB,
} Mode;

Mode currentMode;
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

// ---------- Unseal Show Password ---------- 

void unsealShowPassInit() {
  // Init duplex UART for Thumby to PC comms
  Keyboard.begin(KeyboardLayout_en_US);

  initOnScreenKeyboard(false, false);
}

char* aes256KeyBlockKey = NULL;
char* password = NULL;
void unsealShowPassLoop() {
  if (aes256KeyBlockKey == NULL) {
    char* new_password = keyboardLoop(thumby);
    if (new_password != NULL) {
      size_t length = strlen(new_password);

      int key_length = 32;
      aes256KeyBlockKey = (char*)malloc(key_length);

      PKCS5_PBKDF2_SHA256_HMAC((unsigned char*)new_password, length,
          HARDCODED_SALT, HARDCODED_SALT_LEN, 
          PBKDF_INTERATIONS_COUNT,
          key_length, (unsigned char*)aes256KeyBlockKey);

      // TODO: remove output, proceed with store init and AES decryption
      password = bytesToHexString((const unsigned char*)aes256KeyBlockKey, key_length);
    }
  } else {
    drawMessage(thumby, password);
  }
}

// ---------- Unseal ---------- 

void unsealInit() {
  // Init duplex UART for Thumby to PC comms
  Keyboard.begin(KeyboardLayout_en_US);
  initOnScreenKeyboard(false, true);
}

void unsealLoop() {
  if (aes256KeyBlockKey == NULL) {
    char* new_password = keyboardLoop(thumby);
    if (new_password != NULL) {
      size_t length = strlen(new_password);

      int key_length = 32;
      aes256KeyBlockKey = (char*)malloc(key_length);

      PKCS5_PBKDF2_SHA256_HMAC((unsigned char*)new_password, length,
          HARDCODED_SALT, HARDCODED_SALT_LEN, 
          PBKDF_INTERATIONS_COUNT,
          key_length, (unsigned char*)aes256KeyBlockKey);

      // TODO: remove output, proceed with store init and AES decryption
      password = bytesToHexString((const unsigned char*)aes256KeyBlockKey, key_length);
    }
  } else {
    drawMessage(thumby, password);
  }
}

// ---------- DB Backup ---------- 

void backupLoop() {
  drawMessage(thumby, "backupLoop");
}

// ---------- DB Restore ---------- 

void restoreLoop() {
  drawMessage(thumby, "restoreLoop");

  // Make sure RX buffer is empty
  //removeRxBytes();

  //Receive and display a message from link
  //receive(thumby);

  //Serial.begin(115200);
  //delay(1000);

  //Serial.printf("PSM-1 (Phraser)\n");
  //Serial.printf("Thumby (Pi Pico) USB Password Manager\n\n");
}

// ---------- New DB ---------- 

void createNewDbLoop() {
  drawMessage(thumby, "createNewDbLoop");
}

// ---------- Startup Screen ---------- 

ListItem** startup_screen_items = NULL;
const int startup_screen_item_count = 6;
void startupScreenInit() {
  startup_screen_items = (ListItem**)malloc(startup_screen_item_count * sizeof(ListItem*));

  startup_screen_items[0] = createListItem("Unseal", phraser::Icon_Lock);
  startup_screen_items[1] = createListItem("Backup DB", phraser::Icon_Download);
  startup_screen_items[2] = createListItem("Restore DB", phraser::Icon_Upload);
  startup_screen_items[3] = createListItem("Test Keyboard", phraser::Icon_TextOut);
  startup_screen_items[4] = createListItem("Unseal (show password)", phraser::Icon_Lock);
  startup_screen_items[5] = createListItem("Create New DB", phraser::Icon_Settings);

  initList(startup_screen_items, startup_screen_item_count);
}

void releaseStartupScreenContext() {
  freeList(startup_screen_items, startup_screen_item_count);
  startup_screen_items = NULL;
}

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

    releaseStartupScreenContext();

    switch (currentMode) {
      case UNSEAL: unsealInit(); break;
      case BACKUP: break;
      case RESTORE: break;
      case TEST_KEYBOARD: testKeyboardInit(); break;
      case UNSEAL_SHOW_PASS: unsealShowPassInit(); break;
      case CREATE_NEW_DB: break;
    }
  }
}

// ---------- Main logic ---------- 

// Entry point - setup
void setup() {
  // Sets up buttons, audio, link pins, and screen
  thumby->begin();

  currentMode = STARTUP_SCREEN;
  startupScreenInit();

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
    case UNSEAL: unsealLoop(); break;
    case BACKUP: backupLoop(); break;
    case RESTORE: restoreLoop(); break;
    case TEST_KEYBOARD: testKeyboardLoop(); break;
    case UNSEAL_SHOW_PASS: unsealShowPassLoop(); break;
    case CREATE_NEW_DB: createNewDbLoop(); break;
  }

  // Update the screen
  thumby->writeBuffer(thumby->getBuffer(), thumby->getBufferSize());
}
