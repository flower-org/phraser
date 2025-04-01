#pragma once

#include <Thumby.h>

enum UpdateResponse {
  DB_FULL,
  BLOCK_SIZE_EXCEEDED,
  OK,
  ERROR
};

bool loadBlockFromFlash(uint8_t bank_number, uint16_t block_number, uint32_t block_size, 
  uint8_t* aes_key, uint8_t* aes_iv_mask, 
  uint8_t *out_db_block);

UpdateResponse updateVersionAndEntropyBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask);
