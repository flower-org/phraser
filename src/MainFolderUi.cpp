#include "MainFolderUi.h"

#include "TextAreaDialog.h"
#include "ScreenList.h"
#include "ScreenKeyboard.h"
#include "BlockCache.h"
#include "BlockDAO.h"
#include "SerialUtils.h"

enum MainFolderUiPhase {
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
  FOLDER_MENU_OPERATION_ERROR_REPORT,

  CREATE_NEW_FOLDER_YES_NO,
  CREATE_NEW_FOLDER,
  
  RENAME_FOLDER_YES_NO,
  ENTER_RENAME_FOLDER_NAME,
  RENAME_FOLDER,
  
  DELETE_FOLDER_YES_NO,
  
  CREATE_NEW_PHRASE_YES_NO,
  ENTER_NEW_PHRASE_NAME,
  CREATE_NEW_PHRASE,
  
  RENAME_PHRASE_YES_NO,
  RENAME_PHRASE,
  
  MOVE_PHRASE_YES_NO,
  MOVE_PHRASE_SELECT_PARENT_FOLDER,
  MOVE_PHRASE,
  
  MOVE_FOLDER_YES_NO,
  MOVE_FOLDER_SELECT_PARENT_FOLDER,
  MOVE_FOLDER,
  
  DELETE_PHRASE_YES_NO,
  DELETE_PHRASE
};

MainFolderUiPhase main_folder_ui_phase = FOLDER_BROWSER;

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

  serialDebugPrintf("getFolderContent %d select_folder_id %d select_phrase_id %d\r\n", folder_browser_folder_id, select_folder_id, select_phrase_id);
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

    serialDebugPrintf("Selection %d\r\n", selection);

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

void phraserFolderUiInit() {
  initFolder(0, -1, -1);
}

FullPhrase* current_phrase = NULL;
void initPhrase(int phrase_block_id) {
  // don't reuse loaded phrase, aways free and re-load (in case it's updated from folder menus)
  if (current_phrase != NULL) {
    releaseFullPhrase(current_phrase);
    current_phrase = NULL;
  }

  current_phrase = getFullPhrase(phrase_block_id);
  if (current_phrase == NULL) {
    char text[350];
    sprintf(text, "Failed to load phrase `%d`?", phrase_block_id);
    initTextAreaDialog(text, strlen(text), DLG_OK);
    main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
  } else {
    // TODO: init main phrase view
    char text[350];
    sprintf(text, "Successfully loaded phrase `%d` %s?", current_phrase->phrase_block_id, current_phrase->phrase_name);
    initTextAreaDialog(text, strlen(text), DLG_OK);
    main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
  }
}

// ----------------------------------------------------------------------------

void mainPhraseViewAction(int chosen_item) {
  // TODO: implement
}

