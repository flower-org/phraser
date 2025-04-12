#pragma once

#include <Thumby.h>
#include <Arduino.h>

#include "PhraserUtils.h"
#include "Schema_reader.h"
#include "arraylist.h"
#include "rbtree.h"
#include "hashtable.h"

extern uint8_t HARDCODED_SALT[];
extern const int HARDCODED_SALT_LEN;

extern uint8_t HARDCODED_IV_MASK[];
extern const int IV_MASK_LEN;

extern const int PBKDF_INTERATIONS_COUNT;
extern const int DATA_OFFSET;

struct BlockNumberAndVersionAndCount {
  uint32_t blockNumber;
  uint32_t blockVersion;
  uint32_t copyCount;
  bool isTombstoned;
};

struct SymbolSet {
  uint32_t symbolSetId;
  char* symbolSetName;
  char* symbolSet;
};

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

struct PhraseTemplate {
  uint32_t phraseTemplateId;
  char* phraseTemplateName;

  arraylist* wordTemplateIds;
  arraylist* wordTemplateOrdinals;
};

struct WordTemplate {
  uint32_t wordTemplateId;
  uint8_t permissions;
  phraser_Icon_enum_t icon;
  uint32_t minLength;
  uint32_t maxLength;
  char* wordTemplateName;
  arraylist* symbolSetIds;
};

// DB data structures
extern uint16_t lastBlockId;
extern uint32_t lastBlockVersion;
extern uint32_t lastBlockNumber;
extern uint32_t lastEntropy;

extern hashtable* blockIdByBlockNumber; //key BlockNumber, value BlockId
extern hashtable* blockInfos; //key BlockId, value BlockNumberAndVersionAndCount
extern hashtable* tombstonedBlockIds; //key BlockId, value BlockId 

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
void dbCacheLoadReport();

PhraseFolderAndName* getPhrase(uint16_t phrase_id);
extern Folder* getFolder(uint16_t folder_id);
//  arraylist<uint32_t>
arraylist* getSubFolders(uint16_t parent_folder_id);
//  arraylist<FolderOrPhrase*>
arraylist* getFolderContent(uint16_t parent_folder_id);
int getFolderChildCount(uint16_t parent_folder_id);

//uint32_t, Folder
hashtable* getFolders();

uint16_t db_block_count();
uint16_t valid_block_count();
uint16_t free_block_count_with_tombstones();
bool db_full();
bool db_has_non_tombstoned_space();

uint32_t get_last_entropy();
uint16_t increment_and_get_next_block_id();
uint32_t increment_and_get_next_block_version();
uint32_t last_block_version();

node_t* occupied_block_numbers();
void removeFromOccupiedBlockNumbers(uint32_t key);
void addToOccupiedBlockNumbers(uint32_t key);

uint32_t last_block_number();
uint32_t last_block_id();

uint32_t key_block_number();
uint32_t get_folders_block_number();
uint32_t get_phrase_block_number(uint16_t phrase_block_id);

PhraseTemplate* getPhraseTemplate(uint16_t phrase_template_id);
WordTemplate* getWordTemplate(uint16_t word_template_id);
SymbolSet* getSymbolSet(uint16_t symbol_set_id);

hashtable* getPhraseTemplates();
void nuke_tombstone_blocks();
bool db_has_free_blocks();