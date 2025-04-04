#include "UiCommon.h"
#include "MainFolderUi.h"

Mode currentMode = UNDEFINED;

void startupScreenInit() {
  const int startup_screen_item_count = 6;
  ListItem** startup_screen_items = (ListItem**)malloc(startup_screen_item_count * sizeof(ListItem*));

  startup_screen_items[0] = createListItem("Unseal", phraser_Icon_Lock);
  startup_screen_items[1] = createListItem("Backup DB", phraser_Icon_Download);
  startup_screen_items[2] = createListItem("Restore DB", phraser_Icon_Upload);
  startup_screen_items[3] = createListItem("Test Keyboard", phraser_Icon_TextOut);
  startup_screen_items[4] = createListItem("Unseal (show password)", phraser_Icon_Lock);
  startup_screen_items[5] = createListItem("Create New DB", phraser_Icon_Settings);

  initList(startup_screen_items, startup_screen_item_count);
  freeItemList(startup_screen_items, startup_screen_item_count);
}

void switchToStartupScreen() {
  currentMode = STARTUP_SCREEN;
  startupScreenInit();
}

void mainDbUiInit() {
  phraserFolderUiInit();
}

void switchToMainDbUi() {
  currentMode = MAIN_DB_UI;
  mainDbUiInit();
}
