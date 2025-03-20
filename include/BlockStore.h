#pragma once

extern "C" {
  #include <hardware/sync.h>
  #include <hardware/flash.h>
  #include <string.h>
};

#include <Base64.h>
#include "Arduino.h"
#include "Adler.h"
#include "StreamUtils.h"
#include "aes.hpp"
#include "StructBuf.h"

#define DATA_BLOCK_SIZE FLASH_SECTOR_SIZE
//AbstractBlock header
#define HEADER_SIZE (4 + 4 + 1)
#define CHECKSUM_SIZE 2
#define DATA_WO_CHECKSUM_SIZE (DATA_BLOCK_SIZE - CHECKSUM_SIZE)
#define READ_ADDR_OFFSET XIP_BASE
//0.5 mb mark
#define FIRST_DATA_BLOCK_OFFSET (1024*512)
//2 mb - 4kb
#define LAST_DATA_BLOCK_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

//TODO: restore original BLOCK_COUNT,
//TODO: slow startup due to full AES decoding of all 384 blocks
//TODO: speed up startup by introducing independent checksum encryption and restore full block count

// Startup times (flash load):
// 256 - 15s
// 128 - 8s
// 64 - 4.5s
// 32 - 3s
#define BLOCK_COUNT 32
//#define BLOCK_COUNT ((PICO_FLASH_SIZE_BYTES - FIRST_DATA_BLOCK_OFFSET) / DATA_BLOCK_SIZE)

#define POINTER_SIZE 4

//Encrypted block types
#define FOLDERS_BLOCK_TYPE 1
#define SYMBOL_SETS_BLOCK_TYPE 2
#define PHRASE_TEMPLATES_BLOCK_TYPE 3
#define PHRASE_BLOCK_TYPE 4

//Unencrypted block types
#define KEY_BLOCK_TYPE 5
#define DOGADKA_BLOCK_TYPE 6
#define NAPRYAMKA_BLOCK_TYPE 7
#define SMYCHKA_BLOCK_TYPE 8

const char* getBlockTypeStr(uint32_t blockType);

//------ Block Functions ------

uint8_t initStoreBlock(void* fromBlock4096, StoreBlock* toStoreBlock);

void saveKeyBlock(KeyBlock* keyBlock, void* toBlock4096);
uint8_t initKeyBlock(void* fromBlock4096, KeyBlock* toKeyBlock);
void deleteKeyBlock(KeyBlock* keyBlock);

void saveFoldersBlock(FoldersBlock* foldersBlock, void* toBlock4096);
uint8_t initFoldersBlock(void* fromBlock4096, FoldersBlock* toFoldersBlock);
void deleteFoldersBlock(FoldersBlock* foldersBlock);

void saveSymbolSetsBlock(SymbolSetsBlock* symbolSetBlock, void* toBlock4096);
uint8_t initSymbolSetsBlock(void* fromBlock4096, SymbolSetsBlock* toSymbolSetBlock);
void deleteSymbolSetsBlock(SymbolSetsBlock* symbolSetBlock);

void savePhraseTemplatesBlock(PhraseTemplatesBlock* phraseTemplatesBlock, void* toBlock4096);
uint8_t initPhraseTemplatesBlock(void* fromBlock4096, PhraseTemplatesBlock* toPhraseTemplatesBlock);
void deletePhraseTemplatesBlock(PhraseTemplatesBlock* phraseTemplatesBlock);

void savePhraseBlock(PhraseBlock* phraseBlock, void* toBlock4096);
uint8_t initPhraseBlock(void* fromBlock4096, PhraseBlock* toPhraseBlock);
void deletePhraseBlock(PhraseBlock* phraseBlock);

//------ Utility Functions ------

uint8_t* getStoreKey();

uint8_t validateBlockChecksum(void* block4096);

void inPlaceDecryptBlock4096(uint8_t* key, uint8_t* block4096);
void inPlaceEncryptBlock4096(uint8_t* key, uint8_t* block4096);

uint8_t readBlock4096DataFromFlash(uint16_t blockIndex, void* toBlock4096);
uint8_t writeBlock4096DataToFlash(uint16_t blockIndex, uint8_t* block4096);

void fillBufferWithRandomBytes(void** cursor, uint32_t size);

uint8_t* base64encode(uint32_t size, uint8_t* buffer);
uint8_t* base64decode(uint32_t size, uint8_t* buffer);
