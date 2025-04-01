#pragma once

#include <Thumby.h>

#include "PhraserUtils.h"
#include "TextField.h"
#include "Schema_reader.h"

struct ListItem {
  char* name;
  char* double_name;
  phraser_Icon_enum_t icon;
  int name_length;
  bool too_wide_need_scroll;
  int nameCursor;
  unsigned long last_move;
  int shift;
  int code;
};

enum SELECT_BUTTON {
  SELECT_BUTTON_A, 
  SELECT_BUTTON_B
};

struct SelectionAndCode {
  int selection;
  int code;
};

SELECT_BUTTON getSelectButton();
int listLoop(Thumby* thumby);
int listLoop(Thumby* thumby, bool allow_b_select);

SelectionAndCode listLoopWithCode(Thumby* thumby);
SelectionAndCode listLoopWithCode(Thumby* thumby, bool allow_b_select);

void initList(ListItem** new_items, int new_item_count, int selection);
void initList(ListItem** new_items, int new_item_count);

ListItem* createListItem(char* name, int name_length, phraser_Icon_enum_t icon);
ListItem* createListItem(const char* name, phraser_Icon_enum_t icon);
ListItem* createListItemWithCode(char* name, int name_length, phraser_Icon_enum_t icon, int code);
ListItem* createListItemWithCode(const char* name, phraser_Icon_enum_t icon, int code);

void freeItemList(ListItem** items, int count);
