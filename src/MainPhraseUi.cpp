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
  VIEW_HISTORY_ENTRY_WORD,
  VIEW_WORD_MENU,
  VIEW_HISTORY_ENTRY_MENU_WORD,
  
  EDIT_WORD_YES_NO,
  ENTER_NEW_WORD,
  EDIT_WORD,

  // BlockDAO Dialogs
  PHRASE_VIEW_OPERATION_ERROR_REPORT,
  PHRASE_MENU_OPERATION_ERROR_REPORT,

  VIEW_HISTORY_ENTRY_VIEW_ERROR_REPORT,
  VIEW_HISTORY_ENTRY_MENU_ERROR_REPORT,
};

MainPhraseUiPhase main_phrase_ui_phase = PHRASE_VIEW;

struct WordAndTemplate {
  Word* word;
  uint16_t word_template_id;
};

const int PHRASE_VIEW_RENAME_PHRASE = 1;
const int PHRASE_VIEW_GENERATE_WORD = 2;
const int PHRASE_VIEW_VIEW_WORD = 3;
const int PHRASE_VIEW_EDIT_WORD = 4;
const int PHRASE_VIEW_TYPE_WORD = 5;
const int PHRASE_VIEW_CHANGE_PHRASE_TEMPLATE = 6;
void initPhraseViewMenuScreenList(FullPhrase* phrase, WordAndTemplate* word_and_template, int selection) {
  int menu_items_count = 2;

  if (word_and_template != NULL) {
    if (isGenerateable(word_and_template->word->permissions)) { menu_items_count++; }
    if (isUserEditable(word_and_template->word->permissions)) { menu_items_count++; }
    if (isTypeable(word_and_template->word->permissions)) { menu_items_count++; }
    if (isViewable(word_and_template->word->permissions)) { menu_items_count++; }
  }

  int menu_item_cursor = 0;
  ListItem** screen_items = (ListItem**)malloc(menu_items_count * sizeof(ListItem*));

  char text[350];
  sprintf(text, "Rename phrase `%s`", phrase->phrase_name);
  screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Check, PHRASE_VIEW_RENAME_PHRASE);

  if (word_and_template != NULL) {
    if (isGenerateable(word_and_template->word->permissions)) {
      sprintf(text, "Generate `%s`", word_and_template->word->name);
      screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Stars, PHRASE_VIEW_GENERATE_WORD);
    }
    if (isViewable(word_and_template->word->permissions)) {
      sprintf(text, "View `%s`", word_and_template->word->name);
      screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_LookingGlass, PHRASE_VIEW_VIEW_WORD);
    }
    if (isUserEditable(word_and_template->word->permissions)) {
      sprintf(text, "Edit `%s`", word_and_template->word->name);
      screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Check, PHRASE_VIEW_EDIT_WORD);
    }
    if (isTypeable(word_and_template->word->permissions)) {
      sprintf(text, "Type `%s`", word_and_template->word->name);
      screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_TextOut, PHRASE_VIEW_TYPE_WORD);
    }
  }
  
  sprintf(text, "Change phrase template for `%s`", phrase->phrase_name);
  screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Copy, PHRASE_VIEW_CHANGE_PHRASE_TEMPLATE);
  
  if (selection >= menu_items_count) { selection = 0; }
  initList(screen_items, menu_items_count, selection);
  freeItemList(screen_items, menu_items_count);
}

const int UP_TO_PHRASE_LEVEL = 201;
const int HISTORY_ENTRY = 202;
void initCurrentPhraseHistoryScreenList(FullPhrase* phrase, int selection) {
  int phrase_history_size = arraylist_size(phrase->history);

  int menu_items_count = phrase_history_size + 1;
  int menu_item_cursor = 0;
  ListItem** screen_items = (ListItem**)malloc(menu_items_count * sizeof(ListItem*));

  screen_items[menu_item_cursor++] = createListItemWithCode("..", phraser_Icon_ToParentFolder, UP_TO_PHRASE_LEVEL);

  if (phrase_history_size > 0) {
    screen_items[menu_item_cursor++] = createListItemWithCode("History 0 (current)", phraser_Icon_Message, HISTORY_ENTRY);

    for (int i = 1; i < phrase_history_size; i++) {
      char text[350];
      sprintf(text, "History %d", i);
      screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Message, HISTORY_ENTRY);
    }
  }

  //initialize Screen List
  if (selection >= menu_items_count) { selection = 0; }
  initList(screen_items, menu_items_count, selection);
  freeItemList(screen_items, menu_items_count);
}

