#pragma once

#include <Thumby.h>
#include <Arduino.h>

#include "PhraserUtils.h"

typedef enum {
  TEXT_AREA,
  DLG_OK,
  DLG_YES_NO
} TextAreaType;

typedef enum {
  DLG_RES_NONE,
  DLG_RES_OK,
  DLG_RES_YES,
  DLG_RES_NO
} DialogResult;

const int MAX_DISPLAY_CHARS_PER_LINE = 13;
const int TEXT_AREA_LINES = 4;
const int DIALOG_LINES = 3;

void initTextAreaDialog(char* text, int text_length, TextAreaType type);
DialogResult textAreaLoop(Thumby* thumby);
