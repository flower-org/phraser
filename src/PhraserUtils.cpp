#include "PhraserUtils.h"

void drawRect(Thumby* thumby, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  thumby->drawLine(x0, y0, x0, y1, color);
  thumby->drawLine(x0, y0, x1, y0, color);
  thumby->drawLine(x0, y1, x1, y1, color);
  thumby->drawLine(x1, y0, x1, y1, color);
}

void drawSelectedRect(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, byte isSelected) {
  if (isSelected) {
    drawRect(thumby, x0-2, y0-1, x0+6, y0+9, color);
  }
}

void drawThreeDots(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, byte isSelected) {
  drawSelectedRect(thumby, x0, y0, color, isSelected);
  
  thumby->drawPixel(x0, y0+7, color);
  thumby->drawPixel(x0+2, y0+7, color);
  thumby->drawPixel(x0+4, y0+7, color);
}

void drawSpace(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, byte isSelected) {
  drawSelectedRect(thumby, x0, y0, color, isSelected);
  
  thumby->drawLine(x0, y0+7, x0+4, y0+7, color);
  thumby->drawLine(x0, y0+7, x0, y0+6, color);
  thumby->drawLine(x0+4, y0+7, x0+4, y0+6, color);
}

void drawLetter(Thumby* thumby, char letter, int16_t x0, int16_t y0, uint16_t color, byte isSelected) {
  if (letter == ' ') {
    drawSpace(thumby, x0, y0, color, isSelected);
  } else {
    drawSelectedRect(thumby, x0, y0, color, isSelected);

    thumby->goTo(x0, y0);
    thumby->write(letter);
  }
}

void drawLTTriangle(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, byte isSelected) {
  drawSelectedRect(thumby, x0, y0, color, isSelected);
  
  thumby->drawLine(x0+1, y0+4, x0+3, y0+2, color);
  thumby->drawLine(x0+1, y0+4, x0+3, y0+6, color);
  thumby->drawLine(x0+3, y0+2, x0+3, y0+6, color);
  thumby->drawPixel(x0+2, y0+4, color);
}

void drawGTTriangle(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, byte isSelected) {
  drawSelectedRect(thumby, x0, y0, color, isSelected);
  
  thumby->drawLine(x0+1, y0+2, x0+3, y0+4, color);
  thumby->drawLine(x0+3, y0+4, x0+1, y0+6, color);
  thumby->drawLine(x0+1, y0+2, x0+1, y0+6, color);
  thumby->drawPixel(x0+2, y0+4, color);
}

void drawFolderIcon(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0, y0, x0, y0+6, color);
  thumby->drawLine(x0, y0, x0+4, y0, color);
  thumby->drawLine(x0+4, y0, x0+4, y0+2, color);
  thumby->drawLine(x0, y0+2, x0+8, y0+2, color);
  thumby->drawLine(x0, y0+6, x0+8, y0+6, color);
  thumby->drawLine(x0+8, y0+2, x0+8, y0+6, color);
}

void drawToParentFolder(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0, y0+2, x0+2, y0, color);
  thumby->drawLine(x0+2, y0, x0+4, y0+2, color);
  thumby->drawLine(x0+2, y0, x0+2, y0+6, color);
  thumby->drawLine(x0+2, y0+6, x0+8, y0+6, color);
}

void drawAsterisk(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+2, y0+1, x0+4, y0+3, color);
  thumby->drawLine(x0+2, y0+4, x0+4, y0+2, color);
  thumby->drawLine(x0+6, y0+1, x0+4, y0+3, color);
  thumby->drawLine(x0+6, y0+4, x0+4, y0+2, color);
  thumby->drawLine(x0+4, y0, x0+4, y0+5, color);
}

void drawKey(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+1, y0, x0+3, y0, color);
  thumby->drawLine(x0+1, y0+4, x0+3, y0+4, color);
  thumby->drawLine(x0, y0+1, x0, y0+3, color);
  thumby->drawLine(x0+4, y0+1, x0+4, y0+3, color);
  thumby->drawLine(x0+4, y0+2, x0+8, y0+2, color);
  thumby->drawLine(x0+8, y0+2, x0+8, y0+4, color);
  thumby->drawLine(x0+6, y0+2, x0+6, y0+4, color);
}

void drawSettings(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawPixel(x0+2, y0, color);
  thumby->drawPixel(x0, y0+2, color);
  thumby->drawLine(x0+1, y0+3, x0+3, y0+1, color);
  thumby->drawLine(x0+2, y0+2, x0+6, y0+6, color);
  thumby->drawLine(x0+2, y0+6, x0+8, y0, color);
  thumby->drawLine(x0+8, y0, x0+6, y0, color);
  thumby->drawLine(x0+8, y0, x0+8, y0+2, color);
}

void drawLock(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  drawRect(thumby, x0+3, y0, x0+6, y0+6, color);
  drawRect(thumby, x0+2, y0+3, x0+7, y0+6, color);
  thumby->drawLine(x0+2, y0+6, x0+7, y0+6, color);
}

