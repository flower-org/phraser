#pragma once

#include "ScreenList.h"

typedef enum {
  UNDEFINED,//If something bad happens, fall back here
  STARTUP_SCREEN,
  UNSEAL,
  BACKUP,
  RESTORE,
  TEST_KEYBOARD,
  UNSEAL_SHOW_PASS,
  CREATE_NEW_DB,
  MAIN_DB_UI,
  TEST
} Mode;

extern Mode currentMode;
extern ListItem** startup_screen_items;

void switchToStartupScreen();
void switchToMainDbUi();
