#pragma once

#include <Thumby.h>

#include "PhraserUtils.h"
#include "TextField.h"
#include "Schema_generated.h"

struct ListItem {
  char* name;
  char* double_name;
  phraser::Icon icon;
  int name_length;
  bool too_wide_need_scroll;
  int nameCursor;
  unsigned long last_move;
  int shift;
};

ListItem* listLoop(Thumby* thumby);
void initList(ListItem** new_items, int new_item_count);

ListItem* createListItem(char* name, int name_length, phraser::Icon icon);
ListItem* createListItem(const char* name, phraser::Icon icon);
