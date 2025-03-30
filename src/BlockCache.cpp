#include "BlockCache.h"

#include "rbtree.h"
#include "hashtable.h"
#include "arraylist.h"

//SHA256 of string "PhraserPasswordManager"
uint8_t HARDCODED_SALT[] = {
  0xE9, 0x8A, 0xD5, 0x84, 0x33, 0xB6, 0xE9, 0xE3,
  0x03, 0x30, 0x6F, 0x29, 0xE0, 0x94, 0x43, 0x8B,
  0x13, 0xA5, 0x52, 0x22, 0xD2, 0x89, 0x0E, 0x5F,
  0x6E, 0x0E, 0xC4, 0x29, 0xFB, 0x40, 0xE2, 0x6D
};
const int HARDCODED_SALT_LEN = 32;

//MD5 of string "PhraserPasswordManager"
uint8_t HARDCODED_IV_MASK[] = {
  0x44, 0x75, 0xBB, 0x91, 0x5E, 0xA8, 0x40, 0xDB,
  0xCE, 0x22, 0xDA, 0x4E, 0x22, 0x4B, 0x8A, 0x3C
};
const int IV_MASK_LEN = 16;

const int PBKDF_INTERATIONS_COUNT = 10239;

const int DATA_OFFSET = 3;

struct BlockIdAndVersion {
  uint16_t blockId;
  uint32_t blockVersion;
  uint32_t entropy;
  bool isTombstoned;
};

BlockIdAndVersion BLOCK_NOT_UPDATED = { 0, 0, 0, false };

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

// DB data structures 
uint16_t lastBlockId;
uint32_t lastBlockVersion;
uint32_t lastBlockNumber;
uint32_t lastEntropy;

node_t* occupiedBlockNumbers;
hashtable* blockIdByBlockNumber; //key BlockNumber, value BlockId
hashtable* blockInfos; //key BlockId, value BlockNumberAndVersionAndCount
hashtable* tombstonedBlockIds; //key BlockId, value BlockId 

// - Login data cache
uint8_t* key_block_key = NULL;
uint32_t key_block_key_length = 0;

// - KeyBlock cache
uint8_t* key_block_buffer = NULL;

uint16_t key_block_id = 0;
uint32_t key_block_version = 0;

char* db_name = NULL;
uint32_t db_name_length = 0;

uint16_t max_block_count = 0;
uint8_t* main_key = NULL;
uint32_t main_key_length = 0;
uint8_t* main_iv_mask = NULL;
uint32_t main_iv_mask_length = 0;

// - SymbolSetsBlock cache
uint8_t* symbol_sets_block_buffer = NULL;

uint16_t symbol_sets_block_id = 0;
uint32_t symbol_sets_block_version = 0;
hashtable* symbol_sets = NULL;//uint32_t, SymbolSet

// - FoldersBlock cache
uint8_t* folders_block_buffer = NULL;

uint16_t folders_block_id = 0;
uint32_t folders_block_version = 0;
hashtable* folders;//uint32_t, Folder
hashtable* sub_folders_by_folder;//uint32_t, List<uint32_t>

// - PhraseTemplatesBlock cache
uint16_t phrase_templates_block_id = 0;
uint8_t* phrase_templates_block_buffer = NULL;
//final Map<Integer, PhraseTemplate> phraseTemplates;
//final Map<Integer, WordTemplate> wordTemplates;

// - Phrase Blocks (minimal info) cache
//final Map<Integer, PhraseFolderAndName> phrases;
//final Map<Integer, Set<Integer>> phrasesByFolder;

void setLoginData(uint8_t* key, uint32_t key_length) {
  if (key_block_key != NULL) {
    free(key_block_key);
  }

  if (key != NULL) {
    key_block_key_length = key_length;
    key_block_key = copyBuffer(key, key_length);
  } else {
    key_block_key_length = 0;
    key_block_key = NULL;
  }
}