void mainPhraseViewMenuAction(int chosen_item, int code) {
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
    selected_folder_id = new_selected_folder->folder_id;
    selected_phrase_id = new_selected_folder->phrase_id;
    if (new_selected_folder->folder_id != -1) {
      initFolder(new_selected_folder->folder_id, -1, -1);
    } else {
      initPhrase(new_selected_folder->phrase_id);
    }
  } else {
    selected_folder_id = -1;
    selected_phrase_id = -1;
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
const int FOLDER_MENU_MOVE_FOLDER = 4;
const int FOLDER_MENU_NEW_PHRASE = 5;
const int FOLDER_MENU_DELETE_PHRASE = 6;
const int FOLDER_MENU_RENAME_PHRASE = 7;
const int FOLDER_MENU_MOVE_PHRASE = 8;
void folderBrowserMenuAction(int chosen_item, int code) {
  serialDebugPrintf("%d %d \r\n", chosen_item, code);
  if (FOLDER_MENU_NEW_FOLDER == code) {
    char* text = "Create new folder?";
    initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    main_folder_ui_phase = CREATE_NEW_FOLDER_YES_NO;
  } else if (FOLDER_MENU_RENAME_FOLDER == code) {
    char text[350];
    Folder* selected_folder = getFolder(selected_folder_id);
    sprintf(text, "Rename folder `%s`?", selected_folder->folderName);
    initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    main_folder_ui_phase = RENAME_FOLDER_YES_NO;
  } else if (FOLDER_MENU_DELETE_FOLDER == code) {
    char text[350];
    Folder* selected_folder = getFolder(selected_folder_id);
    sprintf(text, "Delete folder `%s`?", selected_folder->folderName);
    initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    main_folder_ui_phase = DELETE_FOLDER_YES_NO;
  } else if (FOLDER_MENU_NEW_PHRASE == code) {
    if (last_block_left()) {
      char* text = "Database full, can't create";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
    } else {
      char* text = "Create new phrase?";
      initTextAreaDialog(text, strlen(text), DLG_YES_NO);
      main_folder_ui_phase = CREATE_NEW_PHRASE_YES_NO;
    }
  } else if (FOLDER_MENU_DELETE_PHRASE == code) {
    char text[350];
    PhraseFolderAndName* selected_phrase = getPhrase(selected_phrase_id);
    sprintf(text, "Delete phrase `%s`?", selected_phrase->name);
    initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    main_folder_ui_phase = DELETE_PHRASE_YES_NO;
  } else if (FOLDER_MENU_RENAME_PHRASE == code) {
    char text[350];
    PhraseFolderAndName* selected_phrase = getPhrase(selected_phrase_id);
    sprintf(text, "Rename phrase `%s`?", selected_phrase->name);
    initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    main_folder_ui_phase = RENAME_PHRASE_YES_NO;
  } else if (FOLDER_MENU_MOVE_FOLDER == code) {
    char text[350];
    Folder* selected_folder = getFolder(selected_folder_id);
    sprintf(text, "Move folder `%s` to a different folder?", selected_folder->folderName);
    initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    main_folder_ui_phase = MOVE_FOLDER_YES_NO;
  } else if (FOLDER_MENU_MOVE_PHRASE == code) {
    char text[350];
    PhraseFolderAndName* selected_phrase = getPhrase(selected_phrase_id);
    sprintf(text, "Move phrase `%s` to a different folder?", selected_phrase->name);
    initTextAreaDialog(text, strlen(text), DLG_YES_NO);
    main_folder_ui_phase = MOVE_PHRASE_YES_NO;
  }
}

bool isMenuPhase() {
  switch (main_folder_ui_phase) {
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

void runMainFolderUiPhaseAction(int chosen_item, int code) {
  switch (main_folder_ui_phase) {
    case FOLDER_BROWSER: folderBrowserAction(chosen_item); break;
    case FOLDER_MENU: folderBrowserMenuAction(chosen_item, code); break;
    case PHRASE: mainPhraseViewAction(chosen_item); break;
    case PHRASE_MENU: mainPhraseViewMenuAction(chosen_item, code); break;
    case PHRASE_HISTORY:
    case PHRASE_HISTORY_MENU:
    case PHRASE_HISTORY_ENTRY:
    case PHRASE_HISTORY_ENTRY_MENU:
    default: return;
  }
}

void init_folder_browser(int chosen_item) {
  main_folder_ui_phase = FOLDER_BROWSER; 
  initCurrentFolderScreenList(selected_folder_id, selected_phrase_id);
}

void init_folder_menu(int chosen_item) {
  main_folder_ui_phase = FOLDER_MENU;

  FolderPhraseId* selection = getFolderBrowserSelection(chosen_item);
  if (selection != NULL) {
    selected_folder_id = selection->folder_id;
    selected_phrase_id = selection->phrase_id;
  } else {
    selected_folder_id = -1;
    selected_phrase_id = -1;
  }

  int menu_items_count = 2;//new folder, new phrase
  Folder* selected_folder = getFolder(selected_folder_id);
  if (selected_folder != NULL) {
    menu_items_count += 3;
  } 
  PhraseFolderAndName* selected_phrase = getPhrase(selected_phrase_id);
  if (selected_phrase != NULL) {
    menu_items_count += 3;
  }

  int menu_item_cursor = 0;
  ListItem** screen_items = (ListItem**)malloc(menu_items_count * sizeof(ListItem*));

  char text[350];
  if (selected_folder != NULL) {
    sprintf(text, "Rename folder `%s`", selected_folder->folderName);
    screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Check, FOLDER_MENU_RENAME_FOLDER);
    sprintf(text, "Delete folder `%s`", selected_folder->folderName);
    screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_X, FOLDER_MENU_DELETE_FOLDER);
    sprintf(text, "Move folder `%s`", selected_folder->folderName);
    screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Login, FOLDER_MENU_MOVE_FOLDER);
  }
  if (selected_phrase != NULL) {
    sprintf(text, "Rename phrase `%s`", selected_phrase->name);
    screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Check, FOLDER_MENU_RENAME_PHRASE);
    sprintf(text, "Delete phrase `%s`", selected_phrase->name);
    screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_X, FOLDER_MENU_DELETE_PHRASE);
    sprintf(text, "Move phrase `%s`", selected_phrase->name);
    screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Login, FOLDER_MENU_MOVE_PHRASE);
  }

  Folder* folder = getFolder(folder_browser_folder_id);
  sprintf(text, "New phrase under `%s`", folder->folderName);
  screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Plus, FOLDER_MENU_NEW_PHRASE);
  sprintf(text, "New folder under `%s`", folder->folderName);
  screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Plus, FOLDER_MENU_NEW_FOLDER);
  initList(screen_items, menu_items_count);
  freeItemList(screen_items, menu_items_count);
}

