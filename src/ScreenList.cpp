#include "ScreenList.h"

const int max_letters_to_show = 10;
const int lines_count = 4;
const int lines[lines_count] = { 0, 10, 20, 30 };

ListItem** list_items = NULL;
int list_item_count;
int item_cursor;
int selection_pos;

void drawIcon(Thumby* thumby, int lineIndex, phraser_Icon_enum_t icon) {
  const int icon_offset_x = 1;
  const int icon_offset_y = 2;
  const int line = lines[lineIndex];

  switch (icon) {
    case phraser_Icon_Key: 
      drawKey(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Login: 
      drawLogin(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Asterisk: 
      drawAsterisk(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Lock: 
      drawLock(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Aa: 
      drawAa(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Star: 
      drawStar(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Settings: 
      drawSettings(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Folder: 
      drawFolderIcon(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_ToParentFolder: 
      drawToParentFolder(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_LookingGlass: 
      drawLookingGlass(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_LTTriangle: 
      drawLTTriangle(thumby, icon_offset_x, line+icon_offset_y, WHITE, false);
      break;
    case phraser_Icon_GTTriangle: 
      drawGTTriangle(thumby, icon_offset_x, line+icon_offset_y, WHITE, false);
      break;
    case phraser_Icon_TextOut: 
      drawTextOut(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Ledger: 
      drawLedger(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_PlusMinus: 
      drawPlusMinus(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Stars: 
      drawStars(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Message: 
      drawMessage(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Quote: 
      drawQuote(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Question: 
      drawQuestion(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Plus: 
      drawPlus(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Minus: 
      drawMinus(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_X: 
      drawX(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Check: 
      drawCheck(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Copy: 
      drawCopy(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Download: 
      drawDownload(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Upload: 
      drawUpload(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Skull: 
      drawSkull(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser_Icon_Email: 
      drawEmail(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    default: 
      drawSpace(thumby, icon_offset_x+2, line+icon_offset_y-1, WHITE, false);
      break;
  }
}

void drawLineSelectionBorder(Thumby* thumby, int lineIndex) {
  const int selection_border_x1 = 11;
  const int selection_border_x2 = 67;
  const int selection_border_offset_y1 = 0;
  const int selection_border_offset_y2 = 10;

  const int line = lines[lineIndex];
  drawRect(thumby, selection_border_x1, line + selection_border_offset_y1, 
    selection_border_x2, line + selection_border_offset_y2, WHITE);
}

void drawUpScrollable(Thumby* thumby) {
  thumby->drawLine(69, 1, 70, 0, WHITE);
  thumby->drawLine(70, 0, 71, 1, WHITE);
}

void drawDownScrollable(Thumby* thumby) {
  thumby->drawLine(69, 38, 70, 39, WHITE);
  thumby->drawLine(70, 39, 71, 38, WHITE);
}

void printLineText(Thumby* thumby, int lineIndex, char* str) {
  const int text_offset = 13;
  const int text_offset_y = 1;
  const int line = lines[lineIndex];

  printAt(thumby, text_offset, line + text_offset_y, str);
}

void freeItemList(ListItem** items, int count) {
  for (int i = 0; i < count; i++) {
      free(items[i]->name);
      free(items[i]->double_name);
      free(items[i]);
  }
  free(items);
}

ListItem** duplicateItemList(ListItem** items, int count) {
  ListItem** dup_items = (ListItem**)malloc(count * sizeof(ListItem*));
  for (int i = 0; i < count; i++) {
    dup_items[i] = createListItemWithCode(items[i]->name, items[i]->icon, items[i]->code);
  }
  return dup_items;
}

ListItem** getLoopItems() {
  return list_items;
}

char* truncateString(const char* original, size_t length_to_copy) {
  if (original == NULL) {return NULL; }

  char* truncated = (char*)malloc((length_to_copy + 1) * sizeof(char));
  strncpy(truncated, original, length_to_copy);
  truncated[length_to_copy] = '\0'; // Null-terminate

  return truncated;
}

void drawItem(Thumby* thumby, ListItem* item, int line, bool is_selected, bool new_selection) {
  drawIcon(thumby, line, item->icon);

  if (item->too_wide_need_scroll) {
    unsigned long now = millis();
    if (new_selection) {
      item->last_move = now + 300;
      item->shift = 0;
    }
    char* truncated;
    if (is_selected) {
      if (item->last_move + 120 < now) {
        item->last_move = now;
        item->shift++;
        if (item->shift > item->name_length) {
          item->shift = 0;
          item->last_move = now + 1000;
        }
      }

      truncated = truncateString(item->double_name + item->shift, max_letters_to_show);
    } else {
      truncated = truncateString(item->name, max_letters_to_show);
    }
    printLineText(thumby, line, truncated);
    free(truncated);
  } else {
    printLineText(thumby, line, item->name);
  }
}

SELECT_BUTTON select_button;
SELECT_BUTTON getSelectButton() {
  return select_button;
}

bool list_down_pressed = false;
bool list_up_pressed = false;
bool list_a_pressed = false;
bool list_b_pressed = false;

int listLoop(Thumby* thumby) {
  return listLoop(thumby, false);
}

int listLoop(Thumby* thumby, bool allow_b_select) {
  return listLoopWithCode(thumby, allow_b_select).selection;
}

SelectionAndCode listLoopWithCode(Thumby* thumby) {
  return listLoopWithCode(thumby, false);
}

SelectionAndCode listLoopWithCode(Thumby* thumby, bool allow_b_select) {
  ListItem* previously_selected = list_items[item_cursor + selection_pos];

  // Up 
  if (thumby->isPressed(BUTTON_U)) {
    list_up_pressed = true;
  } else {
    if (list_up_pressed) {
      selection_pos--;
      if (selection_pos < 0) {
        if (item_cursor > 0) {
          item_cursor--;
          selection_pos = 0;
        } else {
          if (list_item_count > lines_count) {
            item_cursor = list_item_count - lines_count;
            selection_pos = lines_count-1;
          } else {
            item_cursor = 0;
            selection_pos = list_item_count-1;
          }
        }
      }
    }
    list_up_pressed = false;
  }

  // Down
  if (thumby->isPressed(BUTTON_D)) {
    list_down_pressed = true;
  } else {
    if (list_down_pressed) {
      selection_pos++;
      int last_line = list_item_count > lines_count ? lines_count-1 : list_item_count-1;
      if (selection_pos > last_line) {
        if (item_cursor < list_item_count - lines_count) {
          item_cursor++;
          selection_pos = last_line;
        } else {
          item_cursor = 0;
          selection_pos = 0;
        }
      }
    }
    list_down_pressed = false;
  }

  int newly_selected_position = item_cursor + selection_pos;
  ListItem* newly_selected = list_items[newly_selected_position];

  // A (select item)
  if (thumby->isPressed(BUTTON_A)) {
    list_a_pressed = true;
  } else {
    if (list_a_pressed) {
      list_a_pressed = false;
      select_button = SELECT_BUTTON_A;
      return { newly_selected_position, newly_selected->code };
    }
  }

  if (allow_b_select) {
    // B (alt select item)
    if (thumby->isPressed(BUTTON_B)) {
      list_b_pressed = true;
    } else {
      if (list_b_pressed) {
        list_b_pressed = false;
        select_button = SELECT_BUTTON_B;
        return { newly_selected_position, newly_selected->code };
      }
    }
  }

  if (list_items != NULL) {
    for (int i = 0; i < lines_count; i++) {
      int item_index = item_cursor + i;
      if (item_index >= list_item_count) {
        break;
      } else {
        ListItem* item = list_items[item_index];
        if (item != newly_selected) {
          drawItem(thumby, item, i, false, false);
        } else {
          drawItem(thumby, item, i, true, previously_selected != newly_selected);
        }
      }
    }
  }

  drawLineSelectionBorder(thumby, selection_pos);
  if (list_item_count > lines_count) {
    if (item_cursor > 0) {
      drawUpScrollable(thumby);
    }
    if (item_cursor < list_item_count-lines_count) {
      drawDownScrollable(thumby);
    }
  }

  return { -1, -1 };
}

char* createDoubleName(char* name, int name_length) {
  if (name == NULL || name_length <= 0) { return NULL; }
  int double_name_length = name_length * 2 + 1 + 1; // +1 for separator, +1 for the null terminator
  char* double_name = (char*)malloc(double_name_length * sizeof(char));
  snprintf(double_name, double_name_length, "%s/%s", name, name);
  return double_name;
}

ListItem* createListItemWithCode(char* name, int name_length, phraser_Icon_enum_t icon, int code) {
  ListItem* listItem = (ListItem*)malloc(sizeof(ListItem));

  listItem->name = copyString(name, name_length);
  listItem->double_name = createDoubleName(name, name_length);
  listItem->name_length = name_length;
  listItem->too_wide_need_scroll = (name_length > max_letters_to_show);
  listItem->nameCursor = 0;
  listItem->last_move = millis() + 500;
  listItem->icon = icon;
  listItem->shift = 0;
  listItem->code = code;

  return listItem;
}

ListItem* createListItemWithCode(const char* name, phraser_Icon_enum_t icon, int code) {
  return createListItemWithCode((char*)name, (int)strlen(name), icon, code);
}

ListItem* createListItem(char* name, int name_length, phraser_Icon_enum_t icon) {
  return createListItemWithCode(name, name_length, icon, -1);
}

ListItem* createListItem(const char* name, phraser_Icon_enum_t icon) {
  return createListItem((char*)name, (int)strlen(name), icon);
}

void initList(ListItem** new_items, int new_item_count) {
  initList(new_items, new_item_count, 0);
}

void initList(ListItem** new_items, int new_item_count, int selection) {
  if (list_items != NULL) {
    freeItemList(list_items, list_item_count);
  }

  //Reset list view state 
  list_down_pressed = false;
  list_up_pressed = false;
  item_cursor = 0;
  if (selection >= lines_count) {
    item_cursor = (selection - lines_count) + 1;
  }
  selection_pos = selection - item_cursor;

  //Initialize new list items (copy the items)
  list_items = duplicateItemList(new_items, new_item_count);
  list_item_count = new_item_count;
}
