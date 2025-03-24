#pragma once

extern "C" {
  #include <hardware/sync.h>
  #include <hardware/flash.h>
  #include <string.h>
};

#define BLACK 0
#define WHITE 1

//0.5 mb mark
#define DB_OFFSET (1024*512)

#include <Thumby.h>

void drawRect(Thumby* thumby, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void drawLetter(Thumby* thumby, char letter, int16_t x0, int16_t y0, uint16_t color, byte isSelected);
void drawThreeDots(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, byte isSelected);

void drawLTTriangle(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, byte isSelected);
void drawGTTriangle(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, byte isSelected);

void drawFolderIcon(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawToParentFolder(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawAsterisk(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawKey(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawSettings(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawLock(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawLogin(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawStar(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawLookingGlass(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawAa(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawTextOut(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawLedger(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawPlusMinus(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawStars(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawMessage(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawQuote(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);

void drawQuestion(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawPlus(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawMinus(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawX(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawCheck(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawCopy(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawUpload(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawDownload(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawSkull(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);
void drawEmail(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color);

void drawMessage(Thumby* thumby, const char* str);
void printAt(Thumby* thumby, int x, int y, char* str);
void drawTurnOffMessage(Thumby* thumby);

void drawSpace(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color, byte isSelected);
char* bytesToHexString(const unsigned char* bytes, size_t length);

void readDbBlockFromFlash(uint16_t block_number, void* to_address);
void writeDbBlockToFlash(uint16_t block_number, uint8_t* block);
