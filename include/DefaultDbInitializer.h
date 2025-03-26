#pragma once

#include <Thumby.h>

void initDefaultKeyBlock(uint8_t* buffer, uint8_t* aes_key, uint8_t* aes_iv_mask, uint8_t* new_aes_key, uint8_t* new_aes_iv_mask);
void initDefaultSymbolSetsBlock(uint8_t* buffer, uint8_t* aes_key, uint8_t* aes_iv_mask);
void initDefaultFoldersBlock(uint8_t* buffer, uint8_t* aes_key, uint8_t* aes_iv_mask);
void initDefaultPhraseTemplatesBlock(uint8_t* buffer, uint8_t* aes_key, uint8_t* aes_iv_mask);
