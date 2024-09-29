#pragma once

#include <stdio.h>

#include "Arduino.h"
extern "C" {
#include "my_hashtable.h"
};

#include "BlockStore.h" //for BLOCK_COUNT

#include <Base64.h>
#include "Arduino.h"
#include "Adler.h"
#include "StreamUtils.h"
#include "aes.hpp"
#include "StructBuf.h"

/*
 * 1. Blocks (array with 384 possible ids, by position; +rw status: overwriteable/read only)
 * 2. Block position by id (hashtable int-int)
 * 3. Folders (array for 256 possible folder ids.  Reuse ids.)
 * 4. Character set blocks (active block list. maybe array id-block_id? 256 ids. Reuse ids.)
 * 5. Phrase Template blocks (active block list. maybe array id-block_id? 256 ids. Reuse ids.)
 * 
 */

/*
 * folders
 * symbolSets
 * phraseTemplates
 * phrases
 * 
 */

typedef struct  {
  Folder* folder;
  
  hashtable_t* subfolders;
  hashtable_t* childPhrases;
} FolderTreeNode;

//constructor
void loadRegistryFromFlash();

//Main blocks index
uint8_t addBlock(uint32_t index, StoreBlock* blockToAdd);
RegStoreBlockAndIndex* getBlockById(uint32_t blockId);
StoreBlock* getBlockByIndex(uint32_t index);
uint32_t getNextFreeIndex();
uint32_t getFreeBlocksCount();

//Entities
FolderTreeNode* getRootFolder();
FolderTreeNode* getFolderById(uint32_t folderId);
SymbolSet* getSymbolSetById(uint32_t setId);
PhraseTemplate* getPhraseTemplateById(uint32_t templateId);
PhraseBlock* getPhraseBlockById(uint32_t blockId);

//Helper method for testing
void setTestOverrideBlockCount(uint32_t newBlockCount);
