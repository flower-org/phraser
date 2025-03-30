#pragma once

#include <Thumby.h>
#include <Arduino.h>

#include "PhraserUtils.h"
#include "Schema_reader.h"

extern uint8_t HARDCODED_SALT[];
extern const int HARDCODED_SALT_LEN;

extern uint8_t HARDCODED_IV_MASK[];
extern const int IV_MASK_LEN;

extern const int PBKDF_INTERATIONS_COUNT;

// - Login data cache
extern uint8_t* key_block_key;
extern uint32_t key_block_key_length;

// - KeyBlock cache
extern char* db_name;
extern uint32_t db_name_length;

extern uint16_t max_block_count;
extern uint8_t* main_key;
extern uint32_t main_key_length;
extern uint8_t* main_iv_mask;
extern uint32_t main_iv_mask_length;

void setLoginData(uint8_t* key, uint32_t key_length);

void startBlockCacheInit();
void registerBlockInBlockCache(uint8_t* block, uint16_t block_number);
void finalizeBlockCacheInit();
