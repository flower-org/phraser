#include "BlockCache.h"

#include "c-list.h"

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

uint64_t current_entropy = 0;

// - Login data cache
uint8_t* key_block_key = NULL;
uint32_t key_block_key_length = 0;

// - KeyBlock cache
uint16_t key_block_id = 0;
uint8_t* key_block_buffer = NULL;

char* db_name = NULL;
uint32_t db_name_length = 0;

uint16_t block_count = 0;
uint8_t* main_key = NULL;
uint32_t main_key_length = 0;
uint8_t* main_iv_mask = NULL;
uint32_t main_iv_mask_length = 0;

// - SymbolSetsBlock cache
uint16_t symbol_sets_block_id = 0;
uint8_t* symbol_sets_block_buffer = NULL;

// - FoldersBlock cache
uint16_t folders_block_id = 0;
uint8_t* folders_block_buffer = NULL;

// - PhraseTemplatesBlock cache
uint16_t phrase_templates_block_id = 0;
uint8_t* phrase_templates_block_buffer = NULL;

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

uint8_t* setKeyBlock(uint8_t* block, uint16_t block_number) {
  if (key_block_buffer != NULL) { free(key_block_buffer); }
  if (main_key != NULL) { free(main_key); }
  if (main_iv_mask != NULL) { free(main_iv_mask); }
  if (db_name != NULL) { free(db_name); }
  
  if (block != NULL) {
    key_block_buffer = copyBuffer(block, 4096);

    phraser_KeyBlock_table_t key_block;
    if (!(key_block = phraser_KeyBlock_as_root(key_block_buffer + DATA_OFFSET))) {
      return NULL;
    }

    phraser_StoreBlock_struct_t storeblock = phraser_KeyBlock_block(key_block);
    key_block_id = phraser_StoreBlock_block_id(storeblock);
    Serial.printf("key_block_id %d\r\n", key_block_id);

    //TODO: add to blockId maps
    uint32_t key_block_version = phraser_StoreBlock_version(storeblock);
    Serial.printf("key_block_version %d\r\n", key_block_version);

    block_count = phraser_KeyBlock_block_count(key_block);
    Serial.printf("block_count %d\r\n", block_count);

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

    return key_block_buffer;
  } else {
    key_block_buffer = NULL; 
    main_key = NULL;
    main_iv_mask = NULL;
    db_name = NULL;

    key_block_id = 0;
    
    block_count = 0;
    main_key_length = 0;
    main_iv_mask_length = 0;
    return NULL;
  }
}

uint8_t* setSymbolSetsBlock(uint8_t* block, uint16_t block_number) {
  // TODO: implement
  phraser_SymbolSetsBlock_table_t symbol_sets_block;
  if (!(symbol_sets_block = phraser_SymbolSetsBlock_as_root(block + DATA_OFFSET))) {
    return NULL;
  }

  phraser_StoreBlock_struct_t storeblock = phraser_SymbolSetsBlock_block(symbol_sets_block);
  symbol_sets_block_id = phraser_StoreBlock_block_id(storeblock);
  Serial.printf("symbol_sets_block_id %d\r\n", symbol_sets_block_id);

  //TODO: add to blockId maps
  uint32_t symbol_sets_version = phraser_StoreBlock_version(storeblock);
  Serial.printf("symbol_sets_version %d\r\n", symbol_sets_version);

  return NULL;
}

uint8_t* setFoldersBlock(uint8_t* block, uint16_t block_number) {
  // TODO: implement
  phraser_FoldersBlock_table_t folders_block;
  if (!(folders_block = phraser_FoldersBlock_as_root(block + DATA_OFFSET))) {
    return NULL;
  }

  phraser_StoreBlock_struct_t storeblock = phraser_FoldersBlock_block(folders_block);
  folders_block_id = phraser_StoreBlock_block_id(storeblock);
  Serial.printf("folders_block_id %d\r\n", folders_block_id);

  //TODO: add to blockId maps
  uint32_t folders_block_version = phraser_StoreBlock_version(storeblock);
  Serial.printf("folders_block_version %d\r\n", folders_block_version);

  return NULL;
}

uint8_t* setPhraseTemplatesBlock(uint8_t* block, uint16_t block_number) {
  // TODO: implement
  phraser_PhraseTemplatesBlock_table_t phrase_templates_block;
  if (!(phrase_templates_block = phraser_PhraseTemplatesBlock_as_root(block + DATA_OFFSET))) {
    return NULL;
  }

  phraser_StoreBlock_struct_t storeblock = phraser_PhraseTemplatesBlock_block(phrase_templates_block);
  phrase_templates_block_id = phraser_StoreBlock_block_id(storeblock);
  Serial.printf("phrase_templates_block_id %d\r\n", phrase_templates_block_id);

  //TODO: add to blockId maps
  uint32_t phrase_templates_block_version = phraser_StoreBlock_version(storeblock);
  Serial.printf("phrase_templates_block_version %d\r\n", phrase_templates_block_version);

  return NULL;
}

uint8_t* registerPhraseBlock(uint8_t* block, uint16_t block_number) {
  // TODO: implement
  phraser_PhraseBlock_table_t phrase_block;
  if (!(phrase_block = phraser_PhraseBlock_as_root(block + DATA_OFFSET))) {
    return NULL;
  }

  phraser_StoreBlock_struct_t storeblock = phraser_PhraseBlock_block(phrase_block);
  uint16_t phrase_block_id = phraser_StoreBlock_block_id(storeblock);
  Serial.printf("phrase_block_id %d\r\n", phrase_block_id);

  //TODO: add to blockId maps
  uint32_t phrase_block_version = phraser_StoreBlock_version(storeblock);
  Serial.printf("phrase_block_version %d\r\n", phrase_block_version);

  return NULL;
}

uint8_t* registerBlock(uint8_t* block, uint16_t block_number) {
  switch (block[0]) {
    case phraser_BlockType_KeyBlock: return setKeyBlock(block, block_number); break;
    case phraser_BlockType_SymbolSetsBlock: return setSymbolSetsBlock(block, block_number); break;
    case phraser_BlockType_FoldersBlock: return setFoldersBlock(block, block_number); break;
    case phraser_BlockType_PhraseTemplatesBlock: return setPhraseTemplatesBlock(block, block_number); break;
    case phraser_BlockType_PhraseBlock: return registerPhraseBlock(block, block_number); break;
    default: return NULL;
  }
}
