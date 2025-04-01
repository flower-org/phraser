#include "BlockDAO.h"

#include <Thumby.h>
#include "PhraserUtils.h"
#include "BlockCache.h"
#include "Adler.h"
#include "pbkdf2-sha256.h"
#include "Schema_builder.h"
#include "SerialUtils.h"

// -------------------- DB block load -------------------- 

bool random_seed_initialized = false;
void initRandomIfNeeded() {
  if (!random_seed_initialized) {
    // use prevoiusly stored random value and current micros to create random seed
    uint32_t micros_int = micros();
    char micros_str[50];
    itoa(micros_int, micros_str, 10);

    char last_entropy_str[50];
    itoa(get_last_entropy(), last_entropy_str, 10);

    uint8_t digest[32];
    sha2_context ctx;
    sha2_starts(&ctx, 0);
    sha2_update(&ctx, (uint8_t*)last_entropy_str, strlen(last_entropy_str));
    sha2_update(&ctx, (uint8_t*)micros_str, strlen(micros_str));
    sha2_finish(&ctx, digest);

    uint32_t seed = sha256ToUInt32(digest);
    randomSeed(seed);

    random_seed_initialized = true;
  }
}

bool inPlaceDecryptAndValidateBlock(uint8_t *in_out_db_block, uint32_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  //2. Decrypt using aes256_key_block_key and HARDCODED_IV_MASK
  uint8_t* iv = xorByteArrays(aes_iv_mask, in_out_db_block+(block_size - IV_MASK_LEN), IV_MASK_LEN);
  inPlaceDecryptBlock4096(aes_key, iv, in_out_db_block);
  free(iv);

  //3. validate Adler32 checksum
  uint32_t length_without_adler = block_size - IV_MASK_LEN - 4;
  uint32_t expected_adler = bytesToUInt32(in_out_db_block+length_without_adler);
  uint32_t adler32_checksum = adler32(in_out_db_block, length_without_adler);
  if (expected_adler == adler32_checksum) {
    //4. reverse decrypted block
    reverseInPlace(in_out_db_block, length_without_adler);
    return true;
  } else {
    return false;
  }
}

bool loadBlockFromFlash(uint8_t bank_number, uint16_t block_number, uint32_t block_size,
                        uint8_t* aes_key, uint8_t* aes_iv_mask, 
                        uint8_t *out_db_block) {
    //1. Read block at key_block_decrypt_cursor
    readDbBlockFromFlashBank(bank_number, block_number, (void*)out_db_block);
    return inPlaceDecryptAndValidateBlock(out_db_block, block_size, aes_key, aes_iv_mask);
}

bool throw_block_back(uint16_t block_number) {
  return NULL;
}

// -------------------- FLATBUF -------------------- 

flatbuffers_ref_t str(flatcc_builder_t* builder, const char* s) {
  return flatbuffers_string_create_str(builder, s);
}

// -------------------- BLOCK DB -------------------- 

void wrapDataBufferInBlock(uint8_t block_type, uint8_t* main_buffer, const uint8_t* aes_key, 
  const uint8_t* aes_iv_mask, void *block_buffer, size_t block_buffer_size) {
  serialDebugPrintf("block_buffer_size %zu\r\n", (uint32_t)block_buffer_size);

  // Generate block IV
  uint8_t block_iv[IV_MASK_LEN];
  for (int i = 0; i < IV_MASK_LEN; i++) {
    main_buffer[FLASH_SECTOR_SIZE - IV_MASK_LEN + i] =
      block_iv[i] = (uint8_t)random(256); // Generate random byte
  }

  uint8_t* iv = xorByteArrays(block_iv, (uint8_t*)aes_iv_mask, IV_MASK_LEN);

  main_buffer[0] = block_type;
  uInt16ToBytes(block_buffer_size, main_buffer+1);

  memcpy(main_buffer + 3, block_buffer, block_buffer_size);
  
  uint32_t length_without_adler = FLASH_SECTOR_SIZE - IV_MASK_LEN - 4;
  for (int i = 3 + block_buffer_size; i < length_without_adler; i++) {
    main_buffer[i] = (uint8_t)random(256);
  }
  reverseInPlace(main_buffer, length_without_adler);
  
  uint32_t adler32_checksum = adler32(main_buffer, length_without_adler);
  uInt32ToBytes(adler32_checksum, main_buffer+length_without_adler);

  inPlaceEncryptBlock4096((uint8_t*)aes_key, iv, main_buffer);

  free(iv);
}

