#include "MainPhraseUi.h"

#include "TextAreaDialog.h"
#include "ScreenList.h"
#include "ScreenKeyboard.h"
#include "BlockCache.h"
#include "BlockDAO.h"
#include "SerialUtils.h"

enum MainPhraseUiPhase {
  // Menus
  PHRASE_VIEW,
  PHRASE_VIEW_MENU,
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
  PHRASE_MENU_OPERATION_ERROR_REPORT,
};

MainPhraseUiPhase main_phrase_ui_phase = PHRASE_VIEW;

void initCurrentPhraseScreenList(FullPhrase* phrase) {
  // TODO: implement
}

FullPhrase* current_phrase = NULL;
void initPhrase(FullPhrase* phrase) {
  current_phrase = phrase; 
  main_phrase_ui_phase = PHRASE_VIEW;
  initCurrentPhraseScreenList(current_phrase);
}

// ----------------------------------------------------------------------------

void mainPhraseViewAction(int chosen_item) {
  //
}

const int PHRASE_VIEW_RENAME_PHRASE = 1;
const int PHRASE_VIEW_GENERATE_WORD = 2;
const int PHRASE_VIEW_VIEW_WORD = 3;
const int PHRASE_VIEW_EDIT_WORD = 4;
const int PHRASE_VIEW_TYPE_WORD = 5;
const int PHRASE_VIEW_CHANGE_PHRASE_TEMPLATE = 6;
void mainPhraseViewMenuAction(int chosen_item, int code) {
  //
}

bool isPhraseMenuPhase() {
  switch (main_phrase_ui_phase) {
    case PHRASE_VIEW:
    case PHRASE_VIEW_MENU:
    case PHRASE_HISTORY:
    case PHRASE_HISTORY_MENU:
    case PHRASE_HISTORY_ENTRY:
    case PHRASE_HISTORY_ENTRY_MENU: return true;
  }
  return false;
}


void init_phrase_view(int chosen_item) {
  main_phrase_ui_phase = PHRASE_VIEW;
  initCurrentPhraseScreenList(current_phrase);
}

void init_phrase_view_menu(int chosen_item) {
  main_phrase_ui_phase = PHRASE_VIEW_MENU;

/*  FolderPhraseId* selection = getFolderBrowserSelection(chosen_item);
  if (selection != NULL) {
    selected_folder_id = selection->folder_id;
    selected_phrase_id = selection->phrase_id;
  } else {
    selected_folder_id = -1;
    selected_phrase_id = -1;
  }
*/
}

// -------------------------------------------------------------------------------

void phraseDialogActionsLoop(Thumby* thumby) {
  switch (main_phrase_ui_phase) {
    // TODO: implement
  }
}

void phraseMenuSwitch(int chosen_item) {
  // Switch phases that support menu screens to correspoding menu screen and back
  switch (main_phrase_ui_phase) {
    case PHRASE_VIEW: 
        init_phrase_view_menu(chosen_item);
        main_phrase_ui_phase = PHRASE_VIEW_MENU; 
        break;
    case PHRASE_VIEW_MENU: 
        init_phrase_view(chosen_item);
        main_phrase_ui_phase = PHRASE_VIEW; 
        break;
    case PHRASE_HISTORY: 
        main_phrase_ui_phase = PHRASE_HISTORY_MENU; 
        break;
    case PHRASE_HISTORY_MENU: 
        main_phrase_ui_phase = PHRASE_HISTORY; 
        break;
    case PHRASE_HISTORY_ENTRY: 
        main_phrase_ui_phase = PHRASE_HISTORY_ENTRY_MENU; 
        break;
    case PHRASE_HISTORY_ENTRY_MENU: 
        main_phrase_ui_phase = PHRASE_HISTORY_ENTRY; 
        break;
  }
}

void runMainPhraseUiPhaseAction(int chosen_item, int code) {
  switch (main_phrase_ui_phase) {
    case PHRASE_VIEW: mainPhraseViewAction(chosen_item); break;
    case PHRASE_VIEW_MENU: mainPhraseViewMenuAction(chosen_item, code); break;
    case PHRASE_HISTORY:
    case PHRASE_HISTORY_MENU:
    case PHRASE_HISTORY_ENTRY:
    case PHRASE_HISTORY_ENTRY_MENU:
    default: return;
  }
}

// BlockDAO Dialogs
void phraserPhraseUiLoop(Thumby* thumby) {
  if (isPhraseMenuPhase()) {
    // Menu phase, all menus are ScreenList-based
    SelectionAndCode chosen = listLoopWithCode(thumby, true);
    if (chosen.selection != -1) {
      if (getSelectButton() == SELECT_BUTTON_A) {
        runMainPhraseUiPhaseAction(chosen.selection, chosen.code);
      } else { //SELECT_BUTTON_B
        phraseMenuSwitch(chosen.selection);
      }
    }
  } else {
    // Dialog phase
    phraseDialogActionsLoop(thumby);
  }
}
