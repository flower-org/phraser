#include "UiCommon.h"

Mode currentMode = UNDEFINED;

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

void switchToStartupScreen() {
  currentMode = STARTUP_SCREEN;
  startupScreenInit();
}
