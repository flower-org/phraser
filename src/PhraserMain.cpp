#include <Thumby.h>
#include <Keyboard.h>

#include "ThumbyUtils.h"
#include "PhraserUtils.h"
#include "ScreenKeyboard.h"
#include "ScreenList.h"
#include "SpecialSymbolDrawer.h"
#include "TextField.h"
#include "TextAreaDialog.h"
#include "UiCommon.h"
#include "DbBackup.h"
#include "DbRestore.h"
#include "Unseal.h"
#include "DbCreate.h"
#include "DebugTest.h"
#include "SerialUtils.h"
#include "MainDbUi.h"

// ---------- Common ---------- 

Thumby* thumby = new Thumby();
  
// ---------- Undefined ---------- 
// TODO: change to "About" screen
void undefinedLoop() {
  symbolLoop(thumby);
}

// ---------- USB Keyboard Test ---------- 

void testKeyboardInit() {
  Keyboard.begin(KeyboardLayout_en_US);
  initOnScreenKeyboard(true, false);
}

void testKeyboardLoop() {
  keyboardLoop(thumby);
}

// ---------- Startup Screen ---------- 

void startupScreenLoop() {
  int chosenItem = listLoop(thumby);
  if (chosenItem != -1) {
    currentMode = UNDEFINED;
    if (chosenItem == 0) { currentMode = UNSEAL; } 
    else if (chosenItem == 1) { currentMode = BACKUP; } 
    else if (chosenItem == 2) { currentMode = RESTORE; } 
    else if (chosenItem == 3) { currentMode = TEST_KEYBOARD; } 
    else if (chosenItem == 4) { currentMode = UNSEAL_SHOW_PASS; } 
    else if (chosenItem == 5) { currentMode = CREATE_NEW_DB; }

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

// ---------- Developer Tests ---------- 

int test_phase = 0;
void initDevTestScreen() {
  currentMode = TEST;
  test_phase = 0;

  char* text = "Run dev test.";
  initTextAreaDialog(text, strlen(text), DLG_OK);
}

void testLoop() {
  if (test_phase == 0) {
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      test_phase = 1;
    }
  } else if (test_phase == 1) {
    test_phase = 2;

    debugTest();
  } else if (test_phase == 2) {
    serialDebugPrintf("\r\nEND!\r\n");
    char* text = "Done.";
    initTextAreaDialog(text, strlen(text), DLG_OK);
    test_phase = 3;
  } else if (test_phase == 3) {
    textAreaLoop(thumby);
  }
}

// ---------- Main logic ---------- 

// Entry point - setup
void setup() {
  thumby->begin();
  switchToStartupScreen();
  playStartupSound(thumby);
//  initDevTestScreen(); // Uncomment this to run dev tests
}

// Entry point - loop
void loop() {
  // Clear the screen to black
  thumby->clear();

  switch (currentMode) {
    case TEST: testLoop(); break;
    case UNDEFINED: undefinedLoop(); break;
    case STARTUP_SCREEN: startupScreenLoop(); break;
    case UNSEAL:
    case UNSEAL_SHOW_PASS: unsealLoop(thumby); break;
    case BACKUP: backupLoop(thumby); break;
    case RESTORE: restoreLoop(thumby); break;
    case TEST_KEYBOARD: testKeyboardLoop(); break;
    case CREATE_NEW_DB: createNewDbLoop(thumby); break;
    case MAIN_DB_UI: phraserDbUiLoop(thumby); break;
  }

  // Update the screen
  thumby->writeBuffer(thumby->getBuffer(), thumby->getBufferSize());
}