// -------------------- PHRASER -------------------- 

void storeBlockNewVersionAndEntropy(phraser_StoreBlock_t* store_block, phraser_StoreBlock_struct_t old_store_block) {
  store_block->block_id = phraser_StoreBlock_block_id(old_store_block);
  store_block->version = next_block_version();
  store_block->entropy = random_uint32();
}

void foldersBlock_folder(flatcc_builder_t* builder, uint16_t folder_id, uint16_t parent_folder_id, const char* name) {
  phraser_FoldersBlock_folders_push_create(builder, folder_id, parent_folder_id, str(builder, name));
}

void foldersBlock_folder(flatcc_builder_t* builder, phraser_Folder_table_t folder_fb) {
  phraser_FoldersBlock_folders_push_create(builder, 
    phraser_Folder_folder_id(folder_fb),
    phraser_Folder_parent_folder_id(folder_fb),
    str(builder, phraser_Folder_folder_name(folder_fb)));
}

void foldersBlock_folderVec(flatcc_builder_t* builder, phraser_Folder_vec_t old_folders) {
  size_t folders_vec_length = flatbuffers_vec_len(old_folders);
  for (int i = 0; i < folders_vec_length; i++) {
    foldersBlock_folder(builder, phraser_Folder_vec_at(old_folders, i));
  }
}

void symbolSetsBlock_symbolSet(flatcc_builder_t* builder, uint16_t symbol_set_id, const char* name, const char* set) {
  phraser_SymbolSetsBlock_symbol_sets_push_create(builder, symbol_set_id, str(builder, name), str(builder, set));
}

void symbolSetsBlock_symbolSet(flatcc_builder_t* builder, phraser_SymbolSet_table_t symbol_set_fb) {
  phraser_SymbolSetsBlock_symbol_sets_push_create(builder, 
    phraser_SymbolSet_set_id(symbol_set_fb), 
    str(builder, phraser_SymbolSet_symbol_set_name(symbol_set_fb)), 
    str(builder, phraser_SymbolSet_symbol_set(symbol_set_fb)));
}

void symbolSetsBlock_symbolSetVec(flatcc_builder_t* builder, phraser_SymbolSet_vec_t old_symbol_sets) {
  size_t symbol_sets_vec_length = flatbuffers_vec_len(old_symbol_sets);
  for (int i = 0; i < symbol_sets_vec_length; i++) {
    symbolSetsBlock_symbolSet(builder, phraser_SymbolSet_vec_at(old_symbol_sets, i));
  }
}

// -------------------- PHRASER BLOCKS -------------------- 

// TODO: this could a bit easier if Version was in the header outside flatbuf structure.
//  That would also fit the common DB structure better and make complemantary copy operations agnostic to flatbuf or content format.
//  With that said, we choose to update phraser-specific `entropy` field at the same time, which requires to deserialize flatBuffer.
//  This consideration makes updating the structure-related code ASAP impractical. 

UpdateResponse updateVersionAndEntropyKeyBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  initRandomIfNeeded();
  
  phraser_KeyBlock_table_t key_block;
  if (!(key_block = phraser_KeyBlock_as_root(block + DATA_OFFSET))) {
    return ERROR;
  }
  phraser_StoreBlock_struct_t old_store_block = phraser_KeyBlock_block(key_block);

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_KeyBlock_start_as_root(&builder);

  phraser_KeyBlock_block_count_add(&builder, phraser_KeyBlock_block_count(key_block));

  flatbuffers_int8_vec_t key = phraser_KeyBlock_key(key_block);
  size_t key_length = flatbuffers_vec_len(key);
  phraser_KeyBlock_key_create(&builder, key, key_length);

  flatbuffers_int8_vec_t iv = phraser_KeyBlock_iv(key_block);
  size_t iv_length = flatbuffers_vec_len(iv);
  phraser_KeyBlock_iv_create(&builder, iv, iv_length);

  flatbuffers_int8_vec_t db_name = phraser_KeyBlock_db_name(key_block);
  size_t db_name_length = flatbuffers_vec_len(db_name);
  phraser_KeyBlock_db_name_create(&builder, db_name, db_name_length);

  phraser_StoreBlock_t* store_block = phraser_KeyBlock_block_start(&builder);
  storeBlockNewVersionAndEntropy(store_block, old_store_block);
  phraser_KeyBlock_block_end(&builder);

  phraser_KeyBlock_end_as_root(&builder);

  void *block_buffer;
  size_t block_buffer_size;
  block_buffer = flatcc_builder_finalize_aligned_buffer(&builder, &block_buffer_size);

  wrapDataBufferInBlock(phraser_BlockType_KeyBlock, block, aes_key, aes_iv_mask, block_buffer, block_buffer_size);

  flatcc_builder_aligned_free(block_buffer);
  flatcc_builder_clear(&builder);

  return OK;
}