void menuSwitch(int chosen_item) {
  // Switch phases that support menu screens to correspoding menu screen and back
  switch (main_folder_ui_phase) {
    case FOLDER_BROWSER: 
        init_folder_menu(chosen_item);
        break;
    case FOLDER_MENU: 
        init_folder_browser(chosen_item);
        break;
    case PHRASE: 
        main_folder_ui_phase = PHRASE_MENU; 
        break;
    case PHRASE_MENU: 
        main_folder_ui_phase = PHRASE; 
        break;
    case PHRASE_HISTORY: 
        main_folder_ui_phase = PHRASE_HISTORY_MENU; 
        break;
    case PHRASE_HISTORY_MENU: 
        main_folder_ui_phase = PHRASE_HISTORY; 
        break;
    case PHRASE_HISTORY_ENTRY: 
        main_folder_ui_phase = PHRASE_HISTORY_ENTRY_MENU; 
        break;
    case PHRASE_HISTORY_ENTRY_MENU: 
        main_folder_ui_phase = PHRASE_HISTORY_ENTRY; 
        break;
  }
}

ListItem** tmp_screen_items;
int tmp_screen_item_cursor;
void build_phrase_template_entries(hashtable *t, uint32_t key, void* value) {
  PhraseTemplate* phrase_template = (PhraseTemplate*)value;
  tmp_screen_items[tmp_screen_item_cursor++] = createListItemWithCode(phrase_template->phraseTemplateName, phraser_Icon_Copy, phrase_template->phraseTemplateId);
}

void build_folder_entries(hashtable *t, uint32_t key, void* value) {
  Folder* folder = (Folder*)value;
  tmp_screen_items[tmp_screen_item_cursor++] = createListItemWithCode(folder->folderName, phraser_Icon_Folder, folder->folderId);
}

void createRepeatedSlashes(int level, char* result) {
  for (int i = 0; i < level; i++) {
      result[i] = '/';
  }
  result[level] = '\0';
}