const int DELETE_HISTORY_ENTRY = 301;
const int MAKE_HISTORY_ENTRY_CURRENT = 302;
void initPhraseHistoryMenuScreenList(PhraseHistory* phrase_history, int history_index, int selection) {
  int menu_items_count = 2;
  if (0 == history_index) { menu_items_count = 1; }

  int menu_item_cursor = 0;
  ListItem** screen_items = (ListItem**)malloc(menu_items_count * sizeof(ListItem*));

  char text[350];
  sprintf(text, "Delete (History %d)", history_index);
  screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_X, DELETE_HISTORY_ENTRY);

  if (0 != history_index) {
    sprintf(text, "Make current (History %d)", history_index);
    screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_Upload, MAKE_HISTORY_ENTRY_CURRENT);
  }
  
  if (selection >= menu_items_count) { selection = 0; }
  initList(screen_items, menu_items_count, selection);
  freeItemList(screen_items, menu_items_count);
}

void initPhraseHistoryEntryMenuScreenList(Word* word, int selection) {
  int menu_items_count = 0;

  if (word != NULL) {
    if (isTypeable(word->permissions)) { menu_items_count++; }
    if (isViewable(word->permissions)) { menu_items_count++; }
  }

  int menu_item_cursor = 0;
  ListItem** screen_items = (ListItem**)malloc(menu_items_count * sizeof(ListItem*));

  char text[350];
  if (word != NULL) {
    if (isViewable(word->permissions)) {
      sprintf(text, "View `%s`", word->name);
      screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_LookingGlass, PHRASE_VIEW_VIEW_WORD);
    }
    if (isTypeable(word->permissions)) {
      sprintf(text, "Type `%s`", word->name);
      screen_items[menu_item_cursor++] = createListItemWithCode(text, phraser_Icon_TextOut, PHRASE_VIEW_TYPE_WORD);
    }
  }
  
  if (selection >= menu_items_count) { selection = 0; }
  initList(screen_items, menu_items_count, selection);
  freeItemList(screen_items, menu_items_count);
}

const int UP_TO_PHRASE_HISTORY_LEVEL = 401;
const int HISTORY_ENTRY_WORD = 402;
void initPhraseHistoryEntryScreenList(PhraseHistory* history_entry, int selection) {
  int menu_items_count = arraylist_size(history_entry->phrase) + 1;
  
  int menu_item_cursor = 0;
  ListItem** screen_items = (ListItem**)malloc(menu_items_count * sizeof(ListItem*));

  screen_items[menu_item_cursor++] = createListItemWithCode("..", phraser_Icon_ToParentFolder, UP_TO_PHRASE_HISTORY_LEVEL);

  for (int i = 0; i < arraylist_size(history_entry->phrase); i++) {
    Word* word = (Word*)arraylist_get(history_entry->phrase, i);
    screen_items[menu_item_cursor++] = createListItemWithCode(word->name, word->icon, HISTORY_ENTRY_WORD);
  }

  //initialize Screen List
  if (selection >= menu_items_count) { selection = 0; }
  initList(screen_items, menu_items_count, selection);
  freeItemList(screen_items, menu_items_count);
}