BlockIdAndVersion setKeyBlock(uint8_t* block) {
  if (key_block_buffer != NULL) { free(key_block_buffer); }
  if (main_key != NULL) { free(main_key); }
  if (main_iv_mask != NULL) { free(main_iv_mask); }
  if (db_name != NULL) { free(db_name); }
  
  if (block != NULL) {
    key_block_buffer = copyBuffer(block, 4096);

    phraser_KeyBlock_table_t key_block;
    if (!(key_block = phraser_KeyBlock_as_root(key_block_buffer + DATA_OFFSET))) {
      return BLOCK_NOT_UPDATED;
    }

    phraser_StoreBlock_struct_t storeblock = phraser_KeyBlock_block(key_block);

    uint32_t new_key_block_version = phraser_StoreBlock_version(storeblock);
    Serial.printf("new_key_block_version %d\r\n", new_key_block_version);
    if (new_key_block_version >= key_block_version) {
      key_block_version = new_key_block_version;
      key_block_id = phraser_StoreBlock_block_id(storeblock);
      Serial.printf("key_block_id %d\r\n", key_block_id);
    
      uint32_t entropy = phraser_StoreBlock_entropy(storeblock);
      Serial.printf("entropy %d\r\n", entropy);

      max_block_count = phraser_KeyBlock_block_count(key_block);
      Serial.printf("max_block_count %d\r\n", max_block_count);

      flatbuffers_int8_vec_t db_name_str = phraser_KeyBlock_db_name(key_block);
      db_name_length = flatbuffers_vec_len(db_name_str);
      db_name = copyString((char*)db_name_str, db_name_length);
      Serial.printf("db_name %s\r\n", db_name);

      flatbuffers_int8_vec_t main_key_vec = phraser_KeyBlock_key(key_block);
      main_key_length = flatbuffers_vec_len(main_key_vec);
      main_key = copyBuffer((uint8_t*)main_key_vec, main_key_length);
      Serial.printf("main_key_length %d\r\n", main_key_length);

      flatbuffers_int8_vec_t aes256_iv_mask_vec = phraser_KeyBlock_iv(key_block);
      main_iv_mask_length = flatbuffers_vec_len(aes256_iv_mask_vec);
      main_iv_mask = copyBuffer((uint8_t*)aes256_iv_mask_vec, main_iv_mask_length);
      Serial.printf("aes256_iv_mask_length %d\r\n", main_iv_mask_length);

      Serial.printf("ZXC. Before RETURN\r\n");
      return { key_block_id, key_block_version, entropy, false };
    } else {
      return BLOCK_NOT_UPDATED;
    }
  } else {
    key_block_buffer = NULL; 
    main_key = NULL;
    main_iv_mask = NULL;
    db_name = NULL;

    key_block_id = 0;
    key_block_version = 0;
    max_block_count = 0;

    main_key_length = 0;
    main_iv_mask_length = 0;
    db_name_length = 0;
    return BLOCK_NOT_UPDATED;
  }
}

void removeAllSymbolSets(hashtable *t, uint32_t key, void* value) {
  SymbolSet* removed_symbol_set = (SymbolSet*)hashtable_remove(t, key);
  if (removed_symbol_set != NULL) {
    free(removed_symbol_set->symbolSet);
    free(removed_symbol_set->symbolSetName);
    free(removed_symbol_set);
  }
}