void drawLogin(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0, y0+3, x0+5, y0+3, color);
  thumby->drawLine(x0+5, y0+3, x0+3, y0+1, color);
  thumby->drawLine(x0+5, y0+3, x0+3, y0+5, color);
  thumby->drawLine(x0+5, y0, x0+8, y0, color);
  thumby->drawLine(x0+5, y0+6, x0+8, y0+6, color);
  thumby->drawLine(x0+8, y0, x0+8, y0+6, color);
}

void drawStar(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  drawRect(thumby, x0+3, y0+1, x0+5, y0+4, color);
  drawRect(thumby, x0+2, y0+2, x0+6, y0+3, color);
  thumby->drawPixel(x0+4, y0, color);
  thumby->drawPixel(x0+1, y0+2, color);
  thumby->drawPixel(x0+7, y0+2, color);
  thumby->drawPixel(x0+2, y0+5, color);
  thumby->drawPixel(x0+6, y0+5, color);
}

void drawLookingGlass(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+4, y0, x0+6, y0, color);
  thumby->drawLine(x0+7, y0+1, x0+7, y0+3, color);
  thumby->drawLine(x0+3, y0+1, x0+3, y0+5, color);
  thumby->drawLine(x0+2, y0+4, x0+6, y0+4, color);
  drawRect(thumby, x0+1, y0+5, x0+2, y0+6, color);
}

void drawAa(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0, y0+5, x0, y0+6, color);
  thumby->drawLine(x0+1, y0+3, x0+1, y0+4, color);
  thumby->drawLine(x0+2, y0+1, x0+2, y0+2, color);
  thumby->drawLine(x0+3, y0, x0+3, y0+6, color);
  thumby->drawLine(x0+1, y0+4, x0+3, y0+4, color);

  thumby->drawLine(x0+5, y0+2, x0+6, y0+2, color);
  thumby->drawLine(x0+5, y0+4, x0+5, y0+6, color);
  thumby->drawLine(x0+5, y0+4, x0+7, y0+4, color);
  thumby->drawLine(x0+7, y0+3, x0+7, y0+5, color);
  thumby->drawLine(x0+5, y0+6, x0+6, y0+6, color);
  thumby->drawPixel(x0+8, y0+6, color);
}

void drawTextOut(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0, y0, x0, y0+1, color);
  thumby->drawLine(x0, y0, x0+4, y0, color);
  thumby->drawLine(x0+4, y0, x0+4, y0+1, color);
  thumby->drawLine(x0+2, y0, x0+2, y0+5, color);
  thumby->drawLine(x0+1, y0+5, x0+3, y0+5, color);

  thumby->drawLine(x0+4, y0+3, x0+8, y0+3, color);
  thumby->drawLine(x0+8, y0+3, x0+6, y0+1, color);
  thumby->drawLine(x0+8, y0+3, x0+6, y0+5, color);
}

void drawLedger(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+3, y0, x0+7, y0, color);
  thumby->drawLine(x0+1, y0+2, x0+1, y0+6, color);
  thumby->drawLine(x0+1, y0+6, x0+7, y0+6, color);
  thumby->drawLine(x0+7, y0, x0+7, y0+6, color);
  thumby->drawPixel(x0+2, y0+1, color);

  thumby->drawLine(x0+3, y0+2, x0+5, y0+2, color);
  thumby->drawLine(x0+3, y0+4, x0+5, y0+4, color);
}

void drawPlusMinus(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0, y0+3, x0+2, y0+3, color);
  thumby->drawLine(x0+1, y0+2, x0+1, y0+4, color);
  thumby->drawLine(x0+6, y0+3, x0+8, y0+3, color);
  thumby->drawLine(x0+5, y0, x0+5, y0+1, color);
  thumby->drawLine(x0+4, y0+2, x0+4, y0+4, color);
  thumby->drawLine(x0+3, y0+5, x0+3, y0+6, color);
}

void drawStars(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawPixel(x0+3, y0+0, color);
  thumby->drawPixel(x0+2, y0+1, color);
  thumby->drawPixel(x0+4, y0+1, color);
  thumby->drawPixel(x0+3, y0+2, color);

  thumby->drawPixel(x0+6, y0+3, color);
  thumby->drawPixel(x0+5, y0+4, color);
  thumby->drawPixel(x0+7, y0+4, color);
  thumby->drawPixel(x0+6, y0+5, color);

  thumby->drawPixel(x0+1, y0+4, color);
  thumby->drawPixel(x0+0, y0+5, color);
  thumby->drawPixel(x0+2, y0+5, color);
  thumby->drawPixel(x0+1, y0+6, color);
}

void drawMessage(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  drawRect(thumby, x0, y0, x0+8, y0+4, color);
  thumby->drawLine(x0+2, y0+2, x0+6, y0+2, color);
  thumby->drawLine(x0+1, y0+4, x0+1, y0+6, color);
  thumby->drawLine(x0+2, y0+4, x0+2, y0+5, color);
}

