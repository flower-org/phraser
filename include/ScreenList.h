#pragma once

#include <Thumby.h>

#include "PhraserUtils.h"
#include "TextField.h"
#include "Schema_generated.h"

struct ListItem {
  char* name;
  char* double_name;
  int name_length;
  bool too_wide_need_scroll;
  int nameCursor;
  u_int64_t last_move;
};

ListItem* listLoop(Thumby* thumby);
