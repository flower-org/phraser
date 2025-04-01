#include "BlockDAO.h"

#include <Thumby.h>
#include "PhraserUtils.h"
#include "BlockCache.h"
#include "Adler.h"
#include "pbkdf2-sha256.h"
#include "Schema_builder.h"
#include "DefaultDbInitializer.h"

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

bool loadBlockFromFlash(uint8_t bank_number, uint16_t block_number, uint32_t block_size,
                        uint8_t* aes_key, uint8_t* aes_iv_mask, 
                        uint8_t *out_db_block) {
    //1. Read block at key_block_decrypt_cursor
    readDbBlockFromFlashBank(bank_number, block_number, (void*)out_db_block);

    //2. Decrypt using aes256_key_block_key and HARDCODED_IV_MASK
    uint8_t* iv = xorByteArrays(aes_iv_mask, out_db_block+(block_size - IV_MASK_LEN), IV_MASK_LEN);
    inPlaceDecryptBlock4096(aes_key, iv, out_db_block);
    free(iv);

    //3. validate Adler32 checksum
    uint32_t length_without_adler = block_size - IV_MASK_LEN - 4;
    uint32_t expected_adler = bytesToUInt32(out_db_block+length_without_adler);
    uint32_t adler32_checksum = adler32(out_db_block, length_without_adler);
    if (expected_adler == adler32_checksum) {
      //4. reverse decrypted block
      reverseInPlace(out_db_block, length_without_adler);
      return true;
    } else {
      return false;
    }
}

bool throw_block_back(uint16_t block_number) {
  return NULL;
}

// -------------------- COMMON -------------------- 

// TODO: this could a bit easier if Version was in the header outside flatbuf structure.
//  That would also fit the common DB structure better and make complemantary copy operations agnostic to flatbuf or content format.
//  With that said, we choose to update phraser-specific `entropy` field at the same time, which requires to deserialize flatBuffer.
//  This consideration makes updating the structure-related code ASAP impractical. 

UpdateResponse updateVersionAndEntropyKeyBlock(uint8_t* block, uint16_t block_size) {
  initRandomIfNeeded();
  return ERROR;
}

UpdateResponse updateVersionAndEntropySymbolSetsBlock(uint8_t* block, uint16_t block_size) {
  initRandomIfNeeded();
  return ERROR;
}

UpdateResponse updateVersionAndEntropyFoldersBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  initRandomIfNeeded();
  
  phraser_FoldersBlock_table_t folders_block;
  if (!(folders_block = phraser_FoldersBlock_as_root(block + DATA_OFFSET))) {
    return ERROR;
  }

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  if (!phraser_FoldersBlock_clone_as_root(&builder, folders_block)) {
    return ERROR;
  };

  phraser_StoreBlock_mutable_struct_t store_block = (phraser_StoreBlock_mutable_struct_t)phraser_FoldersBlock_block_get(folders_block);
  store_block->version = next_block_version();
  store_block->entropy = random_uint32();

  phraser_FoldersBlock_block_add(&builder, store_block);

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
  switch (block[0]) {
    case phraser_BlockType_KeyBlock: 
      return updateVersionAndEntropyKeyBlock(block, block_size);
      break;
    case phraser_BlockType_SymbolSetsBlock: 
      return updateVersionAndEntropySymbolSetsBlock(block, block_size);
      break;
    case phraser_BlockType_FoldersBlock: 
      return updateVersionAndEntropyFoldersBlock(block, block_size, aes_key, aes_iv_mask);
      break;
    case phraser_BlockType_PhraseTemplatesBlock: 
      return updateVersionAndEntropyPhraseTemplatesBlock(block, block_size);
      break;
    case phraser_BlockType_PhraseBlock: 
      return updateVersionAndEntropyPhraseBlock(block, block_size);
      break;
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
