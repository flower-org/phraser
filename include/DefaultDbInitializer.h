#pragma once

#include <Thumby.h>

void initDefaultKeyBlock(uint8_t* out_buffer, const uint8_t* in_aes_key, const uint8_t* in_aes_iv_mask,
  uint8_t* out_new_aes_key, uint8_t* out_new_aes_iv_mask, const uint16_t in_block_count);
void initDefaultSymbolSetsBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask);
void initDefaultFoldersBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask);
void initDefaultPhraseTemplatesBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask);
void initDefaultPhraseBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask);
