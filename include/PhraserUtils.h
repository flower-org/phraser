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
#include "aes.hpp"
#include "rbtree.h"

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

void readDbBlockFromFlashBank(uint8_t bank_number, uint16_t block_number, void* to_address);
void writeDbBlockToFlashBank(uint8_t bank_number, uint16_t block_number, uint8_t* block);

void inPlaceDecryptBlock4096(uint8_t* key, uint8_t* iv, uint8_t* block4096);
void inPlaceEncryptBlock4096(uint8_t* key, uint8_t* iv, uint8_t* block4096);

uint8_t* xorByteArrays(uint8_t* array1, uint8_t* array2, size_t length);
void uInt32ToBytes(uint32_t value, uint8_t* bytes);
uint32_t bytesToUInt32(uint8_t* bytes);
void reverseInPlace(uint8_t* array, size_t length);
uint16_t bytesToUInt16(uint8_t* bytes);
void uInt16ToBytes(uint16_t value, uint8_t* bytes);
uint8_t* copyBuffer(uint8_t* buffer, uint16_t length);
char* copyString(char* str, uint16_t length);
void generateUniqueNumbers(int N, int count, int* result);
uint32_t sha256ToUInt32(uint8_t* sha256_digest);

uint32_t random_uint32();

uint8_t getWordPermissions(bool is_generateable, bool is_user_editable, bool is_typeable, bool is_viewable);
bool isGenerateable(uint8_t permissions);
bool isUserEditable(uint8_t permissions);
bool isTypeable(uint8_t permissions);
bool isViewable(uint8_t permissions);

uint32_t get_valid_block_number_on_the_right_of(node_t* root, uint32_t block_number);
uint32_t get_free_block_number_on_the_left_of(node_t* root, uint32_t block_number, uint32_t db_block_count);
