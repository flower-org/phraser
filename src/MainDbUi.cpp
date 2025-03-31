#include "MainDbUi.h"

#include "TextAreaDialog.h"
#include "ScreenList.h"
#include "BlockCache.h"

Folder* current_folder;
//  arraylist<FolderOrPhrase*>
arraylist* current_folder_content;

void initCurrentFolderScreenList(int select_folder_id, int select_phrase_id) {
  int selection = 0;
  if (current_folder_content != NULL) {
    for (int i = 0; i < arraylist_size(current_folder_content); i++) {
      free(arraylist_get(current_folder_content, i));
    }
    arraylist_destroy(current_folder_content);
    current_folder_content = NULL;
  }

  current_folder_content = getFolderContent(current_folder->folderId);
  if (current_folder_content != NULL) {
    int current_folder_content_size = arraylist_size(current_folder_content);
    int screen_item_cursor = 0;
    int screen_item_count = current_folder_content_size;
    if (current_folder->folderId > 0) { screen_item_count++; }
    
    ListItem** screen_items = (ListItem**)malloc(screen_item_count * sizeof(ListItem*));

    if (current_folder->folderId > 0) { 
      screen_items[screen_item_cursor++] = createListItem("..", phraser_Icon_ToParentFolder);
    }

    for (int i = 0; i < arraylist_size(current_folder_content); i++) {
      FolderOrPhrase* folderOrPhrase = (FolderOrPhrase*)arraylist_get(current_folder_content, i);
      if (folderOrPhrase->folder != NULL) {
        if (select_folder_id == folderOrPhrase->folder->folderId) {
          selection = screen_item_cursor;
        }
        screen_items[screen_item_cursor++] = createListItem(folderOrPhrase->folder->folderName, phraser_Icon_Folder);
      } else {
        if (select_phrase_id == folderOrPhrase->phrase->phraseBlockId) {
          selection = screen_item_cursor;
        }
        screen_items[screen_item_cursor++] = createListItem(folderOrPhrase->phrase->name, phraser_Icon_Ledger);
      }
    }

    initList(screen_items, screen_item_count, selection);
    freeItemList(screen_items, screen_item_count);
  } else {
    int screen_item_count = 0;
    if (current_folder->folderId > 0) { screen_item_count++; }
    ListItem** screen_items = (ListItem**)malloc(screen_item_count * sizeof(ListItem*));
    if (current_folder->folderId > 0) { 
      screen_items[0] = createListItem("..", phraser_Icon_ToParentFolder);
    }
    initList(screen_items, screen_item_count);
    freeItemList(screen_items, screen_item_count);
  }
}

void initFolder(int folder_id) {
  Folder* next_folder = getFolder(folder_id);
  if (next_folder != NULL) {
    int select_folder_id = current_folder->folderId;
    current_folder = next_folder;
    initCurrentFolderScreenList(select_folder_id, -1);
  }
}

void phraserDbUiInit() {
  initFolder(0);
}

void initPhrase(int phrase_block_id) {
  // TODO: implement
}

void phraserDbUiLoop(Thumby* thumby) {
  int chosenItem = listLoop(thumby);
  if (chosenItem != -1) {
    int folder_index = chosenItem;
    if (current_folder->folderId > 0) {
      if (chosenItem == 0) {
        // Up one level
        initFolder(current_folder->parentFolderId);
        return;
      } else {
        folder_index--;
      }
    }

    FolderOrPhrase* new_selected_folder = (FolderOrPhrase*)arraylist_get(current_folder_content, folder_index);
    if (new_selected_folder != NULL) {
      if (new_selected_folder->folder != NULL) {
        initFolder(new_selected_folder->folder->folderId);
      } else {
        initPhrase(new_selected_folder->phrase->phraseBlockId);
      }
    }
  }
}