const int UP_TO_PARENT_LEVEL = 101;
const int DOWN_TO_HISTORY = 102;
const int WORD_AND_TEMPLATE = 103;
arraylist* words_and_templates = NULL;// arraylist<WordAndTemplate>
void initCurrentPhraseScreenList(FullPhrase* phrase, int selection) {
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
    word_and_template->word_template_id = word_template->wordTemplateId;

    arraylist_add(words_and_templates, word_and_template);
  }

  for (int i = 0; i < arraylist_size(words); i++) {
    Word* word = (Word*)arraylist_get(words, i);
    WordAndTemplate* word_and_template = (WordAndTemplate*)malloc(sizeof(WordAndTemplate));
    word_and_template->word = word;
    word_and_template->word_template_id = -1;

    arraylist_add(words_and_templates, word_and_template);
  }
  arraylist_destroy(words);

  int menu_items_count = arraylist_size(words_and_templates) + 2;
  int menu_item_cursor = 0;
  ListItem** screen_items = (ListItem**)malloc(menu_items_count * sizeof(ListItem*));

  screen_items[menu_item_cursor++] = createListItemWithCode("..", phraser_Icon_ToParentFolder, UP_TO_PARENT_LEVEL);

  for (int i = 0; i < arraylist_size(words_and_templates); i++) {
    WordAndTemplate* word_and_template = (WordAndTemplate*)arraylist_get(words_and_templates, i);
    WordTemplate* word_template = getWordTemplate(word_and_template->word_template_id);

    char* text = word_template == NULL ? word_and_template->word->name : word_template->wordTemplateName;
    phraser_Icon_enum_t icon;
    if (word_template != NULL && word_and_template->word != NULL) {
      icon = word_template->icon;
    } else if (word_template == NULL) {
      icon = phraser_Icon_Question;
    } else if (word_and_template->word == NULL) {
      icon = phraser_Icon_X;
    }
    screen_items[menu_item_cursor++] = createListItemWithCode(text, icon, WORD_AND_TEMPLATE);
  }

  screen_items[menu_item_cursor++] = createListItemWithCode("->History", phraser_Icon_Ledger, DOWN_TO_HISTORY);

  //initialize Screen List
  if (selection >= menu_items_count) { selection = 0; }
  initList(screen_items, menu_items_count, selection);
  freeItemList(screen_items, menu_items_count);
}

FullPhrase* current_phrase = NULL;
void initPhraseView(FullPhrase* phrase) {
  current_phrase = phrase; 
  main_phrase_ui_phase = PHRASE_VIEW;
  initCurrentPhraseScreenList(current_phrase, 0);
}

// ----------------------------------------------------------------------------

void typeWord(char* word, int word_length) {
  for (int i = 0; i < word_length; i++) {
    char c = word[i];
    setChar(c);
    Keyboard.print(c);
  }
}

PhraseHistory* current_history_entry;
int init_phrase_history_view_selection = 0;
int init_phrase_history_view_menu_selection = 0;
int init_phrase_history_entry_view_selection = 0;
int init_phrase_history_entry_view_menu_selection = 0;
void phraseHistoryEntryViewAction(int chosen_item, int code) {
  if (code == UP_TO_PHRASE_HISTORY_LEVEL) {
    initCurrentPhraseHistoryScreenList(current_phrase, init_phrase_history_view_selection);
    main_phrase_ui_phase = PHRASE_HISTORY;
  } else if (code == HISTORY_ENTRY_WORD) {
    Word* word = NULL;
    init_phrase_history_entry_view_selection = chosen_item;
    if (chosen_item > 0) {
      int index = chosen_item - 1;
      if (current_history_entry != NULL && current_history_entry->phrase != NULL && index < arraylist_size(current_history_entry->phrase)) {
        word = (Word*)arraylist_get(current_history_entry->phrase, index);
      }
    }

    if (word == NULL) {
      // ERROR: word and template not chosen
      char* text = "ERROR: Word not chosen.";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      main_phrase_ui_phase = VIEW_HISTORY_ENTRY_VIEW_ERROR_REPORT;
    } else if (word == NULL || word->word == NULL || strlen(word->word) == 0) {
      // ERROR: word has no value, generate or edit
      char* text = "Word has no value assigned\nGenerate or edit.";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      main_phrase_ui_phase = VIEW_HISTORY_ENTRY_VIEW_ERROR_REPORT;
    } else {
      if (isTypeable(word->permissions)) {
        // Type word
        typeWord(word->word, strlen(word->word));
      } else if (isViewable(word->permissions)) {
        // View word
        initTextAreaDialog(word->word, strlen(word->word), TEXT_AREA);
        main_phrase_ui_phase = VIEW_HISTORY_ENTRY_WORD;
      }
    }
  }
}