BlockIdAndVersion setSymbolSetsBlock(uint8_t* block) {
  if (symbol_sets_block_buffer != NULL) { free(symbol_sets_block_buffer); }
  if (symbol_sets != NULL) { hashtable_iterate_entries(symbol_sets, removeAllSymbolSets); }

  if (block != NULL) {
    key_block_buffer = copyBuffer(block, 4096);

    phraser_SymbolSetsBlock_table_t symbol_sets_block;
    if (!(symbol_sets_block = phraser_SymbolSetsBlock_as_root(block + DATA_OFFSET))) {
      return BLOCK_NOT_UPDATED;
    }

    phraser_StoreBlock_struct_t storeblock = phraser_SymbolSetsBlock_block(symbol_sets_block);

    uint32_t new_symbol_sets_block_version = phraser_StoreBlock_version(storeblock);
    Serial.printf("new_symbol_sets_block_version %d\r\n", new_symbol_sets_block_version);
    if (new_symbol_sets_block_version >= symbol_sets_block_version) {
      symbol_sets_block_version = new_symbol_sets_block_version;
      symbol_sets_block_id = phraser_StoreBlock_block_id(storeblock);
      Serial.printf("symbol_sets_block_id %d\r\n", symbol_sets_block_id);
    
      uint32_t entropy = phraser_StoreBlock_entropy(storeblock);
      Serial.printf("entropy %d\r\n", entropy);

      phraser_SymbolSet_vec_t symbol_sets_vec = phraser_SymbolSetsBlock_symbol_sets(symbol_sets_block);
      size_t symbol_sets_vec_length = flatbuffers_vec_len(symbol_sets_vec);
      Serial.printf("symbol_sets_vec_length %d\r\n", symbol_sets_vec_length);

      if (symbol_sets == NULL) {
        symbol_sets = hashtable_create();
      }

      for (size_t i = 0; i < symbol_sets_vec_length; i++) {
        phraser_SymbolSet_table_t symbol_set_fb = phraser_SymbolSet_vec_at(symbol_sets_vec, i);

        uint16_t symbol_set_id = phraser_SymbolSet_set_id(symbol_set_fb);

        flatbuffers_string_t symbol_set_name_str = phraser_SymbolSet_symbol_set_name(symbol_set_fb);
        size_t symbol_set_name_str_length = flatbuffers_string_len(symbol_set_name_str);

        flatbuffers_string_t symbol_set_str = phraser_SymbolSet_symbol_set(symbol_set_fb);
        size_t symbol_set_str_length = flatbuffers_string_len(symbol_set_str);

        SymbolSet* symbol_set = (SymbolSet*)malloc(sizeof(SymbolSet));
        symbol_set->symbolSetId = symbol_set_id;
        symbol_set->symbolSetName = copyString((char*)symbol_set_name_str, symbol_set_name_str_length);
        symbol_set->symbolSet = copyString((char*)symbol_set_str, symbol_set_str_length);

        hashtable_set(symbol_sets, symbol_set_id, symbol_set);

        Serial.printf("symbol_set_id %d\r\n", symbol_set_id);
        Serial.printf("symbolSetName %s\r\n", symbol_set->symbolSetName);
        Serial.printf("symbolSet %s\r\n", symbol_set->symbolSet);
      }

      return { symbol_sets_block_id, symbol_sets_block_version, entropy, false };
    } else {
      return BLOCK_NOT_UPDATED;
    }
  } else {
    if (symbol_sets != NULL){ 
      hashtable_destroy(symbol_sets);
    }
    symbol_sets_block_id = 0;
    symbol_sets_block_version = 0;
    symbol_sets = NULL;
    symbol_sets_block_buffer = NULL;
    return BLOCK_NOT_UPDATED;
  }
}

void removeAllFolders(hashtable *t, uint32_t key, void* value) {
  Folder* removed_folder = (Folder*)hashtable_remove(t, key);
  if (removed_folder != NULL) {
    free(removed_folder->folderName);
    free(removed_folder);
  }
}

void removeAllSubFoldersByfolder(hashtable *t, uint32_t key, void* value) {
  arraylist* removed_sub_folder_list = (arraylist*)hashtable_remove(t, key);
  if (removed_sub_folder_list != NULL) {
    arraylist_destroy(removed_sub_folder_list);
  }
}

