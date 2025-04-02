#pragma once

#include <Thumby.h>
#include <Arduino.h>

#include "PhraserUtils.h"
#include "Schema_reader.h"
#include "arraylist.h"
#include "rbtree.h"

extern uint8_t HARDCODED_SALT[];
extern const int HARDCODED_SALT_LEN;

extern uint8_t HARDCODED_IV_MASK[];
extern const int IV_MASK_LEN;

extern const int PBKDF_INTERATIONS_COUNT;
extern const int DATA_OFFSET;

struct Folder {
  uint32_t folderId;
  uint32_t parentFolderId;
  char* folderName;
};

struct PhraseFolderAndName {
  uint32_t phraseBlockId;
  uint32_t folderId;
  char* name;
};

struct FolderOrPhrase {
  PhraseFolderAndName* phrase;
  Folder* folder;
};

// - Login data cache
extern uint8_t* key_block_key;
extern uint32_t key_block_key_length;
extern uint8_t bank_number;

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

extern Folder* getFolder(uint16_t folder_id);
//  arraylist<FolderOrPhrase*>
arraylist* getFolderContent(uint16_t parent_folder_id);

uint16_t db_block_count();
uint16_t valid_block_count();
uint16_t free_block_count();
bool last_block_left();

uint32_t get_last_entropy();
uint32_t next_block_version();

node_t* occupied_block_numbers();
uint32_t last_block_number();