void phraseHistoryEntryViewMenuAction(int chosen_item, int code) {
  Word* word = NULL;
  
  if (init_phrase_history_entry_view_selection > 0) {
    int index = init_phrase_history_entry_view_selection - 1;
    
    if (current_history_entry != NULL && current_history_entry->phrase != NULL && index < arraylist_size(current_history_entry->phrase)) {
      word = (Word*)arraylist_get(current_history_entry->phrase, index);
    }

    init_phrase_history_entry_view_menu_selection = chosen_item;

    if (word == NULL) {
      // ERROR: word and template not chosen
      char* text = "ERROR: Word not chosen.";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      main_phrase_ui_phase = VIEW_HISTORY_ENTRY_MENU_ERROR_REPORT;
    } else if (word == NULL || word->word == NULL || strlen(word->word) == 0) {
      // ERROR: word has no value, generate or edit
      char* text = "Word has no value assigned\nGenerate or edit.";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      main_phrase_ui_phase = VIEW_HISTORY_ENTRY_MENU_ERROR_REPORT;
    } else {
      if (code == PHRASE_VIEW_VIEW_WORD && isViewable(word->permissions)) {
        // View word
        initTextAreaDialog(word->word, strlen(word->word), TEXT_AREA);
        main_phrase_ui_phase = VIEW_HISTORY_ENTRY_MENU_WORD;
      } else if (code == PHRASE_VIEW_TYPE_WORD && isTypeable(word->permissions)) {
        // Type word
        typeWord(word->word, strlen(word->word));
      }
    }
  }
}

int init_phrase_view_selection = 0;
void phraseHistoryViewAction(int chosen_item, int code) {
  if (code == UP_TO_PHRASE_LEVEL) {
    initCurrentPhraseScreenList(current_phrase, init_phrase_view_selection);
    main_phrase_ui_phase = PHRASE_VIEW;
  } else if (code == HISTORY_ENTRY) {
    // switch to History Entry View
    init_phrase_history_view_selection = chosen_item;
    if (chosen_item > 0) {
      int index = chosen_item - 1;

      if (current_phrase != NULL && current_phrase->history != NULL && index < arraylist_size(current_phrase->history)) {
        current_history_entry = (PhraseHistory*)arraylist_get(current_phrase->history, index);
      }

      if (current_history_entry != NULL) {
        main_phrase_ui_phase = PHRASE_HISTORY_ENTRY;
        initPhraseHistoryEntryScreenList(current_history_entry/*, index*/, 0);
      }
    }
  }
}

void phraseHistoryViewMenuAction(int chosen_item, int code) {
  //
}

bool mainPhraseViewAction(int chosen_item, int code) {
  if (code == UP_TO_PARENT_LEVEL) {
    // Returning false from here moves back to Folders UI
    return false;
  } else if (code == DOWN_TO_HISTORY) {
    // switch to History View
    init_phrase_view_selection = chosen_item;
    initCurrentPhraseHistoryScreenList(current_phrase, 0);
    main_phrase_ui_phase = PHRASE_HISTORY;
  } else if (code == WORD_AND_TEMPLATE) {
    WordAndTemplate* word_and_template = NULL;
    init_phrase_view_selection = chosen_item;
    if (chosen_item > 0) {
      int index = chosen_item - 1;
      if (words_and_templates != NULL && index < arraylist_size(words_and_templates)) {
        word_and_template = (WordAndTemplate*)arraylist_get(words_and_templates, index);
      }
    }

    if (word_and_template == NULL) {
      // ERROR: word and template not chosen
      char* text = "ERROR: Word and Word template not chosen.";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      main_phrase_ui_phase = PHRASE_VIEW_OPERATION_ERROR_REPORT;
    } else if (word_and_template->word == NULL || word_and_template->word->word == NULL || strlen(word_and_template->word->word) == 0) {
      // ERROR: word has no value, generate or edit
      char* text = "Word has no value assigned\nGenerate or edit.";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      main_phrase_ui_phase = PHRASE_VIEW_OPERATION_ERROR_REPORT;
    } else {
      if (isTypeable(word_and_template->word->permissions)) {
        // Type word
        typeWord(word_and_template->word->word, strlen(word_and_template->word->word));
      } else if (isViewable(word_and_template->word->permissions)) {
        // View word
        initTextAreaDialog(word_and_template->word->word, strlen(word_and_template->word->word), TEXT_AREA);
        main_phrase_ui_phase = VIEW_WORD;
      }
    }
  }

  return true;
}

