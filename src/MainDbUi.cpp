#include "MainDbUi.h"

#include "TextAreaDialog.h"
#include "ScreenList.h"
#include "ScreenKeyboard.h"
#include "BlockCache.h"
#include "BlockDAO.h"
#include "SerialUtils.h"

enum MainUiPhase {
  // Menus
  FOLDER_BROWSER,
  FOLDER_MENU,
  PHRASE,
  PHRASE_MENU,
  PHRASE_HISTORY,
  PHRASE_HISTORY_MENU,
  PHRASE_HISTORY_ENTRY,
  PHRASE_HISTORY_ENTRY_MENU,

  // Word Actions
  VIEW_WORD,

  EDIT_WORD_YES_NO,
  ENTER_NEW_WORD,
  EDIT_WORD,

  // BlockDAO Dialogs
  FOLDER_OPERATION_ERROR_REPORT,

  CREATE_NEW_FOLDER_YES_NO,
  CREATE_NEW_FOLDER,
  
  RENAME_FOLDER_YES_NO,
  ENTER_RENAME_FOLDER_NAME,
  RENAME_FOLDER,
  
  DELETE_FOLDER_YES_NO,
  
  CREATE_NEW_PHRASE_YES_NO,
  ENTER_NEW_PHRASE_NAME,
  SELECT_NEW_PHRASE_TEMPLATE_OK,
  SELECT_NEW_PHRASE_TEMPLATE,
  CREATE_NEW_PHRASE,
  
  RENAME_PHRASE_YES_NO,
  ENTER_RENAME_PHRASE_NAME,
  RENAME_PHRASE,
  
  CHANGE_PHRASE_FOLDER_YES_NO,
  SELECT_NEW_FOLDER,
  CHANGE_PHRASE_FOLDER,
  SWITCH_FOLDER_YES_NO,
  
  DELETE_PHRASE_YES_NO,
  DELETE_PHRASE
};

MainUiPhase main_ui_phase = FOLDER_BROWSER;

struct FolderPhraseId {
  int folder_id;
  int phrase_id;
};

arraylist* folder_content;// arraylist<FolderPhraseId*>

// Folder Browser
int folder_browser_folder_id;
int selected_folder_id;
int selected_phrase_id;

void initCurrentFolderScreenList(int select_folder_id, int select_phrase_id) {
  serialDebugPrintf("initCurrentFolderScreenList\r\n");
  int selection = 0;
  if (folder_content != NULL) {
    for (int i = 0; i < arraylist_size(folder_content); i++) {
      free(arraylist_get(folder_content, i));
    }
    arraylist_destroy(folder_content);
    folder_content = NULL;
  }

  serialDebugPrintf("getFolderContent %d\r\n", folder_browser_folder_id);
  folder_content = arraylist_create();
  arraylist* current_list_content = getFolderContent(folder_browser_folder_id);
  if (current_list_content != NULL) {
    int current_folder_content_size = arraylist_size(current_list_content);
    int screen_item_cursor = 0;
    int screen_item_count = current_folder_content_size;
    if (folder_browser_folder_id > 0) { screen_item_count++; }
    
    ListItem** screen_items = (ListItem**)malloc(screen_item_count * sizeof(ListItem*));

    if (folder_browser_folder_id > 0) { 
      screen_items[screen_item_cursor++] = createListItem("..", phraser_Icon_ToParentFolder);
    }

    for (int i = 0; i < arraylist_size(current_list_content); i++) {
      FolderPhraseId* folder_phrase_id = (FolderPhraseId*)malloc(sizeof(FolderPhraseId));
      folder_phrase_id->folder_id = -1;
      folder_phrase_id->phrase_id = -1;

      FolderOrPhrase* folderOrPhrase = (FolderOrPhrase*)arraylist_get(current_list_content, i);
      if (folderOrPhrase->folder != NULL) {
        folder_phrase_id->folder_id = folderOrPhrase->folder->folderId;
        if (select_folder_id == folderOrPhrase->folder->folderId) {
          selection = screen_item_cursor;
        }
        screen_items[screen_item_cursor++] = createListItem(folderOrPhrase->folder->folderName, phraser_Icon_Folder);
      } else {
        folder_phrase_id->phrase_id = folderOrPhrase->phrase->phraseBlockId;
        if (select_phrase_id == folderOrPhrase->phrase->phraseBlockId) {
          selection = screen_item_cursor;
        }
        screen_items[screen_item_cursor++] = createListItem(folderOrPhrase->phrase->name, phraser_Icon_Ledger);
      }

      arraylist_add(folder_content, folder_phrase_id);
    }

    serialDebugPrintf("Selection %d", selection);

    initList(screen_items, screen_item_count, selection);
    freeItemList(screen_items, screen_item_count);
  } else {
    int screen_item_count = 0;
    if (folder_browser_folder_id > 0) { screen_item_count++; }
    ListItem** screen_items = (ListItem**)malloc(screen_item_count * sizeof(ListItem*));
    if (folder_browser_folder_id > 0) { 
      screen_items[0] = createListItem("..", phraser_Icon_ToParentFolder);
    }
    initList(screen_items, screen_item_count);
    freeItemList(screen_items, screen_item_count);
  }
}