UpdateResponse updateVersionAndEntropySymbolSetsBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  initRandomIfNeeded();
  
  phraser_SymbolSetsBlock_table_t symbol_sets_block;
  if (!(symbol_sets_block = phraser_SymbolSetsBlock_as_root(block + DATA_OFFSET))) {
    return ERROR;
  }

  phraser_StoreBlock_struct_t old_store_block = phraser_SymbolSetsBlock_block(symbol_sets_block);
  phraser_SymbolSet_vec_t old_symbol_sets = phraser_SymbolSetsBlock_symbol_sets(symbol_sets_block);

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_SymbolSetsBlock_start_as_root(&builder);

  phraser_SymbolSetsBlock_symbol_sets_start(&builder);
  symbolSetsBlock_symbolSetVec(&builder, old_symbol_sets);
  phraser_SymbolSetsBlock_symbol_sets_end(&builder);

  phraser_StoreBlock_t* store_block = phraser_SymbolSetsBlock_block_start(&builder);
  storeBlockNewVersionAndEntropy(store_block, old_store_block);
  phraser_SymbolSetsBlock_block_end(&builder);

  phraser_SymbolSetsBlock_end_as_root(&builder);

  void *block_buffer;
  size_t block_buffer_size;
  block_buffer = flatcc_builder_finalize_aligned_buffer(&builder, &block_buffer_size);

  wrapDataBufferInBlock(phraser_BlockType_SymbolSetsBlock, block, aes_key, aes_iv_mask, block_buffer, block_buffer_size);

  flatcc_builder_aligned_free(block_buffer);
  flatcc_builder_clear(&builder);

  return OK;
}

UpdateResponse updateVersionAndEntropyFoldersBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  initRandomIfNeeded();
  
  phraser_FoldersBlock_table_t folders_block;
  if (!(folders_block = phraser_FoldersBlock_as_root(block + DATA_OFFSET))) {
    return ERROR;
  }

  phraser_StoreBlock_struct_t old_store_block = phraser_FoldersBlock_block(folders_block);
  phraser_Folder_vec_t old_folders = phraser_FoldersBlock_folders(folders_block);

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_FoldersBlock_start_as_root(&builder);

  phraser_FoldersBlock_folders_start(&builder);
  foldersBlock_folderVec(&builder, old_folders);
  phraser_FoldersBlock_folders_end(&builder);

  phraser_StoreBlock_t* store_block = phraser_FoldersBlock_block_start(&builder);
  storeBlockNewVersionAndEntropy(store_block, old_store_block);
  phraser_FoldersBlock_block_end(&builder);

  phraser_FoldersBlock_end_as_root(&builder);

  void *block_buffer;
  size_t block_buffer_size;
  block_buffer = flatcc_builder_finalize_aligned_buffer(&builder, &block_buffer_size);

  wrapDataBufferInBlock(phraser_BlockType_FoldersBlock, block, aes_key, aes_iv_mask, block_buffer, block_buffer_size);

  flatcc_builder_aligned_free(block_buffer);
  flatcc_builder_clear(&builder);

  return OK;
}

UpdateResponse updateVersionAndEntropyPhraseTemplatesBlock(uint8_t* block, uint16_t block_size) {
  initRandomIfNeeded();
  return ERROR;
}

UpdateResponse updateVersionAndEntropyPhraseBlock(uint8_t* block, uint16_t block_size) {
  initRandomIfNeeded();
  return ERROR;
}

