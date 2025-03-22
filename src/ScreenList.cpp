#include "ScreenList.h"

const int max_letters_to_show = 10;
const int lines_count = 4;
const int lines[lines_count] = { 0, 10, 20, 30 };

ListItem** items = NULL;
int item_count;
int item_cursor;
int selection_pos;

void drawIcon(Thumby* thumby, int lineIndex, phraser::Icon icon) {
  const int icon_offset_x = 1;
  const int icon_offset_y = 2;
  const int line = lines[lineIndex];

  switch (icon) {
    case phraser::Icon_Key: 
      drawKey(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Login: 
      drawLogin(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Asterisk: 
      drawAsterisk(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Lock: 
      drawLock(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Aa: 
      drawAa(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Star: 
      drawStar(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Settings: 
      drawSettings(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Folder: 
      drawFolderIcon(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_ToParentFolder: 
      drawToParentFolder(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_LookingGlass: 
      drawLookingGlass(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_LTTriangle: 
      drawLTTriangle(thumby, icon_offset_x, line+icon_offset_y, WHITE, false);
      break;
    case phraser::Icon_GTTriangle: 
      drawGTTriangle(thumby, icon_offset_x, line+icon_offset_y, WHITE, false);
      break;
    case phraser::Icon_TextOut: 
      drawTextOut(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Ledger: 
      drawLedger(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_PlusMinus: 
      drawPlusMinus(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Stars: 
      drawStars(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Message: 
      drawMessage(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Quote: 
      drawQuote(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Question: 
      drawQuestion(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Plus: 
      drawPlus(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Minus: 
      drawMinus(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_X: 
      drawX(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Check: 
      drawCheck(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Copy: 
      drawCopy(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Download: 
      drawDownload(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Upload: 
      drawUpload(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Skull: 
      drawSkull(thumby, icon_offset_x, line+icon_offset_y, WHITE);
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

void freeList(ListItem** items, int count) {
  for (int i = 0; i < count; i++) {
      free(items[i]->name);
      free(items[i]->double_name);
      free(items[i]);
  }
  free(items);
}

ListItem** getLoopItems() {
  return items;
}

void drawItem(Thumby* thumby, ListItem* item, int line) {
  drawIcon(thumby, line, item->icon);

  // TODO: scroll text horizontally if it's too long
  printLineText(thumby, line, item->name);
}

bool down_pressed = false;
bool up_pressed = false;
ListItem* listLoop(Thumby* thumby) {
  if (thumby->isPressed(BUTTON_U)) {
    up_pressed = true;
  } else {
    if (up_pressed) {
      selection_pos--;
      if (selection_pos < 0) {
        if (item_cursor > 0) {
          item_cursor--;
          selection_pos = 0;
        } else {
          if (item_count > lines_count) {
            item_cursor = item_count - lines_count;
            selection_pos = lines_count-1;
          } else {
            item_cursor = 0;
            selection_pos = item_count-1;
          }
        }
      }
    }
    up_pressed = false;
  }

  if (thumby->isPressed(BUTTON_D)) {
    down_pressed = true;
  } else {
    if (down_pressed) {
      selection_pos++;
      int last_line = item_count > lines_count ? lines_count-1 : item_count-1;
      if (selection_pos > last_line) {
        if (item_cursor < item_count - lines_count) {
          item_cursor++;
          selection_pos = last_line;
        } else {
          item_cursor = 0;
          selection_pos = 0;
        }
      }
    }
    down_pressed = false;
  }

  if (items != NULL) {
    for (int i = 0; i < lines_count; i++) {
      int item_index = item_cursor + i;
      if (item_index >= item_count) {
        break;
      } else {
        ListItem* item = items[item_index];
        drawItem(thumby, item, i);
      }
    }
  }

  drawLineSelectionBorder(thumby, selection_pos);
  if (item_count > lines_count) {
    if (item_cursor > 0) {
      drawUpScrollable(thumby);
    }
    if (item_cursor < item_count-lines_count) {
      drawDownScrollable(thumby);
    }
  }

  return NULL;
}

char* createDoubleName(char* name, int name_length) {
  if (name == NULL || name_length <= 0) { return NULL; }
  int double_name_length = name_length * 2 + 1; // +1 for the null terminator
  char* double_name = (char*)malloc(double_name_length * sizeof(char));
  snprintf(double_name, double_name_length, "%s%s", name, name);
  return double_name;
}

ListItem* createListItem(char* name, int name_length, phraser::Icon icon) {
  ListItem* listItem = (ListItem*)malloc(sizeof(ListItem));

  listItem->name = strdup(name);
  listItem->double_name = createDoubleName(name, name_length);
  listItem->name_length = name_length;
  listItem->too_wide_need_scroll = (name_length > max_letters_to_show);
  listItem->nameCursor = 0;
  listItem->last_move = 0;
  listItem->icon = icon;

  return listItem;
}

ListItem* createListItem(const char* name, phraser::Icon icon) {
  return createListItem((char*)name, (int)strlen(name), icon);
}

void initList(ListItem** new_items, int new_item_count) {
  //Reset list view state 
  down_pressed = false;
  up_pressed = false;
  item_cursor = 0;
  selection_pos = 0;
  
  //Initialize new list items
  items = new_items;
  item_count = new_item_count;
}