void initFolder(int folder_id, int select_folder_id, int select_phrase_id) {
  Folder* next_folder = getFolder(folder_id);
  if (next_folder == NULL) {
    next_folder = getFolder(0);//root folder
  }

  if (next_folder != NULL) {
    folder_browser_folder_id = next_folder->folderId;
    initCurrentFolderScreenList(select_folder_id, select_phrase_id);
  }
}

void phraserDbUiInit() {
  initFolder(0, -1, -1);
}

void initPhrase(int phrase_block_id) {
  // TODO: implement
}

// ----------------------------------------------------------------------------

FolderPhraseId* getFolderBrowserSelection(int chosen_item) {
  int folder_index = chosen_item;
  if (folder_browser_folder_id > 0) {
    if (chosen_item == 0) {
      return NULL;
    } else {
      folder_index--;
    }
  }
  return (FolderPhraseId*)arraylist_get(folder_content, folder_index);
}

void folderBrowserAction(int chosen_item) {
  FolderPhraseId* new_selected_folder = getFolderBrowserSelection(chosen_item);
  if (new_selected_folder != NULL) {
    if (new_selected_folder->folder_id != -1) {
      initFolder(new_selected_folder->folder_id, -1, -1);
    } else {
      initPhrase(new_selected_folder->phrase_id);
    }
  } else {
    // Up one level
    int select_folder_id = folder_browser_folder_id;
    Folder* select_folder = getFolder(select_folder_id);
    // Choose 
    initFolder(select_folder->parentFolderId, select_folder_id, -1);
  }
}

const int FOLDER_MENU_NEW_FOLDER = 1;
const int FOLDER_MENU_RENAME_FOLDER = 2;
const int FOLDER_MENU_DELETE_FOLDER = 3;
const int FOLDER_MENU_NEW_PHRASE = 4;
const int FOLDER_MENU_DELETE_PHRASE = 5;
const int FOLDER_MENU_RENAME_PHRASE = 6;
const int FOLDER_MENU_CHANGE_PHRASE_FOLDER = 7;
void folderBrowserMenuAction(int chosen_item, int code) {
  serialDebugPrintf("%d %d \r\n", chosen_item, code);
  if (FOLDER_MENU_NEW_FOLDER == code) {
    char* text = "Create new folder?";
    initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    main_ui_phase = CREATE_NEW_FOLDER_YES_NO;
  } else if (FOLDER_MENU_RENAME_FOLDER == code) {
    char text[350];
    Folder* selected_folder = getFolder(selected_folder_id);
    sprintf(text, "Rename folder `%s`?", selected_folder->folderName);
    initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    main_ui_phase = RENAME_FOLDER_YES_NO;
  } else if (FOLDER_MENU_DELETE_FOLDER == code) {
    char text[350];
    Folder* selected_folder = getFolder(selected_folder_id);
    sprintf(text, "Delete folder `%s`?", selected_folder->folderName);
    initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    main_ui_phase = DELETE_FOLDER_YES_NO;
  } else if (FOLDER_MENU_NEW_PHRASE == code) {
    //CREATE_NEW_PHRASE_YES_NO,
    //ENTER_NEW_PHRASE_NAME,
    //SELECT_NEW_PHRASE_TEMPLATE_OK,
    //SELECT_NEW_PHRASE_TEMPLATE,
    //CREATE_NEW_PHRASE
  } else if (FOLDER_MENU_DELETE_PHRASE == code) {
    //DELETE_PHRASE_YES_NO,
    //DELETE_PHRASE
  } else if (FOLDER_MENU_RENAME_PHRASE == code) {
    //RENAME_PHRASE_YES_NO,
    //ENTER_RENAME_PHRASE_NAME,
    //RENAME_PHRASE/
  } else if (FOLDER_MENU_CHANGE_PHRASE_FOLDER == code) {
    //CHANGE_PHRASE_FOLDER_YES_NO,
    //SELECT_NEW_FOLDER,
    //CHANGE_PHRASE_FOLDER
    //SWITCH_FOLDER_YES_NO
  }
}

bool isMenuPhase() {
  switch (main_ui_phase) {
    case FOLDER_BROWSER:
    case FOLDER_MENU:
    case PHRASE:
    case PHRASE_MENU:
    case PHRASE_HISTORY:
    case PHRASE_HISTORY_MENU:
    case PHRASE_HISTORY_ENTRY:
    case PHRASE_HISTORY_ENTRY_MENU: return true;
  }
  return false;
}

