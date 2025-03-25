#include <Thumby.h>
#include <Keyboard.h>

#include <vector>

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
#include "DbRestore.h"
#include "Unseal.h"

#include "Schema_generated.h"
#include "RootTypeFinishingMethods.h"

using namespace phraser; // FlatBuf

unsigned char MAGIC_NUMBER[] = {0xC3, 0xD2, 0xE1, 0xF0};

void startupScreenInit();

// -------------------------------------------------------------------------

// TODO: FlatBuf usage example, for now keep it for the reference, remove later
void playWithWords() {
  flatbuffers::FlatBufferBuilder builder(4096);
  auto name = builder.CreateString("worrd_name");
  auto word = builder.CreateString("worrd_tmpl");
  short word_id = 3;

  //auto name2 = builder.CreateString("worrd_tmpl_2");
  //short word_id2 = 23;

  auto word1 = CreateWord(builder, word_id, 0, name, word, 15, phraser::Icon_GTTriangle);

  builder.Finish(word1);

  uint8_t *buf = builder.GetBufferPointer();
  int size = builder.GetSize();

  //Serial.printf("serialized address %d\n", buf);
  //Serial.printf("serialized size %d\n\n", size);

  const phraser::Word* word_r = GetWord(buf);

  const flatbuffers::String* wordName = word_r->word();
  const char* wordCstr = wordName->c_str();

  //Serial.printf("deserialized template_id %d\n", word_r->word_template_id());
  //Serial.printf("deserialized word %s\n", wordCstr);
}

// ---------- Common ---------- 

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
    case UNSEAL_SHOW_PASS: unsealLoop(thumby); break;
    case BACKUP: backupLoop(thumby); break;
    case RESTORE: restoreLoop(thumby); break;
    case TEST_KEYBOARD: testKeyboardLoop(); break;
    case CREATE_NEW_DB: createNewDbLoop(); break;
  }

  // Update the screen
  thumby->writeBuffer(thumby->getBuffer(), thumby->getBufferSize());
}