BlockIdAndVersion setFoldersBlock(uint8_t* block) {
  if (folders_block_buffer != NULL) { free(folders_block_buffer); }
  if (folders != NULL) { hashtable_iterate_entries(folders, removeAllFolders); }
  if (sub_folders_by_folder != NULL) { hashtable_iterate_entries(sub_folders_by_folder, removeAllSubFoldersByfolder); }
  
  if (block != NULL) {
    folders_block_buffer = copyBuffer(block, 4096);

    phraser_FoldersBlock_table_t folders_block;
    if (!(folders_block = phraser_FoldersBlock_as_root(block + DATA_OFFSET))) {
      return BLOCK_NOT_UPDATED;
    }
  
    phraser_StoreBlock_struct_t storeblock = phraser_FoldersBlock_block(folders_block);
    uint32_t new_folders_block_version = phraser_StoreBlock_version(storeblock);
    Serial.printf("new_folders_block_version %d\r\n", new_folders_block_version);
    if (new_folders_block_version >= folders_block_id) {
      folders_block_version = new_folders_block_version;
      folders_block_id = phraser_StoreBlock_block_id(storeblock);
      Serial.printf("folders_block_id %d\r\n", folders_block_id);
    
      uint32_t entropy = phraser_StoreBlock_entropy(storeblock);
      Serial.printf("entropy %d\r\n", entropy);

      phraser_SymbolSet_vec_t folders_vec = phraser_FoldersBlock_folders(folders_block);
      size_t folders_vec_length = flatbuffers_vec_len(folders_vec);
      Serial.printf("folders_vec_length %d\r\n", folders_vec_length);

      if (folders == NULL) {
        folders = hashtable_create();
      }
      if (sub_folders_by_folder == NULL) {
        sub_folders_by_folder = hashtable_create();
      }

      for (size_t i = 0; i < folders_vec_length; i++) {
        phraser_Folder_table_t folder_fb = phraser_Folder_vec_at(folders_vec, i);

        uint16_t folder_id = phraser_Folder_folder_id(folder_fb);
        uint16_t parent_folder_id = phraser_Folder_parent_folder_id(folder_fb);

        flatbuffers_string_t folder_name_str = phraser_Folder_folder_name(folder_fb);
        size_t folder_name_length = flatbuffers_string_len(folder_name_str);

        Folder* folder = (Folder*)malloc(sizeof(Folder));
        folder->folderId = folder_id;
        folder->parentFolderId = parent_folder_id;
        folder->folderName = copyString((char*)folder_name_str, folder_name_length);

        hashtable_set(folders, folder_id, folder);

        Serial.printf("folder_id %d\r\n", folder_id);
        Serial.printf("parent_id %d\r\n", parent_folder_id);
        Serial.printf("folderName %s\r\n", folder->folderName);

        arraylist* subfolder_list_of_parent_folder = (arraylist*)hashtable_get(sub_folders_by_folder, parent_folder_id);
        if (subfolder_list_of_parent_folder == NULL) {
          subfolder_list_of_parent_folder = arraylist_create();
          hashtable_set(sub_folders_by_folder, parent_folder_id, subfolder_list_of_parent_folder);
        }
        arraylist_add(subfolder_list_of_parent_folder, (void*)folder_id);
        
        Serial.printf("subfolders of parent_id %d: ", parent_folder_id);
        for (int j = 0; j < subfolder_list_of_parent_folder->size; j++) {
          uint32_t child_folder_id = (uint32_t)arraylist_get(subfolder_list_of_parent_folder, j);
          Serial.printf("%d, ", child_folder_id);
        }
        Serial.printf("\r\n");
      }

      return { folders_block_id, folders_block_version, entropy, false };
    } else {
      return BLOCK_NOT_UPDATED;
    }
  } else {
    if (folders != NULL){ 
      hashtable_destroy(folders);
    }
    if (sub_folders_by_folder != NULL){ 
      hashtable_destroy(sub_folders_by_folder);
    }
    folders_block_id = 0;
    folders_block_version = 0;
    folders = NULL;
    sub_folders_by_folder = NULL;
    folders_block_buffer = NULL;
    return BLOCK_NOT_UPDATED;
  }
}

BlockIdAndVersion setPhraseTemplatesBlock(uint8_t* block) {
  // TODO: implement
  phraser_PhraseTemplatesBlock_table_t phrase_templates_block;
  if (!(phrase_templates_block = phraser_PhraseTemplatesBlock_as_root(block + DATA_OFFSET))) {
    return BLOCK_NOT_UPDATED;
  }

  phraser_StoreBlock_struct_t storeblock = phraser_PhraseTemplatesBlock_block(phrase_templates_block);
  phrase_templates_block_id = phraser_StoreBlock_block_id(storeblock);
  Serial.printf("phrase_templates_block_id %d\r\n", phrase_templates_block_id);

  //TODO: add to blockId maps
  uint32_t phrase_templates_block_version = phraser_StoreBlock_version(storeblock);
  Serial.printf("phrase_templates_block_version %d\r\n", phrase_templates_block_version);

  return BLOCK_NOT_UPDATED;
}

BlockIdAndVersion registerPhraseBlock(uint8_t* block) {
  // TODO: implement
  phraser_PhraseBlock_table_t phrase_block;
  if (!(phrase_block = phraser_PhraseBlock_as_root(block + DATA_OFFSET))) {
    return BLOCK_NOT_UPDATED;
  }

  phraser_StoreBlock_struct_t storeblock = phraser_PhraseBlock_block(phrase_block);
  uint16_t phrase_block_id = phraser_StoreBlock_block_id(storeblock);
  Serial.printf("phrase_block_id %d\r\n", phrase_block_id);

  //TODO: add to blockId maps
  uint32_t phrase_block_version = phraser_StoreBlock_version(storeblock);
  Serial.printf("phrase_block_version %d\r\n", phrase_block_version);

  return BLOCK_NOT_UPDATED;
}

void startBlockCacheInit() {
  Serial.printf("startBlockCacheInit!\r\n");
  // will be initialized by first tree_insert
  occupiedBlockNumbers = NULL;
  blockIdByBlockNumber = hashtable_create();
  blockInfos = hashtable_create();
  tombstonedBlockIds = hashtable_create();
  Serial.printf("startBlockCacheInit DONE\r\n");
}

