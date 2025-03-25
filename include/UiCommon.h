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
  DB_RUNTIME
} Mode;

extern Mode currentMode;
extern ListItem** startup_screen_items;

void switchToStartupScreen();
void switchToDbRuntime();