void fillFolder(Folder* root_folder, arraylist* folder_list, int level, int exclude_folder_id) {
  if (root_folder == NULL) {
    return;
  }

  if (root_folder->folderId == exclude_folder_id) {
    // Can't move a folder into itself or into it's own child folder
    return;
  }

  char slashes[level+1];
  createRepeatedSlashes(level, slashes);

  char text[350];
  sprintf(text, "%s%s", slashes, root_folder->folderName);

  arraylist_add(folder_list, createListItemWithCode(text, phraser_Icon_Folder, root_folder->folderId));

  arraylist* subfolders = getSubFolders(root_folder->folderId);
  if (subfolders != NULL) {
    for (int i = 0; i < arraylist_size(subfolders); i++) {
      uint16_t child_folder_id = (uint32_t)arraylist_get(subfolders, i);
      Folder* child_folder = getFolder(child_folder_id);
      fillFolder(child_folder, folder_list, level+1, exclude_folder_id);
    }
  }
}

void initFoldersList(int exclude_folder_id) {
  arraylist* folder_list = arraylist_create();
  Folder* root_folder = getFolder(0);
  fillFolder(root_folder, folder_list, 0, exclude_folder_id);

  int screen_item_count = arraylist_size(folder_list);
  ListItem** screen_items = (ListItem**)malloc(screen_item_count * sizeof(ListItem*));
  for (int i = 0; i < screen_item_count; i++) {
    screen_items[i] = (ListItem*)arraylist_get(folder_list, i);
  }
  arraylist_destroy(folder_list);

  initList(screen_items, screen_item_count);
  freeItemList(screen_items, screen_item_count);
}

