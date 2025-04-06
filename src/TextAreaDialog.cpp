#include "TextAreaDialog.h"

#include "arraylist.h"

TextAreaType text_area_type;
arraylist* text_area_lines = arraylist_create();

typedef struct TextLine TextLine;

void addLinesToList(char* text, int text_length, arraylist* list) {
  int currentLineLength = 0;
  int lineIndex = 0;
  char* currentLine = (char*)malloc((MAX_DISPLAY_CHARS_PER_LINE + 1) * sizeof(char)); // +1 for null terminator

  int cnt = 0;
  for (const char* p = text; *p != '\0' && cnt < text_length; p++) {
    if (*p == '\n') {
      // End the current line and store it
      currentLine[currentLineLength] = '\0';

      arraylist_add(list, strdup(currentLine));

      currentLineLength = 0;
      currentLine[currentLineLength++] = *p;
    } else {
        if (currentLineLength < MAX_DISPLAY_CHARS_PER_LINE) {
            currentLine[currentLineLength++] = *p;
        } else {
          // End the current line and store it
          currentLine[currentLineLength] = '\0';

          arraylist_add(list, strdup(currentLine));

          currentLineLength = 0;
          currentLine[currentLineLength++] = *p;
        }
    }
    cnt++;
  }

  // Store any remaining text in the current line
  if (currentLineLength > 0) {
    currentLine[currentLineLength] = '\0';

    arraylist_add(list, strdup(currentLine));
  }

  //serialDebugPrintf("Init List size %d\r\n", c_list_length(list));

  free(currentLine);
}

void clearList(arraylist* list) {
  if (arraylist_size(list) > 0) {
    for (int i = 0; i < arraylist_size(list); i++) {
      free(arraylist_get(list, i));
    }
    arraylist_clear(list);
  }
}

bool text_area_a_pressed = false;
bool text_area_down_pressed = false;
bool text_area_up_pressed = false;
bool text_area_left_pressed = false;
bool text_area_right_pressed = false;
int text_area_selection_pos = 0;
DialogResult text_area_result;

void initTextAreaDialog(char* text, int text_length, TextAreaType type) {
  text_area_a_pressed = false;
  text_area_down_pressed = false;
  text_area_up_pressed = false;
  text_area_left_pressed = false;
  text_area_right_pressed = false;
  clearList(text_area_lines);
  addLinesToList(text, text_length, text_area_lines);
  text_area_type = type;
  text_area_selection_pos = 0;

  if (type == DLG_OK || type == TEXT_AREA) {
    text_area_result = DLG_RES_OK;
  } else if (type == DLG_YES_NO) {
    text_area_result = DLG_RES_YES;
  }
}

void drawTextAreaBorder(Thumby* thumby) {
  int y2 = text_area_type == TEXT_AREA ? 34 : 26;
  drawRect(thumby, 0, 0, 70, y2, WHITE);

  int text_area_line_count = text_area_type == TEXT_AREA ? TEXT_AREA_LINES : DIALOG_LINES;
  int lines_count = arraylist_size(text_area_lines);

  int positions_count = 1 + lines_count - text_area_line_count;
  if (positions_count > 1) {
    int range = y2 - 2 - 1; 

    int bar_height = range / positions_count;
    int bar_y1 = 2 + range * text_area_selection_pos / positions_count;
    int bar_y2 = bar_y1 + bar_height;

    drawRect(thumby, 69, bar_y1, 71, bar_y2, WHITE);
  }
}

DialogResult textAreaLoop(Thumby* thumby) {
  int text_area_line_count = text_area_type == TEXT_AREA ? TEXT_AREA_LINES : DIALOG_LINES;
  
  // Up 
  if (thumby->isPressed(BUTTON_U)) {
    text_area_up_pressed = true;
  } else {
    if (text_area_up_pressed) {
      if (text_area_selection_pos > 0) {
        text_area_selection_pos--;
      }
    }
    text_area_up_pressed = false;
  }

  // Down
  if (thumby->isPressed(BUTTON_D)) {
    text_area_down_pressed = true;
  } else {
    if (text_area_down_pressed) {
      int lines_count = arraylist_size(text_area_lines);
      int last_line = lines_count - text_area_line_count;
      if (text_area_selection_pos < last_line) {
        text_area_selection_pos++;
      }
    }
    text_area_down_pressed = false;
  }
  
  drawTextAreaBorder(thumby);

  int lines_count = arraylist_size(text_area_lines);
  for (int i = 0; i < text_area_line_count; i++) {
    int position = text_area_selection_pos + i;
    if (position < lines_count) {
      char *tl = (char*)arraylist_get(text_area_lines, position);
      printAt(thumby, 2, 1+8*i, tl);
    }
  }

  if (text_area_type == DLG_OK) {
    // TODO: draw ok button
    printAt(thumby, 30, 30, "OK");
    drawRect(thumby, 26, 29, 44, 39, WHITE);
  } else if (text_area_type == DLG_YES_NO) {
    // Left
    if (thumby->isPressed(BUTTON_L)) {
      text_area_left_pressed = true;
    } else {
      if (text_area_left_pressed) {
        text_area_result = DLG_RES_YES;
      }
      text_area_left_pressed = false;
    }

    // Right
    if (thumby->isPressed(BUTTON_R)) {
      text_area_right_pressed = true;
    } else {
      if (text_area_right_pressed) {
        text_area_result = DLG_RES_NO;
      }
      text_area_right_pressed = false;
    }

    printAt(thumby, 12, 30, "Yes");
    printAt(thumby, 46, 30, "No");

    if (text_area_result == DLG_RES_YES) {
      drawRect(thumby, 10, 29, 29, 39, WHITE);
    } else if (text_area_result == DLG_RES_NO) {
      drawRect(thumby, 41, 29, 60, 39, WHITE);
    }
  }

  // A
  if (thumby->isPressed(BUTTON_A)) {
    text_area_a_pressed = true;
  } else {
    if (text_area_a_pressed) {
      text_area_a_pressed = false;
      return text_area_result;
    }
  }

  return DLG_RES_NONE;
}
