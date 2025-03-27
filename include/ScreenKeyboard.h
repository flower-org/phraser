#pragma once

#include <Thumby.h>
#include <Keyboard.h>

#include "PhraserUtils.h"
#include "TextField.h"

extern const char* KB_B_PRESSED;
extern const char* KB_A_PRESSED;

void initOnScreenKeyboard(bool emulated_keyboard, bool password_mode);
void initOnScreenKeyboard(bool emulated_keyboard, bool password_mode, bool integer_mode);
void initOnScreenKeyboard(char* init_text, int init_text_length, bool emulated_keyboard, bool password_mode, bool integer_mode);
void initOnScreenKeyboard(char* init_text, int init_text_length, bool emulated_keyboard, bool password_mode, bool integer_mode, 
  int16_t selectedXPrm, int16_t selectedYPrm);

char* keyboardLoop(Thumby* thumby);
char* specialKeyboardLoop(Thumby* thumby);
