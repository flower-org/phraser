#include "ScreenKeyboard.h"

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
char u_pressed = 0, d_pressed = 0, l_pressed = 0, r_pressed = 0, b_pressed = 0, a_pressed = 0;
bool is_emulated_keyboard;

void initOnScreenKeyboard(bool emulated_keyboard, bool password_mode) {
  initTextField(password_mode);
  
  selectedX = 0;
  selectedY = 0;
  selectedCharset = 0;
  is_emulated_keyboard = emulated_keyboard;
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
      selectedCharset = selectedCharset == 3 ? 0 : selectedCharset + 1;
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
        if (is_emulated_keyboard) {
          setChar(c);
          Keyboard.print(c);
        } else {
          appendChar(c);
        }
      }
    }
    a_pressed = false;
  }

  textFieldLoop(thumby);
  return NULL;
}