char* ui_new_phrase_name;
uint16_t ui_new_phrase_phrase_template_id;
uint16_t ui_new_phrase_folder_id;
uint16_t ui_move_folder_to_folder_id;
void dialogActionsLoop(Thumby* thumby) {
  switch (main_folder_ui_phase) {
    case FOLDER_MENU_OPERATION_ERROR_REPORT: {
      DialogResult result = textAreaLoop(thumby);
      if (result == DLG_RES_OK) {
        initFolder(folder_browser_folder_id, selected_folder_id, selected_phrase_id);
        main_folder_ui_phase = FOLDER_BROWSER;
      }
    }
    break;

    case CREATE_NEW_FOLDER_YES_NO: { 
        DialogResult result = textAreaLoop(thumby);
        if (result == DLG_RES_YES) {
          main_folder_ui_phase = CREATE_NEW_FOLDER;
          initOnScreenKeyboard(false, false);
        } else if (result == DLG_RES_NO) {
          main_folder_ui_phase = FOLDER_MENU;
        }
      }
      break;
    case CREATE_NEW_FOLDER: {
        char* new_folder_name = keyboardLoop(thumby);
        if (new_folder_name != NULL) {
          if (strlen(new_folder_name) == 0) {
            char* text = "Folder name can't be empty.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
            main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
          } else {
            uint16_t new_folder_id;
            UpdateResponse new_folder_response = addNewFolder(new_folder_name, folder_browser_folder_id, &new_folder_id);
            if (new_folder_response == OK) {
              initFolder(folder_browser_folder_id, new_folder_id, selected_phrase_id);
              main_folder_ui_phase = FOLDER_BROWSER;
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
              main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
            }
          }
        }
      }
      break;

    case DELETE_FOLDER_YES_NO: {
        DialogResult result = textAreaLoop(thumby);
        if (result == DLG_RES_YES) {
          int child_count = getFolderChildCount(selected_folder_id);
          if (child_count > 0) {
            char* text = "Folder not empty, can't delete.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
            main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
          } else {
            UpdateResponse delete_folder_response = deleteFolder(selected_folder_id);
            if (delete_folder_response == OK) {
              initFolder(folder_browser_folder_id, selected_folder_id, selected_phrase_id);
              main_folder_ui_phase = FOLDER_BROWSER;
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
              main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
            }
          }
        } else if (result == DLG_RES_NO) {
          main_folder_ui_phase = FOLDER_MENU;
        }
      }
      break;

      case RENAME_FOLDER_YES_NO: { 
        DialogResult result = textAreaLoop(thumby);
        if (result == DLG_RES_YES) {
          main_folder_ui_phase = RENAME_FOLDER;
          Folder* selected_folder = getFolder(selected_folder_id);
          initOnScreenKeyboard(selected_folder->folderName, strlen(selected_folder->folderName), false, false, false);
        } else if (result == DLG_RES_NO) {
          main_folder_ui_phase = FOLDER_MENU;
        }
      }
      break;
      case RENAME_FOLDER: {
        char* new_folder_name = keyboardLoop(thumby);
        if (new_folder_name != NULL) {
          if (strlen(new_folder_name) == 0) {
            char* text = "Folder name can't be empty.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
            main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
          } else {
            UpdateResponse new_folder_response = renameFolder(selected_folder_id, new_folder_name);
            if (new_folder_response == OK) {
              initFolder(folder_browser_folder_id, selected_folder_id, selected_phrase_id);
              main_folder_ui_phase = FOLDER_BROWSER;
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
              main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
            }
          }
        }
      }
      break;

    case CREATE_NEW_PHRASE_YES_NO: { 
        DialogResult result = textAreaLoop(thumby);
        if (result == DLG_RES_YES) {
          main_folder_ui_phase = ENTER_NEW_PHRASE_NAME;
          initOnScreenKeyboard(false, false);
        } else if (result == DLG_RES_NO) {
          main_folder_ui_phase = FOLDER_MENU;
        }
      }
      break;
    case ENTER_NEW_PHRASE_NAME: {
        char* text = keyboardLoop(thumby);
        if (text != NULL) {
          if (strlen(text) == 0) {
            char* text = "Phrase name can't be empty.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
            main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
          } else {
            ui_new_phrase_name = text;

            //initialize phrase template list
            hashtable* phrase_templates = getPhraseTemplates();

            int screen_item_count = phrase_templates->size;
            ListItem** screen_items = (ListItem**)malloc(screen_item_count * sizeof(ListItem*));
            tmp_screen_items = screen_items;
            tmp_screen_item_cursor = 0;
            hashtable_iterate_entries(phrase_templates, build_phrase_template_entries);
            tmp_screen_items = NULL;
            tmp_screen_item_cursor = 0;

            initList(screen_items, screen_item_count);
            freeItemList(screen_items, screen_item_count);
            main_folder_ui_phase = CREATE_NEW_PHRASE;
          }
        }
      }
      break;
    case CREATE_NEW_PHRASE: {
      SelectionAndCode chosen = listLoopWithCode(thumby, true);
      if (chosen.selection != -1) {
        ui_new_phrase_phrase_template_id = chosen.code;
        ui_new_phrase_folder_id = folder_browser_folder_id;

        // TODO: remove debug output
        serialDebugPrintf("About to create new phrase: new_phrase_name %s, new_phrase_folder_id %d, new_phrase_phrase_template_id %d\r\n", 
          ui_new_phrase_name, ui_new_phrase_folder_id, ui_new_phrase_phrase_template_id);

        uint16_t created_phrase_id;
        UpdateResponse new_phrase_response = addNewPhrase(ui_new_phrase_name, ui_new_phrase_phrase_template_id, ui_new_phrase_folder_id, &created_phrase_id);
        if (new_phrase_response == OK) {
          initFolder(folder_browser_folder_id, -1, created_phrase_id);
          main_folder_ui_phase = FOLDER_BROWSER;
        } else {
          if (new_phrase_response == ERROR) {
            char* text = "Phrase creation ERROR.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          } else if (new_phrase_response == DB_FULL) {
            char* text = "Database full, can't create";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          } else if (new_phrase_response == BLOCK_SIZE_EXCEEDED) {
            char* text = "Block size exceeded.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          }
          main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
        }
      }
    }
    break;

    case DELETE_PHRASE_YES_NO: {
      DialogResult result = textAreaLoop(thumby);
      if (result == DLG_RES_YES) {
        UpdateResponse delete_phrase_response = deletePhrase(selected_phrase_id);
        if (delete_phrase_response == OK) {
          initFolder(folder_browser_folder_id, selected_folder_id, selected_phrase_id);
          main_folder_ui_phase = FOLDER_BROWSER;
        } else {
          if (delete_phrase_response == ERROR) {
            char* text = "Delete phrase ERROR.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          } else if (delete_phrase_response == DB_FULL) {
            char* text = "Database full (probably something is really wrong since we're trying to delete)";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          } else if (delete_phrase_response == BLOCK_SIZE_EXCEEDED) {
            char* text = "Block size exceeded. Can't delete? doesn't make sense.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          }
          main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
        }
      } else if (result == DLG_RES_NO) {
        main_folder_ui_phase = FOLDER_MENU;
      }
    }
    break;

    case MOVE_PHRASE_YES_NO: {
      DialogResult result = textAreaLoop(thumby);
      if (result == DLG_RES_YES) {
        initFoldersList(-1);
        main_folder_ui_phase = MOVE_PHRASE;
      } else if (result == DLG_RES_NO) {
        main_folder_ui_phase = FOLDER_MENU;
      }
    }
    break;
    case MOVE_PHRASE: {
      SelectionAndCode chosen = listLoopWithCode(thumby, true);
      if (chosen.selection != -1) {
        ui_new_phrase_folder_id = chosen.code;

        UpdateResponse new_phrase_response = movePhrase(selected_phrase_id, ui_new_phrase_folder_id);
        if (new_phrase_response == OK) {
          char text[350];
          Folder* moved_to_folder = getFolder(ui_new_phrase_folder_id);
          sprintf(text, "Phrase moved. Switch folder to `%s`?", moved_to_folder->folderName);
          initTextAreaDialog(text, strlen(text), DLG_YES_NO);
          main_folder_ui_phase = MOVE_PHRASE_SELECT_PARENT_FOLDER;
        } else {
          if (new_phrase_response == ERROR) {
            char* text = "Phrase move ERROR.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          } else if (new_phrase_response == DB_FULL) {
            char* text = "Database full (probably something is really wrong since we're trying to move)";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          } else if (new_phrase_response == BLOCK_SIZE_EXCEEDED) {
            char* text = "Block size exceeded. Can't move? doesn't make sense.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          }
          main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
        }
      }
    }
    break;
    case MOVE_PHRASE_SELECT_PARENT_FOLDER: {
        DialogResult result = textAreaLoop(thumby);
        if (result == DLG_RES_NO) {
          initFolder(folder_browser_folder_id, -1, selected_phrase_id);
          main_folder_ui_phase = FOLDER_BROWSER;
        } else if (result == DLG_RES_YES) {
          initFolder(ui_new_phrase_folder_id, -1, selected_phrase_id);
          main_folder_ui_phase = FOLDER_BROWSER;
        }
    }

    case MOVE_FOLDER_YES_NO: {
      DialogResult result = textAreaLoop(thumby);
      if (result == DLG_RES_YES) {
        initFoldersList(selected_folder_id);
        main_folder_ui_phase = MOVE_FOLDER;
      } else if (result == DLG_RES_NO) {
        main_folder_ui_phase = FOLDER_MENU;
      }
    }
    break;
    case MOVE_FOLDER: {
      SelectionAndCode chosen = listLoopWithCode(thumby, true);
      if (chosen.selection != -1) {
        ui_move_folder_to_folder_id = chosen.code;

        UpdateResponse move_folder_response = moveFolder(selected_folder_id, ui_move_folder_to_folder_id);
        if (move_folder_response == OK) {
          char text[350];
          Folder* moved_to_folder = getFolder(ui_move_folder_to_folder_id);
          sprintf(text, "Folder moved. Switch folder to `%s`?", moved_to_folder->folderName);
          initTextAreaDialog(text, strlen(text), DLG_YES_NO);
          main_folder_ui_phase = MOVE_FOLDER_SELECT_PARENT_FOLDER;
        } else {
          if (move_folder_response == ERROR) {
            char* text = "Folder move ERROR.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          } else if (move_folder_response == DB_FULL) {
            char* text = "Database full (probably something is really wrong since we're trying to move)";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          } else if (move_folder_response == BLOCK_SIZE_EXCEEDED) {
            char* text = "Block size exceeded. Can't move? doesn't make sense.";
            initTextAreaDialog(text, strlen(text), DLG_OK);
          }
          main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
        }
      }
    }
    break;
    case MOVE_FOLDER_SELECT_PARENT_FOLDER: {
        DialogResult result = textAreaLoop(thumby);
        if (result == DLG_RES_NO) {
          initFolder(folder_browser_folder_id, selected_folder_id, -1);
          main_folder_ui_phase = FOLDER_BROWSER;
        } else if (result == DLG_RES_YES) {
          initFolder(ui_move_folder_to_folder_id, selected_folder_id, -1);
          main_folder_ui_phase = FOLDER_BROWSER;
        }
    }

    case RENAME_PHRASE_YES_NO: { 
      DialogResult result = textAreaLoop(thumby);
      if (result == DLG_RES_YES) {
        main_folder_ui_phase = RENAME_PHRASE;
        PhraseFolderAndName* selected_phrase = getPhrase(selected_phrase_id);
        initOnScreenKeyboard(selected_phrase->name, strlen(selected_phrase->name), false, false, false);
      } else if (result == DLG_RES_NO) {
        main_folder_ui_phase = FOLDER_MENU;
      }
    }
    break;
    case RENAME_PHRASE: {
      char* new_phrase_name = keyboardLoop(thumby);
      if (new_phrase_name != NULL) {
        if (strlen(new_phrase_name) == 0) {
          char* text = "Phrase name can't be empty.";
          initTextAreaDialog(text, strlen(text), DLG_OK);
          main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
        } else {
          UpdateResponse rename_phrase_response = renamePhrase(selected_phrase_id, new_phrase_name);
          if (rename_phrase_response == OK) {
            initFolder(folder_browser_folder_id, selected_folder_id, selected_phrase_id);
            main_folder_ui_phase = FOLDER_BROWSER;
          } else {
            if (rename_phrase_response == ERROR) {
              char* text = "Rename phrase ERROR.";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            } else if (rename_phrase_response == DB_FULL) {
              char* text = "Database full (probably something is really wrong since we're trying to rename)";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            } else if (rename_phrase_response == BLOCK_SIZE_EXCEEDED) {
              char* text = "Block size exceeded. Can't rename";
              initTextAreaDialog(text, strlen(text), DLG_OK);
            }
            main_folder_ui_phase = FOLDER_MENU_OPERATION_ERROR_REPORT;
          }
        }
      }
    }
    break;
  }
}

// BlockDAO Dialogs
void phraserFolderUiLoop(Thumby* thumby) {
  if (isMenuPhase()) {
    // Menu phase, all menus are ScreenList-based
    SelectionAndCode chosen = listLoopWithCode(thumby, true);
    if (chosen.selection != -1) {
      if (getSelectButton() == SELECT_BUTTON_A) {
        runMainFolderUiPhaseAction(chosen.selection, chosen.code);
      } else { //SELECT_BUTTON_B
        menuSwitch(chosen.selection);
      }
    }
  } else {
    // Dialog phase
    dialogActionsLoop(thumby);
  }
}