void mainPhraseViewMenuAction(int chosen_item, int code) {
  if (code == PHRASE_VIEW_RENAME_PHRASE) {
    // TODO: Rename Phrase
  } else if (code == PHRASE_VIEW_CHANGE_PHRASE_TEMPLATE) {
    // TODO: Change Phrase Template
  } else if (code == PHRASE_VIEW_GENERATE_WORD) {
    // TODO: Change Phrase Template
  } else if (code == PHRASE_VIEW_EDIT_WORD) {
    // TODO: Change Phrase Template
  } else if (code == PHRASE_VIEW_VIEW_WORD || code == PHRASE_VIEW_TYPE_WORD) {
    WordAndTemplate* word_and_template = NULL;
    if (init_phrase_view_selection > 0) {
      int index = init_phrase_view_selection - 1;
      if (words_and_templates != NULL && index < arraylist_size(words_and_templates)) {
        word_and_template = (WordAndTemplate*)arraylist_get(words_and_templates, index);
      }
    }

    init_phrase_history_view_menu_selection = chosen_item;
    if (word_and_template == NULL) {
      // ERROR: word and template not chosen
      char* text = "ERROR: Word and Word template not chosen.";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      main_phrase_ui_phase = PHRASE_MENU_OPERATION_ERROR_REPORT;
    } else if (word_and_template->word == NULL || word_and_template->word->word == NULL || strlen(word_and_template->word->word) == 0) {
      // ERROR: word has no value, generate or edit
      char* text = "Word has no value assigned\nGenerate or edit.";
      initTextAreaDialog(text, strlen(text), DLG_OK);
      main_phrase_ui_phase = PHRASE_MENU_OPERATION_ERROR_REPORT;
    } else {
      if (code == PHRASE_VIEW_VIEW_WORD && isViewable(word_and_template->word->permissions)) {
        // View word
        initTextAreaDialog(word_and_template->word->word, strlen(word_and_template->word->word), TEXT_AREA);
        main_phrase_ui_phase = VIEW_WORD_MENU;
      } else if (code == PHRASE_VIEW_TYPE_WORD && isTypeable(word_and_template->word->permissions)) {
        // Type word
        typeWord(word_and_template->word->word, strlen(word_and_template->word->word));
      } 
    }
  }
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
  initCurrentPhraseScreenList(current_phrase, init_phrase_view_selection);
}

void init_phrase_view_menu(int chosen_item) {
  WordAndTemplate* word_and_template = NULL;
  init_phrase_view_selection = chosen_item;
  if (chosen_item > 0) {
    int index = chosen_item - 1;
    if (words_and_templates != NULL && index < arraylist_size(words_and_templates)) {
      word_and_template = (WordAndTemplate*)arraylist_get(words_and_templates, index);
    }
  }

  main_phrase_ui_phase = PHRASE_VIEW_MENU;
  initPhraseViewMenuScreenList(current_phrase, word_and_template, 0);
}

void init_phrase_history_view(int chosen_item) {
  main_phrase_ui_phase = PHRASE_HISTORY;
  initCurrentPhraseHistoryScreenList(current_phrase, init_phrase_history_view_selection);
}

void init_phrase_history_view_menu(int chosen_item) {
  PhraseHistory* phrase_history = NULL;
  init_phrase_history_view_selection = chosen_item;
  if (chosen_item > 0) {
    int index = chosen_item - 1;
    
    if (current_phrase != NULL && current_phrase->history != NULL && index < arraylist_size(current_phrase->history)) {
      phrase_history = (PhraseHistory*)arraylist_get(current_phrase->history, index);
    }

    if (phrase_history == NULL) {
      main_phrase_ui_phase = PHRASE_HISTORY_MENU;
      initPhraseHistoryMenuScreenList(phrase_history, index, 0);
    }
  }
}

void init_phrase_history_entry_view(int chosen_item) {
  initPhraseHistoryEntryScreenList(current_history_entry, init_phrase_history_entry_view_selection);
  main_phrase_ui_phase = PHRASE_HISTORY_ENTRY; 
}