void runMainUiPhaseAction(int chosen_item, int code) {
  switch (main_ui_phase) {
    case FOLDER_BROWSER: folderBrowserAction(chosen_item); break;
    case FOLDER_MENU: folderBrowserMenuAction(chosen_item, code); break;
    case PHRASE:
    case PHRASE_MENU:
    case PHRASE_HISTORY:
    case PHRASE_HISTORY_MENU:
    case PHRASE_HISTORY_ENTRY:
    case PHRASE_HISTORY_ENTRY_MENU:
    default: return;
  }
}

void init_folder_browser(int chosen_item) {
  main_ui_phase = FOLDER_BROWSER; 
  initCurrentFolderScreenList(selected_folder_id, selected_phrase_id);
}

void init_folder_menu(int chosen_item) {
  main_ui_phase = FOLDER_MENU;

  FolderPhraseId* selection = getFolderBrowserSelection(chosen_item);
  if (selection != NULL) {
    selected_folder_id = selection->folder_id;
    selected_phrase_id = selection->phrase_id;
  } else {
    selected_folder_id = -1;
    selected_phrase_id = -1;
  }

  int menu_items_count = 2;//new folder, new phrase
  if (selected_folder_id != -1) {
    menu_items_count += 2;
  } 
  if (selected_phrase_id != -1) {
    menu_items_count += 1;
  }

  int menu_item_cursor = 0;
  ListItem** screen_items = (ListItem**)malloc(menu_items_count * sizeof(ListItem*));

  char text[350];
  if (selected_folder_id != -1) {
    Folder* selected_folder = getFolder(selected_folder_id);
    sprintf(text, "Rename folder `%s`", selected_folder->folderName);
    screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Check, FOLDER_MENU_RENAME_FOLDER);
    sprintf(text, "Delete folder `%s`", selected_folder->folderName);
    screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_X, FOLDER_MENU_DELETE_FOLDER);
  }
  if (selected_phrase_id != -1) {
    PhraseFolderAndName* selected_phrase = getPhrase(selected_folder_id);
    sprintf(text, "Delete phrase `%s`", selected_phrase->name);
    screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_X, FOLDER_MENU_DELETE_PHRASE);
    menu_items_count += 1;
  }

  Folder* folder = getFolder(folder_browser_folder_id);
  sprintf(text, "New folder under `%s`", folder->folderName);
  screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Plus, FOLDER_MENU_NEW_FOLDER);
  sprintf(text, "New phrase under `%s`", folder->folderName);
  screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Plus, FOLDER_MENU_NEW_PHRASE);
  initList(screen_items, menu_items_count);
  freeItemList(screen_items, menu_items_count);
}

void menuSwitch(int chosen_item) {
  // Switch phases that support menu screens to correspoding menu screen and back
  switch (main_ui_phase) {
    case FOLDER_BROWSER: 
        init_folder_menu(chosen_item);
        break;
    case FOLDER_MENU: 
        init_folder_browser(chosen_item);
        break;
    case PHRASE: 
        main_ui_phase = PHRASE_MENU; 
        break;
    case PHRASE_MENU: 
        main_ui_phase = PHRASE; 
        break;
    case PHRASE_HISTORY: 
        main_ui_phase = PHRASE_HISTORY_MENU; 
        break;
    case PHRASE_HISTORY_MENU: 
        main_ui_phase = PHRASE_HISTORY; 
        break;
    case PHRASE_HISTORY_ENTRY: 
        main_ui_phase = PHRASE_HISTORY_ENTRY_MENU; 
        break;
    case PHRASE_HISTORY_ENTRY_MENU: 
        main_ui_phase = PHRASE_HISTORY_ENTRY; 
        break;
  }
}

