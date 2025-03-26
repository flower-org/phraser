#include "DbCreate.h"
#include "TextAreaDialog.h"
#include "UiCommon.h"
#include "BlockStore.h"
#include "PhraserUtils.h"
#include "SerialUtils.h"
#include "ScreenKeyboard.h"
#include "BlockCache.h"
#include <sha256.h>

const uint16_t BANK_BLOCK_COUNT = 128;

// ---------- New DB ---------- 

void bankScreenInit() {
  int count = 5;
  ListItem** startup_screen_items = (ListItem**)malloc(count * sizeof(ListItem*));
  startup_screen_items[0] = createListItem("ALL (1-3)", phraser_Icon_Ledger);
  startup_screen_items[1] = createListItem("BANK 1", phraser_Icon_Ledger);
  startup_screen_items[2] = createListItem("BANK 2", phraser_Icon_Ledger);
  startup_screen_items[3] = createListItem("BANK 3", phraser_Icon_Ledger);
  startup_screen_items[4] = createListItem("Cancel", phraser_Icon_X);

  initList(startup_screen_items, count);
  freeItemList(startup_screen_items, count);
}

char* new_password = NULL;
char* random_str = NULL;

int create_new_db_phase = 0;
int bank = -1;
int current_bank = 0;
int current_bank_cursor = 0;
int db_block_count = BANK_BLOCK_COUNT;
int init_block_count = 0;
int init_pbkdf2_iterations = 0;

