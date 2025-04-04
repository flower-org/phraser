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

struct WordAndTemplate {
  Word* word;
  WordTemplate* word_template;
};

const int UP_TO_PARENT_LEVEL = 101;
const int DOWN_TO_HISTORY = 102;
const int WORD_AND_TEMPLATE = 103;
arraylist* words_and_templates = NULL;// arraylist<WordAndTemplate>
void initCurrentPhraseScreenList(FullPhrase* phrase) {
  if (words_and_templates != NULL) {
    for (int i = 0; i < arraylist_size(words_and_templates); i++) {
      WordAndTemplate* word_and_template = (WordAndTemplate*)arraylist_get(words_and_templates, i);  
      free(word_and_template);
    }
    arraylist_destroy(words_and_templates);
    words_and_templates = NULL;
  }

  PhraseHistory* history0 = (PhraseHistory*)arraylist_get(phrase->history, 0);

  arraylist* words = arraylist_create();// arraylist<Word>
  for (int i = 0; i < arraylist_size(history0->phrase); i++) {
    Word* word = (Word*)arraylist_get(history0->phrase, i);
    arraylist_add(words, word);
  }

  words_and_templates = arraylist_create();// arraylist<WordAndTemplate>
  PhraseTemplate* phrase_template = getPhraseTemplate(phrase->phrase_template_id);
  for (int i = 0; i < arraylist_size(phrase_template->wordTemplateIds); i++) {
    uint16_t word_template_id = (uint32_t)arraylist_get(phrase_template->wordTemplateIds, i);
    uint8_t word_template_ordinal = (uint32_t)arraylist_get(phrase_template->wordTemplateOrdinals, i);
    WordTemplate* word_template = getWordTemplate(word_template_id);

    //TODO: optimizable with hashtable
    int matching_word_id = -1;
    for (int j = 0; j < arraylist_size(words); j++) {
      Word* word = (Word*)arraylist_get(words, j);
      if (word->word_template_id == word_template_id && word->word_template_ordinal == word_template_ordinal) {
        matching_word_id = j;
      }
    }

    Word* matching_word = NULL;
    if (matching_word_id != -1) { matching_word = (Word*)arraylist_remove(words, matching_word_id); }
    WordAndTemplate* word_and_template = (WordAndTemplate*)malloc(sizeof(WordAndTemplate));
    word_and_template->word = matching_word;
    word_and_template->word_template = word_template;

    arraylist_add(words_and_templates, word_and_template);
  }

  for (int i = 0; i < arraylist_size(words); i++) {
    Word* word = (Word*)arraylist_get(words, i);
    WordAndTemplate* word_and_template = (WordAndTemplate*)malloc(sizeof(WordAndTemplate));
    word_and_template->word = word;
    word_and_template->word_template = NULL;

    arraylist_add(words_and_templates, word_and_template);
  }
  arraylist_destroy(words);

  int menu_items_count = arraylist_size(words_and_templates) + 2;
  int menu_item_cursor = 0;
  ListItem** screen_items = (ListItem**)malloc(menu_items_count * sizeof(ListItem*));

  screen_items[menu_item_cursor++] = createListItemWithCode("..", phraser_Icon_ToParentFolder, UP_TO_PARENT_LEVEL);

  for (int i = 0; i < arraylist_size(words_and_templates); i++) {
    WordAndTemplate* word_and_template = (WordAndTemplate*)arraylist_get(words_and_templates, i);

    char* text = word_and_template->word_template == NULL ? word_and_template->word->name : word_and_template->word_template->wordTemplateName;
    phraser_Icon_enum_t icon;
    if (word_and_template->word_template != NULL && word_and_template->word != NULL) {
      icon = word_and_template->word_template->icon;
    } else if (word_and_template->word_template == NULL) {
      icon = phraser_Icon_Question;
    } else if (word_and_template->word == NULL) {
      icon = phraser_Icon_X;
    }
    screen_items[menu_item_cursor++] = createListItemWithCode(text, icon, WORD_AND_TEMPLATE);
  }

  screen_items[menu_item_cursor++] = createListItemWithCode("->History", phraser_Icon_Ledger, DOWN_TO_HISTORY);

  //initialize Screen List
  initList(screen_items, menu_items_count);
  freeItemList(screen_items, menu_items_count);
}

FullPhrase* current_phrase = NULL;
void initPhrase(FullPhrase* phrase) {
  current_phrase = phrase; 
  main_phrase_ui_phase = PHRASE_VIEW;
  initCurrentPhraseScreenList(current_phrase);
}

// ----------------------------------------------------------------------------

bool mainPhraseViewAction(int chosen_item, int code) {
  if (code == UP_TO_PARENT_LEVEL) {
    return false;
  }

  return true;
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
        break;
    case PHRASE_VIEW_MENU: 
        init_phrase_view(chosen_item);
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

bool runMainPhraseUiPhaseAction(int chosen_item, int code) {
  switch (main_phrase_ui_phase) {
    case PHRASE_VIEW: return mainPhraseViewAction(chosen_item, code); break;
    case PHRASE_VIEW_MENU: mainPhraseViewMenuAction(chosen_item, code); break;
    case PHRASE_HISTORY:
    case PHRASE_HISTORY_MENU:
    case PHRASE_HISTORY_ENTRY:
    case PHRASE_HISTORY_ENTRY_MENU:
    default: return true;
  }
  return true;
}

// BlockDAO Dialogs
bool phraserPhraseUiLoop(Thumby* thumby) {
  if (isPhraseMenuPhase()) {
    // Menu phase, all menus are ScreenList-based
    SelectionAndCode chosen = listLoopWithCode(thumby, true);
    if (chosen.selection != -1) {
      if (getSelectButton() == SELECT_BUTTON_A) {
        return runMainPhraseUiPhaseAction(chosen.selection, chosen.code);
      } else { //SELECT_BUTTON_B
        phraseMenuSwitch(chosen.selection);
      }
    }
  } else {
    // Dialog phase
    phraseDialogActionsLoop(thumby);
  }
  // TODO: return false to get back to folders menu
  return true;
}