void init_phrase_history_entry_view_menu(int chosen_item) {
  Word* word = NULL;
  init_phrase_history_entry_view_selection = chosen_item;
  if (chosen_item > 0) {
    int index = chosen_item - 1;
    
    if (current_history_entry != NULL && current_history_entry->phrase != NULL && index < arraylist_size(current_history_entry->phrase)) {
      word = (Word*)arraylist_get(current_history_entry->phrase, index);
    }

    if (word != NULL) {
      initPhraseHistoryEntryMenuScreenList(word, 0);
      main_phrase_ui_phase = PHRASE_HISTORY_ENTRY_MENU; 
    }
  }
}

// -------------------------------------------------------------------------------

void phraseDialogActionsLoop(Thumby* thumby) {
  switch (main_phrase_ui_phase) {
    case PHRASE_VIEW_OPERATION_ERROR_REPORT:
    case VIEW_WORD: {
      DialogResult result = textAreaLoop(thumby);
      if (result == DLG_RES_OK) {
        initCurrentPhraseScreenList(current_phrase, init_phrase_view_selection);
        main_phrase_ui_phase = PHRASE_VIEW;
      }
    }
    break;

    case PHRASE_MENU_OPERATION_ERROR_REPORT:
    case VIEW_WORD_MENU: {
      DialogResult result = textAreaLoop(thumby);
      if (result == DLG_RES_OK) {
        WordAndTemplate* word_and_template = NULL;
        if (init_phrase_view_selection > 0) {
          int index = init_phrase_view_selection - 1;
          if (words_and_templates != NULL && index < arraylist_size(words_and_templates)) {
            word_and_template = (WordAndTemplate*)arraylist_get(words_and_templates, index);
          }
        }
      
        main_phrase_ui_phase = PHRASE_VIEW_MENU;
        initPhraseViewMenuScreenList(current_phrase, word_and_template, init_phrase_history_view_menu_selection);
      }
    }
    break;

    case VIEW_HISTORY_ENTRY_VIEW_ERROR_REPORT: 
    case VIEW_HISTORY_ENTRY_WORD: {
      DialogResult result = textAreaLoop(thumby);
      if (result == DLG_RES_OK) {
        initPhraseHistoryEntryScreenList(current_history_entry, init_phrase_history_entry_view_selection);
        main_phrase_ui_phase = PHRASE_HISTORY_ENTRY;
      }
    }
    break;

    case VIEW_HISTORY_ENTRY_MENU_ERROR_REPORT: 
    case VIEW_HISTORY_ENTRY_MENU_WORD: {
      DialogResult result = textAreaLoop(thumby);
      if (result == DLG_RES_OK) {
        Word* word = NULL;
        if (init_phrase_history_entry_view_selection > 0) {
          int index = init_phrase_history_entry_view_selection - 1;
          
          if (current_history_entry != NULL && current_history_entry->phrase != NULL && index < arraylist_size(current_history_entry->phrase)) {
            word = (Word*)arraylist_get(current_history_entry->phrase, index);
          }
        }
        initPhraseHistoryEntryMenuScreenList(word, init_phrase_history_entry_view_menu_selection);
        main_phrase_ui_phase = PHRASE_HISTORY_ENTRY_MENU;
      }
    }
    break;
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
        init_phrase_history_view_menu(chosen_item);
        break;
    case PHRASE_HISTORY_MENU: 
        init_phrase_history_view(chosen_item);
        break;
    case PHRASE_HISTORY_ENTRY: 
        init_phrase_history_entry_view_menu(chosen_item);
        break;
    case PHRASE_HISTORY_ENTRY_MENU: 
        init_phrase_history_entry_view(chosen_item);
        break;
  }
}

bool runMainPhraseUiPhaseAction(int chosen_item, int code) {
  switch (main_phrase_ui_phase) {
    case PHRASE_VIEW: return mainPhraseViewAction(chosen_item, code); break;
    case PHRASE_VIEW_MENU: mainPhraseViewMenuAction(chosen_item, code); break;
    case PHRASE_HISTORY: phraseHistoryViewAction(chosen_item, code); break;
    case PHRASE_HISTORY_MENU: phraseHistoryViewMenuAction(chosen_item, code); break;
    case PHRASE_HISTORY_ENTRY: phraseHistoryEntryViewAction(chosen_item, code); break;
    case PHRASE_HISTORY_ENTRY_MENU: phraseHistoryEntryViewMenuAction(chosen_item, code); break;
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