UpdateResponse updateVersionAndEntropyBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  initRandomIfNeeded();
  if (!inPlaceDecryptAndValidateBlock(block, block_size, aes_key, aes_iv_mask)) {
    return ERROR;
  }

  switch (block[0]) {
    case phraser_BlockType_KeyBlock: 
      return updateVersionAndEntropyKeyBlock(block, block_size, aes_key, aes_iv_mask);
    case phraser_BlockType_SymbolSetsBlock: 
      return updateVersionAndEntropySymbolSetsBlock(block, block_size, aes_key, aes_iv_mask);
    case phraser_BlockType_FoldersBlock: 
      return updateVersionAndEntropyFoldersBlock(block, block_size, aes_key, aes_iv_mask);
    case phraser_BlockType_PhraseTemplatesBlock: 
      return updateVersionAndEntropyPhraseTemplatesBlock(block, block_size);
    case phraser_BlockType_PhraseBlock: 
      return updateVersionAndEntropyPhraseBlock(block, block_size);
  }
  return ERROR;
}

// -------------------- FOLDERS -------------------- 

UpdateResponse addNewFolder(char* new_folder_name) {
  initRandomIfNeeded();
  // 1. check that db capacity is enough to add new block
  if (max_db_block_count() - valid_block_count() <= 1) {
    return DB_FULL;
  } 

  // 2. load folders block
  uint16_t folder_block_number;
  uint16_t block_size = FLASH_SECTOR_SIZE;
  uint8_t block[block_size];
  if (!loadBlockFromFlash(bank_number, folder_block_number, block_size,
    main_key, main_iv_mask, 
    block)) {
    return ERROR;
  }

  // 2.1 deserialize folders flatbuf block
  phraser_FoldersBlock_table_t folders_block;
  if (!(folders_block = phraser_FoldersBlock_as_root(block + DATA_OFFSET))) {
    return ERROR;
  }

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  if (!phraser_FoldersBlock_clone_as_root(&builder, folders_block)) {
    return ERROR;
  };

  flatbuffers_buffer_ref_t unused_prob = phraser_FoldersBlock_clone_as_root(&builder, folders_block);
  phraser_StoreBlock_mutable_struct_t store_block = (phraser_StoreBlock_mutable_struct_t)phraser_FoldersBlock_block_get(folders_block);
  store_block->version = next_block_version();
  store_block->entropy = random_uint32();

  phraser_FoldersBlock_block_add(&builder, store_block);

  /*
  folders_block_id = phraser_StoreBlock_block_id(storeblock);
  serialDebugPrintf("folders_block_id %d\r\n", folders_block_id);
*/

  // 3. common updates
  //  3.1 version update
  //  3.2 entropy

  // 4. add new folder
  // 5. build new Folders block
  //  5.1 check Folders block size is within limits
  // 6. perform DB update
  // 7. resync folders from block
  // 8. reinit UI

  return ERROR;
}

UpdateResponse renameFolder(uint16_t folder_id, char* new_folder_name) {
  initRandomIfNeeded();
  //
  return ERROR;
}

UpdateResponse deleteFolder(uint16_t folder_id) {
  initRandomIfNeeded();
  //
  return ERROR;
}

// -------------------- PHRASES -------------------- 

UpdateResponse addNewPhrase(char* new_phrase_name, uint16_t phrase_template_id, uint16_t folder_id) {
  initRandomIfNeeded();
  //
  return ERROR;
}

UpdateResponse deletePhrase(uint16_t phrase_block_id) {
  initRandomIfNeeded();
  //
  return ERROR;
}

UpdateResponse renamePhrase(uint16_t phrase_block_id, char* new_phrase_name) {
  initRandomIfNeeded();
  //
  return ERROR;
}

UpdateResponse changePhraseTemplate(uint16_t phrase_block_id, uint16_t new_phrase_template_id) {
  initRandomIfNeeded();
  //
  return ERROR;
}

UpdateResponse changePhraseFolder(uint16_t phrase_block_id, uint16_t new_folder_id) {
  initRandomIfNeeded();
  //
  return ERROR;
}

UpdateResponse generatePhraseWord(uint16_t phrase_block_id, uint16_t word_template_id, uint8_t word_template_ordinal) {
  initRandomIfNeeded();
  //
  return ERROR;
}

UpdateResponse userEditPhraseWord(uint16_t phrase_block_id, uint16_t word_template_id, uint8_t word_template_ordinal, char* new_word, uint16_t new_word_length) {
  initRandomIfNeeded();
  //
  return ERROR;
}

UpdateResponse deletePhraseHistory(uint16_t phrase_block_id, uint16_t phrase_history_index) {
  initRandomIfNeeded();
  //
  return ERROR;
}

UpdateResponse makePhraseHistoryCurrent(uint16_t phrase_block_id, uint16_t phrase_history_index) {
  initRandomIfNeeded();
  //
  return ERROR;
}
