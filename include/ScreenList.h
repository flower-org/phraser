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
  void* data;
};

int listLoop(Thumby* thumby);
ListItem* listLoopItem(Thumby* thumby);

void initList(ListItem** new_items, int new_item_count, int selection);
void initList(ListItem** new_items, int new_item_count);

ListItem* createListItem(char* name, int name_length, phraser_Icon_enum_t icon);
ListItem* createListItem(const char* name, phraser_Icon_enum_t icon);

ListItem* createListItem(char* name, int name_length, phraser_Icon_enum_t icon, void* data);
ListItem* createListItem(const char* name, phraser_Icon_enum_t icon, void* data);

void freeItemList(ListItem** items, int count);
