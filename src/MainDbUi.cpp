#include "MainDbUi.h"

#include "TextAreaDialog.h"
#include "ScreenList.h"

void phraserDbUiInit() {
  const int startup_screen_item_count = 6;
  ListItem** startup_screen_items = (ListItem**)malloc(startup_screen_item_count * sizeof(ListItem*));

  startup_screen_items[0] = createListItem("Folder1", phraser_Icon_Folder);
  startup_screen_items[1] = createListItem("Folder2", phraser_Icon_Folder);
  startup_screen_items[2] = createListItem("Folder3", phraser_Icon_Folder);
  startup_screen_items[3] = createListItem("Phrase1", phraser_Icon_Ledger);
  startup_screen_items[4] = createListItem("Phrase2", phraser_Icon_Ledger);
  startup_screen_items[5] = createListItem("Phrase3", phraser_Icon_Ledger);

  initList(startup_screen_items, startup_screen_item_count);
  freeItemList(startup_screen_items, startup_screen_item_count);
}

void phraserDbUiLoop(Thumby* thumby) {
  int chosenItem = listLoop(thumby);
}