void drawQuote(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  drawRect(thumby, x0+2, y0+1, x0+3, y0+2, color);
  drawRect(thumby, x0+5, y0+1, x0+6, y0+2, color);
  thumby->drawLine(x0+3, y0+3, x0+2, y0+4, color);
  thumby->drawLine(x0+5, y0+4, x0+6, y0+3, color);
}

void drawQuestion(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawPixel(x0+2, y0+1, color);
  thumby->drawPixel(x0+4, y0+6, color);
  thumby->drawLine(x0+4, y0+4, x0+6, y0+2, color);
  thumby->drawPixel(x0+6, y0+1, color);
  thumby->drawLine(x0+3, y0, x0+5, y0, color);
}

void drawPlus(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+4, y0+1, x0+4, y0+5, color);
  thumby->drawLine(x0+2, y0+3, x0+6, y0+3, color);
}

void drawMinus(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+2, y0+3, x0+6, y0+3, color);
}

void drawX(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+2, y0+1, x0+6, y0+5, color);
  thumby->drawLine(x0+6, y0+1, x0+2, y0+5, color);
}

void drawCheck(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+1, y0+3, x0+3, y0+5, color);
  thumby->drawLine(x0+3, y0+5, x0+7, y0+1, color);
}

void drawCopy(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+2, y0, x0+5, y0, color);
  thumby->drawLine(x0+4, y0+1, x0+7, y0+1, color);
  thumby->drawLine(x0+1, y0+1, x0+1, y0+5, color);
  thumby->drawLine(x0+3, y0+2, x0+3, y0+6, color);
  thumby->drawLine(x0+1, y0+5, x0+2, y0+5, color);
  thumby->drawLine(x0+7, y0+1, x0+7, y0+6, color);
  thumby->drawLine(x0+3, y0+6, x0+7, y0+6, color);
}

void drawUpload(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+1, y0, x0+7, y0, color);
  thumby->drawLine(x0+1, y0, x0+1, y0+1, color);
  thumby->drawLine(x0+7, y0, x0+7, y0+1, color);

  thumby->drawLine(x0+2, y0+4, x0+4, y0+2, color);
  thumby->drawLine(x0+6, y0+4, x0+4, y0+2, color);
  thumby->drawLine(x0+4, y0+2, x0+4, y0+6, color);
}

void drawDownload(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  thumby->drawLine(x0+1, y0+6, x0+7, y0+6, color);
  thumby->drawLine(x0+1, y0+6, x0+1, y0+5, color);
  thumby->drawLine(x0+7, y0+6, x0+7, y0+5, color);

  thumby->drawLine(x0+2, y0+2, x0+4, y0+4, color);
  thumby->drawLine(x0+6, y0+2, x0+4, y0+4, color);
  thumby->drawLine(x0+4, y0+4, x0+4, y0, color);
}

void drawSkull(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  x0 = x0+1;
  thumby->drawLine(x0+1, y0, x0+5, y0, color);
  thumby->drawLine(x0, y0+1, x0+6, y0+1, color);
  drawRect(thumby, x0, y0+3, x0+2, y0+4, color);
  drawRect(thumby, x0+4, y0+3, x0+6, y0+4, color);

  thumby->drawPixel(x0, y0+2, color);
  thumby->drawPixel(x0+6, y0+2, color);
  thumby->drawLine(x0+3, y0+2, x0+3, y0+3, color);
  thumby->drawLine(x0+1, y0+5, x0+5, y0+5, color);
  thumby->drawPixel(x0+1, y0+6, color);
  thumby->drawPixel(x0+3, y0+6, color);
  thumby->drawPixel(x0+5, y0+6, color);
}

void drawMessage(Thumby* thumby, const char* str) {
  printAt(thumby, 0, 0, (char*)str);
}

void drawTurnOffMessage(Thumby* thumby, int x, int y) {
  printAt(thumby, x+3, y, "It's now safe");
  printAt(thumby, x+8, y+8, "to turn off");
  printAt(thumby, x+2, y+16, "your computer.");
}

void drawTurnOffMessage(Thumby* thumby) {
  drawTurnOffMessage(thumby, 0, 7);
}

void printAt(Thumby* thumby, int x, int y, char* str) {
  thumby->setCursor(x, y);
  thumby->print(str);
}

char* bytesToHexString(const unsigned char* bytes, size_t length) {
  // Allocate memory for the hex string (2 characters per byte + 1 for null terminator)
  char* hexString = (char*)malloc(length * 2 + 1);
  if (hexString == NULL) {
      return NULL; // Memory allocation failed
  }

  // Convert each byte to two hex characters
  for (size_t i = 0; i < length; i++) {
      sprintf(hexString + (i * 2), "%02X", bytes[i]);
  }

  // Null-terminate the string
  hexString[length * 2] = '\0';
  return hexString;
}
