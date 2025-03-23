#include "ScreenKeyboard.h"

const char* KB_B_PRESSED = "B";

void drawEnter(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, char isSelected) {
  if (isSelected) {
    drawRect(thumby, x0-2, y0-2, x0+7, y0+11, WHITE);
  }

  //Enter sign
  thumby->drawLine(x0, y0, x0, y0+7, WHITE);
  thumby->drawLine(x0, y0+7, x0+5, y0+7, WHITE);
  thumby->drawLine(x0+3, y0+9, x0+5, y0+7, WHITE);
  thumby->drawLine(x0+3, y0+5, x0+5, y0+7, WHITE);
}

void drawBackspace(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, char isSelected) {
  if (isSelected) {
    drawRect(thumby, x0-2, y0-1, x0+6, y0+9, color);
  }

  //Backspace sign
  thumby->drawLine(x0, y0+4, x0+2, y0+2, color);
  thumby->drawLine(x0, y0+4, x0+2, y0+6, color);
  thumby->drawLine(x0, y0+4, x0+4, y0+4, color);
}

const char* capitalLetters = "QWERTYUIOPASDFGHJKL\\ZXCVBNM ,.";
const char* smallLetters = "qwertyuiopasdfghjkl\\zxcvbnm ,.";
const char* numbers = "1234567890_@#$&-+()/*\"':;!? <>";
const char* specialCharacters = "~`|^={}[]\\%@#$&-+()/*\"':;!? <>";

const char* charsets[4] = { capitalLetters, smallLetters, numbers, specialCharacters };

void drawKeyboard(Thumby* thumby, const char* characters, int16_t selectedX, int16_t selectedY) {
  //Backspace key
  drawBackspace(thumby, 64, 15, WHITE, selectedX == 10 && selectedY == 0);

  //Enter key
  drawEnter(thumby, 64, 27, WHITE, selectedX == 10 && selectedY > 0);

  //Character keys
  for (int i = 0; i < 10; i++) {
    //Row 1
    drawLetter(thumby, characters[i], 3 + i*6, 15, WHITE, selectedY == 0 && selectedX == i);
    //Row 2
    drawLetter(thumby, characters[i+10], 3 + i*6, 23, WHITE, selectedY == 1 && selectedX == i);
    //Row 3
    drawLetter(thumby, characters[i+20], 3 + i*6, 31, WHITE, selectedY == 2 && selectedX == i);
  }
}

int16_t selectedX = 0, selectedY = 0;
char selectedCharset = 0;
bool u_pressed = false, d_pressed = false, l_pressed = false, r_pressed = false, b_pressed = false, a_pressed = false;
bool is_emulated_keyboard;
bool special_activated = false;
bool int_mode = false;

void initOnScreenKeyboard(bool emulated_keyboard, bool password_mode) {
  initOnScreenKeyboard(emulated_keyboard, password_mode, false);
}

void initOnScreenKeyboard(bool emulated_keyboard, bool password_mode, bool integer_mode) {
  initTextField(password_mode);
  
  int_mode = integer_mode;
  selectedX = 0;
  selectedY = 0;
  selectedCharset = integer_mode ? 2 : 0;
  u_pressed = false; d_pressed = false; l_pressed = false; r_pressed = false; b_pressed = false; a_pressed = false;
  is_emulated_keyboard = emulated_keyboard;
  special_activated = false;
}

void initOnScreenKeyboard(char* init_text, int init_text_length, bool emulated_keyboard, bool password_mode, bool integer_mode) {
  initTextField(init_text, init_text_length, password_mode);
  
  int_mode = integer_mode;
  selectedX = 0;
  selectedY = 0;
  selectedCharset = integer_mode ? 2 : 0;
  u_pressed = false; d_pressed = false; l_pressed = false; r_pressed = false; b_pressed = false; a_pressed = false;
  is_emulated_keyboard = emulated_keyboard;
  special_activated = false;
}

char* specialKeyboardLoop(Thumby* thumby) {
  char* originalResult = keyboardLoop(thumby);
  if (u_pressed && a_pressed && b_pressed) {
    special_activated = true;
  }
  if (special_activated) {
    if (!u_pressed && !a_pressed && !b_pressed) {
      special_activated = false;
      return (char*)KB_B_PRESSED;
    }
  }
  return originalResult;
}

char* keyboardLoop(Thumby* thumby) {
  drawKeyboard(thumby, charsets[selectedCharset], selectedX, selectedY);

  if (thumby->isPressed(BUTTON_L)) {
    l_pressed = true;
  } else {
    if (l_pressed) {
      selectedX = selectedX == 0 ? 10 : selectedX-1;
    }
    l_pressed = false;
  }
  
  if (thumby->isPressed(BUTTON_R)) {
    r_pressed = true;
  } else {
    if (r_pressed) {
      selectedX = selectedX == 10 ? 0 : selectedX+1;
    }
    r_pressed = false;
  }
  
  if (thumby->isPressed(BUTTON_U)) {
    u_pressed = true;
  } else {
    if (u_pressed) {
      if (selectedX == 10) {
        selectedY = selectedY == 0 ? 2 : 0;
      } else {
        selectedY = selectedY == 0 ? 2 : selectedY-1;
      }
    }
    u_pressed = false;
  }
  
  if (thumby->isPressed(BUTTON_D)) {
    d_pressed = true;
  } else {
    if (d_pressed) {
      if (selectedX == 10) {
        selectedY = (selectedY == 2 || selectedY == 1) ? 0 : 1;
      } else {
        selectedY = selectedY == 2 ? 0 : selectedY+1;
      }
    }
    d_pressed = false;
  }
  
  //Left button
  if (thumby->isPressed(BUTTON_B)) {
    b_pressed = true;
  } else {
    if (b_pressed) {
      if (int_mode) {
        selectedCharset = 2;
      } else {
        selectedCharset = selectedCharset == 3 ? 0 : selectedCharset + 1;
      }
    }
    b_pressed = false;
  }

  //Right button
  if (thumby->isPressed(BUTTON_A)) {
    a_pressed = true;
  } else {
    if (a_pressed) {
      if (selectedX == 10) {
        if (selectedY == 0) {
          if (is_emulated_keyboard) {
            Keyboard.press(KEY_BACKSPACE);
            setChar(' ');
          } else {
            deleteLastChar();
          }
        } else {
          if (is_emulated_keyboard) {
            setChar(' ');
            Keyboard.press(KEY_RETURN);
          } else {
            a_pressed = false;
            //return entered string
            return getTextFieldText();
          }
        }
        delay(50);
        if (is_emulated_keyboard) {
          Keyboard.releaseAll();
        }
      } else {
        char c = charsets[selectedCharset][selectedX + selectedY*10];
        boolean allowed = true;
        if (int_mode) {
          if (c < '0' || c > '9') {
            allowed = false;
          }
        } 

        if (allowed) {
          if (is_emulated_keyboard) {
            setChar(c);
            Keyboard.print(c);
          } else {
            appendChar(c);
          }
        }
      }
    }
    a_pressed = false;
  }

  textFieldLoop(thumby);
  return NULL;
}
