#include "DbCreate.h"
#include "TextAreaDialog.h"
#include "UiCommon.h"
#include "BlockStore.h"
#include "PhraserUtils.h"
#include "SerialUtils.h"

// ---------- New DB ---------- 

void bankScreenInit() {
  int count = 5;
  ListItem** startup_screen_items = (ListItem**)malloc(count * sizeof(ListItem*));
  startup_screen_items[0] = createListItem("ALL BANKS", phraser_Icon_Ledger);
  startup_screen_items[1] = createListItem("BANK 1", phraser_Icon_Ledger);
  startup_screen_items[2] = createListItem("BANK 2", phraser_Icon_Ledger);
  startup_screen_items[3] = createListItem("BANK 3", phraser_Icon_Ledger);
  startup_screen_items[4] = createListItem("Cancel", phraser_Icon_X);

  initList(startup_screen_items, count);
  freeItemList(startup_screen_items, count);
}

bool bank_block_counter = 0;
int create_new_db_phase = 0;
int bank = -1;
void createNewDbInit() {
  create_new_db_phase = 0;
  bank = -1;
  char* text = "Create DB?\nExisting data\nwill be lost!";
  initTextAreaDialog(text, strlen(text), DLG_YES_NO);
}

void createNewDbLoop(Thumby* thumby) {
  if (create_new_db_phase == 0) {
    // Phase 0. Get user confirmation
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_YES) {
      bankScreenInit();
      create_new_db_phase = 1;
    } else if (result == DLG_RES_NO) {
      switchToStartupScreen();
    }
  } else if (create_new_db_phase == 1) {
    // Phase 1. Choose Bank
    int chosenItem = listLoop(thumby);
    if (chosenItem >= 0 && chosenItem <= 3) {
      bank = chosenItem;
      create_new_db_phase = 2;
    } else if (chosenItem == 4) {
      // Cancel
      switchToStartupScreen();
    }
  } else if (create_new_db_phase == 2 || create_new_db_phase == 7) {
    // 2. Nullify Banks
    // 7. Randomize Banks
    uint8_t buffer[FLASH_SECTOR_SIZE];
    if (create_new_db_phase == 2) {
      randomSeed(analogRead(0));
    } else if (create_new_db_phase == 7) {
      // TODO: randomize truly based on 
      // password, entered random number, micros(), analogRead(0)
    }
    // Fill the buffer with random data
    for (int i = 0; i < FLASH_SECTOR_SIZE; i++) {
      buffer[i] = random(0, 256);
    }
  } else if (create_new_db_phase == 3) {
    // 3. Enter password
  } else if (create_new_db_phase == 4) {
    // 4. Confirm password
  } else if (create_new_db_phase == 5) {
    // 5. Enter random number
  } else if (create_new_db_phase == 6) {
    // 6. Init random
  } else if (create_new_db_phase == 8) {
    // 8. Form and fill DB blocks
  } else if (create_new_db_phase == 9) {
    // 9. Done
  } else if (create_new_db_phase == 2222) {
    drawTurnOffMessage(thumby);
  }
}
