#pragma once

#include "ScreenList.h"
#include "hashtable.h"

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

extern ListItem** tmp_screen_items;
extern int tmp_screen_item_cursor;

void switchToStartupScreen();
void switchToMainDbUi();
void build_phrase_template_entries(hashtable *t, uint32_t key, void* value);