void dialogActionsLoop(Thumby* thumby) {
  switch (main_ui_phase) {
    case FOLDER_OPERATION_ERROR_REPORT: {
      DialogResult result = textAreaLoop(thumby);
      if (result == DLG_RES_OK) {
        initFolder(folder_browser_folder_id, selected_folder_id, selected_phrase_id);
        main_ui_phase = FOLDER_BROWSER;
      }
    }
    break;

    case CREATE_NEW_FOLDER_YES_NO: { 
        DialogResult result = textAreaLoop(thumby);
        if (result == DLG_RES_YES) {
          main_ui_phase = CREATE_NEW_FOLDER;
          initOnScreenKeyboard(false, false);
        } else if (result == DLG_RES_NO) {
          main_ui_phase = FOLDER_MENU;
        }
      }
      break;
    case CREATE_NEW_FOLDER: {
        char* new_folder_name = keyboardLoop(thumby);
        if (new_folder_name != NULL) {
          uint16_t new_folder_id;
          UpdateResponse new_folder_response = addNewFolder(new_folder_name, folder_browser_folder_id, &new_folder_id);
          if (new_folder_response == OK) {
            initFolder(folder_browser_folder_id, new_folder_id, selected_phrase_id);
            main_ui_phase = FOLDER_BROWSER;
          } else {
            if (new_folder_response == ERROR) {
              char* text = "Create new folder ERROR.";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            } else if (new_folder_response == DB_FULL) {
              char* text = "Database full, can't create";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            } else if (new_folder_response == BLOCK_SIZE_EXCEEDED) {
              char* text = "Block size exceeded - too many folders. Can't create";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            }
            main_ui_phase = FOLDER_OPERATION_ERROR_REPORT;
          }
        }
      }
      break;

    case DELETE_FOLDER_YES_NO: {
        DialogResult result = textAreaLoop(thumby);
        if (result == DLG_RES_YES) {
          UpdateResponse delete_folder_response = deleteFolder(selected_folder_id);
          if (delete_folder_response == OK) {
            initFolder(folder_browser_folder_id, selected_folder_id, selected_phrase_id);
            main_ui_phase = FOLDER_BROWSER;
          } else {
            if (delete_folder_response == ERROR) {
              char* text = "Delete folder ERROR.";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            } else if (delete_folder_response == DB_FULL) {
              char* text = "Database full (probably something is really wrong since we're trying to delete)";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            } else if (delete_folder_response == BLOCK_SIZE_EXCEEDED) {
              char* text = "Block size exceeded - too many folders. Can't delete? doesn't make sense.";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            }
            main_ui_phase = FOLDER_OPERATION_ERROR_REPORT;
          }
        } else if (result == DLG_RES_NO) {
          main_ui_phase = FOLDER_MENU;
        }
      }
      break;

      case RENAME_FOLDER_YES_NO: { 
        DialogResult result = textAreaLoop(thumby);
        if (result == DLG_RES_YES) {
          main_ui_phase = RENAME_FOLDER;
          Folder* selected_folder = getFolder(selected_folder_id);
          initOnScreenKeyboard(selected_folder->folderName, strlen(selected_folder->folderName), false, false, false);
        } else if (result == DLG_RES_NO) {
          main_ui_phase = FOLDER_MENU;
        }
      }
      break;
      case RENAME_FOLDER: {
        char* new_folder_name = keyboardLoop(thumby);
        if (new_folder_name != NULL) {
          UpdateResponse new_folder_response = renameFolder(selected_folder_id, new_folder_name);
          if (new_folder_response == OK) {
            initFolder(folder_browser_folder_id, selected_folder_id, selected_phrase_id);
            main_ui_phase = FOLDER_BROWSER;
          } else {
            if (new_folder_response == ERROR) {
              char* text = "Rename folder ERROR.";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            } else if (new_folder_response == DB_FULL) {
              char* text = "Database full (probably something is really wrong since we're trying to rename)";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            } else if (new_folder_response == BLOCK_SIZE_EXCEEDED) {
              char* text = "Block size exceeded - too many folders/name too long. Can't rename";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            }
            main_ui_phase = FOLDER_OPERATION_ERROR_REPORT;
          }
        }
      }
      break;
  }

  // BlockDAO Dialogs

  // TODO: CHANGE PARENT FOLDER for Folder?

  // CREATE_NEW_PHRASE_YES_NO,
  // ENTER_NEW_PHRASE_NAME,
  // SELECT_NEW_PHRASE_TEMPLATE_OK,
  // SELECT_NEW_PHRASE_TEMPLATE,
  // CREATE_NEW_PHRASE,
  
  // RENAME_PHRASE_YES_NO,
  // ENTER_RENAME_PHRASE_NAME,
  // RENAME_PHRASE,
  
  // CHANGE_PHRASE_FOLDER_YES_NO,
  // SELECT_NEW_FOLDER,
  // CHANGE_PHRASE_FOLDER,
  // SWITCH_FOLDER_YES_NO,
  
  // DELETE_PHRASE_YES_NO,
  // DELETE_PHRASE
}

void phraserDbUiLoop(Thumby* thumby) {
  if (isMenuPhase()) {
    // Menu phase, all menus are ScreenList-based
    SelectionAndCode chosen = listLoopWithCode(thumby, true);
    if (chosen.selection != -1) {
      if (getSelectButton() == SELECT_BUTTON_A) {
        runMainUiPhaseAction(chosen.selection, chosen.code);
      } else { //SELECT_BUTTON_B
        menuSwitch(chosen.selection);
      }
    }
  } else {
    // Dialog phase
    dialogActionsLoop(thumby);
  }
}