void createNewDbInit() {
  if (new_password != NULL) {
    free(new_password);
    new_password = NULL;
  }
  if (random_str != NULL) {
    free(random_str);
    random_str = NULL;
  }
  
  //create_new_db_phase = 52;
  create_new_db_phase = 0;

  bank = -1;
  current_bank = 0;
  current_bank_cursor = 0;
  db_block_count = BANK_BLOCK_COUNT;
  init_block_count = 0;
  init_pbkdf2_iterations = 0;

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
      current_bank = bank == 0 ? 1 : bank;
      current_bank_cursor = 0;
      // Superficial randomization at first pass
      randomSeed(analogRead(0));

      create_new_db_phase = 2;
    } else if (chosenItem == 4) {
      // Cancel
      switchToStartupScreen();
    }
  } else if (create_new_db_phase == 2 || create_new_db_phase == 9) {
    // 2. Erase Banks
    // 9. Randomize Banks
    char text[50];
    if (create_new_db_phase == 2) {
      sprintf(text, "Cleaning up \nBank: %d\nBlock: %d", current_bank, current_bank_cursor);
    } else if (create_new_db_phase == 9) {
      sprintf(text, "Randomizing \nBank: %d\nBlock: %d", current_bank, current_bank_cursor);
    }
    initTextAreaDialog(text, strlen(text), TEXT_AREA);

    textAreaLoop(thumby);

    if (current_bank_cursor >= db_block_count) {
      boolean randomization_done = false;
      if (bank == 0) {
        if (current_bank < 3) {
          current_bank++;
          current_bank_cursor = 0;
        } else {
          randomization_done = true;
        }
      } else {
        randomization_done = true;
      }

      if (randomization_done) {
        if (create_new_db_phase == 2) {
          create_new_db_phase = 31;
          char* text = "Enter password";
          initTextAreaDialog(text, strlen(text), DLG_OK);
        } else if (create_new_db_phase == 9) {
          create_new_db_phase = 10;
          char* text = "Forming DB blocks";
          initTextAreaDialog(text, strlen(text), TEXT_AREA);
        }
      }
    }

    // Fill the buffer with random data
    uint8_t buffer[FLASH_SECTOR_SIZE];
    for (int i = 0; i < FLASH_SECTOR_SIZE; i++) {
      buffer[i] = random(0, 256);
    }

    //Save random data to block
    writeDbBlockToFlash(bank, current_bank_cursor, buffer);
    current_bank_cursor++;
  } else if (create_new_db_phase == 31) {
    // 3.1. Enter password
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      initOnScreenKeyboard(false, true);
      create_new_db_phase = 32;
    }
  } else if (create_new_db_phase == 32) {
    // 3.2. Enter password
    char* new_password_cnd = keyboardLoop(thumby);
    if (new_password_cnd != NULL) {
      new_password = copyString(new_password_cnd, strlen(new_password_cnd));
      create_new_db_phase = 41;
      char* text = "Confirm password";
      initTextAreaDialog(text, strlen(text), DLG_OK);
    }
  } else if (create_new_db_phase == 41) {
    // 4.1 Confirm password
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      initOnScreenKeyboard(false, true);
      create_new_db_phase = 42;
    }
  } else if (create_new_db_phase == 42) {
    // 4.2 Confirm password
    char* new_password_cnd = keyboardLoop(thumby);
    if (new_password_cnd != NULL) {
      if (strcmp(new_password, new_password_cnd) != 0) {
        char* text = "Passwords don't match, try again";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        create_new_db_phase = 31;
      } else {
        char* text = "Enter random string";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        create_new_db_phase = 51;
      }
    }
  } else if (create_new_db_phase == 51) {
    // 5.1 Enter random string
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      initOnScreenKeyboard(false, false);
      create_new_db_phase = 52;
    }
  } else if (create_new_db_phase == 52) {
    // 5.2 Enter random string
    char* new_random_str = keyboardLoop(thumby);
    if (new_random_str != NULL) {
      random_str = copyString(new_random_str, strlen(new_random_str));
      char* text = "Adjust block\ncount if\ndesired";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      create_new_db_phase = 61;
    }
  } else if (create_new_db_phase == 61) {
    // 6.1 Adjust block count
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      char default_count[20];
      itoa(BANK_BLOCK_COUNT, default_count, 10);
      initOnScreenKeyboard(default_count, strlen(default_count), false, false, true, 10, 2);//"Enter" key selected
      create_new_db_phase = 62;
    }
  } else if (create_new_db_phase == 62) {
    // 6.2 Adjust block count
    char* new_block_count = keyboardLoop(thumby);
    if (new_block_count != NULL) {
      init_block_count = atoi(new_block_count);

      bool issue_found = false;
      if (init_block_count < 32) {
        char* text = "Block count can't be < 32.";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        create_new_db_phase = 61;
        issue_found = true;
      } else if (bank == 2) {
        if (init_block_count > 256) {
          char* text = "Block count can't be > 256 for BANK2.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          create_new_db_phase = 61;
          issue_found = true;
        }
      } else if (bank == 3) {
        if (init_block_count > 128) {
          char* text = "Block count can't be > 128 for BANK3.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          create_new_db_phase = 61;
          issue_found = true;
        }
      } else if (init_block_count > 384) {
        char* text = "Block count can't be > 384 for any BANK.";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        create_new_db_phase = 61;
        issue_found = true;
      }

      if (!issue_found) {
        char* text = "Adjust PBKDF2\niterations\nif desired";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        create_new_db_phase = 71;
        }
    }
  } else if (create_new_db_phase == 71) {
    // 7.1 Adjust PBKDF2 iterations
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      char default_count[20];
      itoa(PBKDF_INTERATIONS_COUNT, default_count, 10);
      initOnScreenKeyboard(default_count, strlen(default_count), false, false, true, 10, 2);//"Enter" key selected
      create_new_db_phase = 72;
    }
  } else if (create_new_db_phase == 72) {
    // 7.2 Adjust PBKDF2 iterations
    char* new_pbkdf2_iterations = keyboardLoop(thumby);
    if (new_pbkdf2_iterations != NULL) {
      init_pbkdf2_iterations = atoi(new_pbkdf2_iterations);

      bool issue_found = false;

      if (init_pbkdf2_iterations < 2500) {
        char* text = "Can't accept less than 2500";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        create_new_db_phase = 71;
        issue_found = true;
      }
      if (init_pbkdf2_iterations < 2500) {
        char* text = "Can't accept less than 2500";
        initTextAreaDialog(text, strlen(text), DLG_OK);
        create_new_db_phase = 71;
        issue_found = true;
      }
      if (init_pbkdf2_iterations > 50000) {
        char text[50];
        sprintf(text, "50_000 iters take 15 seconds\nYou're adjusting to\n%d.\nAre you sure?", init_pbkdf2_iterations);
    
        initTextAreaDialog(text, strlen(text), DLG_YES_NO);
        create_new_db_phase = 73;
        issue_found = true;
      }

      if (!issue_found) {
        char* text = "Initialize random";
        initTextAreaDialog(text, strlen(text), TEXT_AREA);
        create_new_db_phase = 8;
      }
    }
  } else if (create_new_db_phase == 73) {
    // 7.3 Adjust PBKDF2 iterations
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_YES) {
      char* text = "Initializing random";
      initTextAreaDialog(text, strlen(text), TEXT_AREA);
      create_new_db_phase = 8;
    } else if (result == DLG_RES_NO) {
      char default_count[20];
      itoa(PBKDF_INTERATIONS_COUNT, default_count, 10);
      initOnScreenKeyboard(default_count, strlen(default_count), false, false, true, 10, 2);//"Enter" key selected
      create_new_db_phase = 72;
    }
  } else if (create_new_db_phase == 8) {
    // 8. Init random
    textAreaLoop(thumby);

    uint32_t micros_int = micros();
    uint32_t analog = analogRead(0);
    char micros_str[50];
    itoa(micros_int, micros_str, 10);
    char analog_str[20];
    itoa(analog, analog_str, 10);

    uint8_t digest[32];
    struct tc_sha256_state_struct s;

    (void)tc_sha256_init(&s);
    tc_sha256_update(&s, (uint8_t*)new_password, strlen(new_password));
    tc_sha256_update(&s, (uint8_t*)random_str, strlen(random_str));
    tc_sha256_update(&s, (uint8_t*)micros_str, strlen(random_str));
    tc_sha256_update(&s, (uint8_t*)analog_str, strlen(random_str));

    (void)tc_sha256_final(digest, &s);
    
    uint8_t* xor16 = xorByteArrays(digest, digest+16, 16);
    uint8_t* xor8 = xorByteArrays(xor16, xor16+8, 8);
    uint8_t* xor4 = xorByteArrays(xor8, xor8+4, 4);
  
    uint32_t seed = bytesToUInt32(xor4);

    free(xor16);
    free(xor8);
    free(xor4);

    randomSeed(seed);

    current_bank = bank == 0 ? 1 : bank;
    current_bank_cursor = 0;

    create_new_db_phase = 9;//Randomize blocks 2
  } else if (create_new_db_phase == 10) {
    // 9. Form and fill DB blocks
    textAreaLoop(thumby);

    int block_numbers[4];
    generateUniqueNumbers(init_block_count, 4, block_numbers);

    char text[50];
    sprintf(text, "%d; %d;\n%d; %d", block_numbers[0], block_numbers[1], block_numbers[2], block_numbers[3]);
    initTextAreaDialog(text, strlen(text), DLG_OK);

    create_new_db_phase = 11;
  } else if (create_new_db_phase == 11) {
    // 11. Done
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      create_new_db_phase = 11;
    }
  } else if (create_new_db_phase == 11) {
    // 10. Final
    drawTurnOffMessage(thumby);
  }
}
