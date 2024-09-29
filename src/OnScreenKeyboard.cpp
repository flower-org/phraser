#include "OnScreenKeyboard.h"

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

void initOnScreenKeyboard() {
  initTextField();
  
  selectedX = 0;
  selectedY = 0;
  selectedCharset = 0;
}

void keyboardLoop(Thumby* thumby) {
/*  drawKeyboard(thumby, charsets[selectedCharset], selectedX, selectedY);

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
          //Keyboard.press(KEY_BACKSPACE);
          deleteLastChar();
        } else {
          //Keyboard.press(KEY_RETURN);
        }
        delay(50);
        //Keyboard.releaseAll();
      } else {
        char c = charsets[selectedCharset][selectedX + selectedY*10];
        appendChar(c);
        //Keyboard.print(c);
      }
    }
    a_pressed = false;
  }

  textFieldLoop(thumby);
  */

  drawFolderIcon(thumby, 3+12*0, 3, WHITE);
  drawToParentFolder(thumby, 3+12*1, 3, WHITE);
  drawAsterisk(thumby, 3+12*2, 3, WHITE);
  drawKey(thumby, 3+12*3, 3, WHITE);
  drawAa(thumby, 3+12*4, 3, WHITE);
  drawLTTriangle(thumby, 3+12*5, 3, WHITE, false);
  
  drawSettings(thumby, 3+12*0, 13, WHITE);
  drawLock(thumby, 3+12*1, 13, WHITE);
  drawLogin(thumby, 3+12*2, 13, WHITE);
  drawStar(thumby, 3+12*3, 13, WHITE);
  drawLookingGlass(thumby, 3+12*4, 13, WHITE);
  drawGTTriangle(thumby, 3+12*5, 13, WHITE, false);

  drawTextOut(thumby, 3+12*0, 23, WHITE);
  drawLedger(thumby, 3+12*1, 23, WHITE);
  drawPlusMinus(thumby, 3+12*2, 23, WHITE);
  drawStars(thumby, 3+12*3, 23, WHITE);
  drawMessage(thumby, 3+12*4, 23, WHITE);
  drawQuote(thumby, 3+12*5, 23, WHITE);
  
  drawQuestion(thumby, 3+12*0, 33, WHITE);
  drawPlus(thumby, 3+12*1, 33, WHITE);
  drawMinus(thumby, 3+12*2, 33, WHITE);
  drawX(thumby, 3+12*3, 33, WHITE);
  drawCheck(thumby, 3+12*4, 33, WHITE);
  drawCopy(thumby, 3+12*5, 33, WHITE);
}
