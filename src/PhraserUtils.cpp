#include "PhraserUtils.h"

const uint16_t BANK_BLOCK_COUNT = 128;

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

void drawEmail(Thumby* thumby, int16_t x0, int16_t y0, uint16_t color) {
  x0 = x0+1;
  thumby->drawLine(x0+1, y0+2, x0+1, y0+5, color);
  thumby->drawLine(x0+3, y0, x0+6, y0, color);
  thumby->drawPixel(x0+2, y0+1, color);

  thumby->drawLine(x0+7, y0+1, x0+7, y0+3, color);
  thumby->drawLine(x0+2, y0+6, x0+5, y0+6, color);
  thumby->drawPixel(x0+6, y0+4, color);

  thumby->drawLine(x0+4, y0+2, x0+5, y0+2, color);
  thumby->drawLine(x0+5, y0+2, x0+5, y0+3, color);
  thumby->drawLine(x0+3, y0+3, x0+3, y0+4, color);
  thumby->drawLine(x0+3, y0+4, x0+4, y0+4, color);
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

void readDbBlockFromFlash(uint16_t block_number, void* to_address) {
  uint32_t read_address = XIP_BASE + DB_OFFSET + block_number * FLASH_SECTOR_SIZE;
  memcpy(to_address, (void* )read_address, FLASH_SECTOR_SIZE);
}

void readDbBlockFromFlashBank(uint8_t bank_number, uint16_t block_number, void* to_address) {
  uint16_t raw_block_number = (bank_number-1)*BANK_BLOCK_COUNT + block_number;
  readDbBlockFromFlash(raw_block_number, to_address);
}

void writeDbBlockToFlash(uint16_t block_number, uint8_t* block) {
  uint32_t write_addr = DB_OFFSET + block_number * FLASH_SECTOR_SIZE;

  //Disable interrupts for safe write Flash operation
  uint32_t ints = save_and_disable_interrupts();
  // Erase the sector of the flash
  flash_range_erase(write_addr, FLASH_SECTOR_SIZE);
  // Program block4096 into the sector
  flash_range_program(write_addr, block, FLASH_SECTOR_SIZE);
  //Restore interrupts
  restore_interrupts(ints);
}

void writeDbBlockToFlashBank(uint8_t bank_number, uint16_t block_number, uint8_t* block) {
  uint16_t raw_block_number = (bank_number-1)*BANK_BLOCK_COUNT + block_number;
  writeDbBlockToFlash(raw_block_number, block);
}

void inPlaceDecryptBlock4096(uint8_t* key, uint8_t* iv, uint8_t* block4096) {
  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CBC_decrypt_buffer(&ctx, block4096, FLASH_SECTOR_SIZE-16);//last 16 is IV
}

void inPlaceEncryptBlock4096(uint8_t* key, uint8_t* iv, uint8_t* block4096) {
  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CBC_encrypt_buffer(&ctx, block4096, FLASH_SECTOR_SIZE-16);//last 16 is IV
}

uint8_t* xorByteArrays(uint8_t* array1, uint8_t* array2, size_t length) {
  uint8_t* result = (uint8_t*)malloc(length * sizeof(uint8_t));
  for (size_t i = 0; i < length; i++) {
      result[i] = array1[i] ^ array2[i];
  }

  return result;
}

void uInt32ToBytes(uint32_t value, uint8_t* bytes) {
  bytes[0] = (value >> 24) & 0xFF;
  bytes[1] = (value >> 16) & 0xFF;
  bytes[2] = (value >> 8) & 0xFF;
  bytes[3] = value & 0xFF;
}

uint32_t bytesToUInt32(uint8_t* bytes) {
  uint32_t result = (static_cast<uint32_t>(bytes[0]) << 24) |
                    (static_cast<uint32_t>(bytes[1]) << 16) |
                    (static_cast<uint32_t>(bytes[2]) << 8)  |
                    (static_cast<uint32_t>(bytes[3]));
  return result;
}

uint16_t bytesToUInt16(uint8_t* bytes) {
  uint16_t result = (static_cast<uint16_t>(bytes[0]) << 8)  |
                    (static_cast<uint16_t>(bytes[1]));
  return result;
}

void uInt16ToBytes(uint16_t value, uint8_t* bytes) {
  bytes[0] = (value >> 8) & 0xFF;
  bytes[1] = value & 0xFF;
}

void reverseInPlace(uint8_t* array, size_t length) {
  if (array == NULL || length == 0) {
      return;
  }

  for (size_t start = 0, end = length - 1; start < end; start++, end--) {
      uint8_t temp = array[start];
      array[start] = array[end];
      array[end] = temp;
  }
}

uint8_t* copyBuffer(uint8_t* buffer, uint16_t length) {
  uint8_t* localBuffer = (uint8_t*)malloc(length * sizeof(uint8_t));
  memcpy(localBuffer, buffer, length);
  return localBuffer;
}

char* copyString(char* str, uint16_t length) {
  char* localStr = (char*)malloc((length + 1) * sizeof(char));
  strncpy(localStr, str, length);
  localStr[length] = '\0'; // Null-terminate the string
  return localStr;
}

void generateUniqueNumbers(int N, int count, int* result) {
  if (count > N) {
      printf("Count cannot be greater than N.\n");
      return;
  }

  int i = 0;
  while (i < count) {
      int num = random(N);
      bool unique = true;

      // Check if the number is already in the array
      for (int j = 0; j < i; j++) {
          if (result[j] == num) {
              unique = false;
              break;
          }
      }

      // If the number is unique, add it to the array
      if (unique) {
        result[i] = num;
          i++;
      }
  }
}

uint32_t sha256ToUInt32(uint8_t* sha256_digest) {
  uint8_t* xor16 = xorByteArrays(sha256_digest, sha256_digest+16, 16);
  uint8_t* xor8 = xorByteArrays(xor16, xor16+8, 8);
  uint8_t* xor4 = xorByteArrays(xor8, xor8+4, 4);

  uint32_t seed = bytesToUInt32(xor4);

  free(xor16);
  free(xor8);
  free(xor4);

  return seed;
}

uint32_t random_uint32() {
  uint32_t high = random();
  uint32_t low = random();
  return (high << 1) | (low & 0x1);
}


const uint8_t GENERATEABLE = 1;
const uint8_t TYPEABLE = 2;
const uint8_t VIEWABLE = 4;
const uint8_t USER_EDITABLE = 8;

uint8_t getWordPermissions(bool is_generateable, bool is_user_editable, bool is_typeable, bool is_viewable) {
  uint8_t permissions = 0;
  if (is_generateable) { permissions = permissions | GENERATEABLE; }
  if (is_typeable) { permissions = permissions | TYPEABLE; }
  if (is_viewable) { permissions = permissions | VIEWABLE; }
  if (is_user_editable) { permissions = permissions | USER_EDITABLE; }
  return permissions;
}

bool isGenerateable(uint8_t permissions) {
  return (permissions & GENERATEABLE) != 0;
}

bool isUserEditable(uint8_t permissions) {
  return (permissions & USER_EDITABLE) != 0;
}

bool isTypeable(uint8_t permissions) {
  return (permissions & TYPEABLE) != 0;
}

bool isViewable(uint8_t permissions) {
  return (permissions & VIEWABLE) != 0;
}