void registerBlockInBlockCache(uint8_t* block, uint16_t block_number) {
  // 1. Update caches
  BlockIdAndVersion blockAndVersion = BLOCK_NOT_UPDATED;
  switch (block[0]) {
    case phraser_BlockType_KeyBlock: 
      blockAndVersion = setKeyBlock(block); 
      break;
    case phraser_BlockType_SymbolSetsBlock: 
      blockAndVersion = setSymbolSetsBlock(block); 
      break;
    case phraser_BlockType_FoldersBlock: 
      blockAndVersion = setFoldersBlock(block); 
      break;
    case phraser_BlockType_PhraseTemplatesBlock: 
      blockAndVersion = setPhraseTemplatesBlock(block); 
      break;
    case phraser_BlockType_PhraseBlock: 
      blockAndVersion = registerPhraseBlock(block); 
      break;
  }

  // 2. Update DB structures
  Serial.printf("2. Update DB structures!\r\n");
  Serial.printf("blockAndVersion.blockId %d\r\n", blockAndVersion.blockId);
  if (blockAndVersion.blockId > 0) {
    Serial.printf("2.1. Update holders of last values / counters\r\n");
    // 2.1. Update holders of last values / counters
    Serial.printf("lastBlockId %d\r\n", lastBlockId);
    if (blockAndVersion.blockId > lastBlockId) {
      lastBlockId = blockAndVersion.blockId;
    }
    Serial.printf("blockAndVersion.blockVersion %d\r\n", blockAndVersion.blockVersion);
    Serial.printf("lastBlockVersion %d\r\n", lastBlockVersion);
    Serial.printf("lastBlockNumber %d\r\n", lastBlockNumber);
    Serial.printf("block_number %d\r\n", block_number);
    Serial.printf("lastEntropy %d\r\n", lastEntropy);
    Serial.printf("blockAndVersion.entropy %d\r\n", blockAndVersion.entropy);
    if (blockAndVersion.blockVersion > lastBlockVersion) {
      lastBlockVersion = blockAndVersion.blockVersion;
      lastBlockNumber = block_number;
      lastEntropy = blockAndVersion.entropy;
    }

    Serial.printf("2.2. Add BlockNumber to BlockId mapping\r\n");
    // 2.2. Add BlockNumber to BlockId mapping
    hashtable_set(blockIdByBlockNumber, block_number, (void*)blockAndVersion.blockId);

    Serial.printf("2.3. Update BlockId to BlockNumberAndVersionAndCount mapping\r\n");
    // 2.3. Update BlockId to BlockNumberAndVersionAndCount mapping
    BlockNumberAndVersionAndCount* blockInfo = 
            (BlockNumberAndVersionAndCount*)hashtable_get(blockInfos, blockAndVersion.blockId);
    if (blockInfo == NULL) {
      blockInfo = (BlockNumberAndVersionAndCount*)malloc(sizeof(BlockNumberAndVersionAndCount));
      blockInfo->blockNumber = block_number;
      blockInfo->blockVersion = blockAndVersion.blockVersion;
      blockInfo->copyCount = 1;
      blockInfo->isTombstoned = blockAndVersion.isTombstoned;
      hashtable_set(blockInfos, blockAndVersion.blockId, blockInfo);
    } else {
      if (blockInfo->blockVersion < blockAndVersion.blockVersion) {
        blockInfo->blockNumber = block_number;
        blockInfo->blockVersion = blockAndVersion.blockVersion;
        blockInfo->isTombstoned = blockAndVersion.isTombstoned;
      }
      blockInfo->copyCount++;
    }
  }
}

void processTombstonedBlocks(hashtable *t, uint32_t key, void* value) {
  BlockNumberAndVersionAndCount* blockInfo = (BlockNumberAndVersionAndCount*)value;
  if (blockInfo->isTombstoned) {
    if (blockInfo->copyCount == 1) {
      void* removed_value = hashtable_remove(t, key);
      free(removed_value);//should be the same as parameter value;
    } else {
      hashtable_set(tombstonedBlockIds, key, (void*)key);
    }
  }
}

void formOccupiedBlockNumbers(hashtable *t, uint32_t key, void* value) {
  tree_insert(&occupiedBlockNumbers, key);
}

void finalizeBlockCacheInit() {
  // 3. Invalidate all Tombstoned blocks with a single copy, add those with multiple copies to `tombstoneBlockIds`.
  hashtable_iterate_entries(blockInfos, processTombstonedBlocks);

  // 4. Use blockInfos to add all actual block numbers to `occupiedBlockNumbers`
  hashtable_iterate_entries(blockInfos, formOccupiedBlockNumbers);
}
