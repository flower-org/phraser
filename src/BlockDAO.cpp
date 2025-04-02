#include "BlockDAO.h"

#include <Thumby.h>
#include "PhraserUtils.h"
#include "BlockCache.h"
#include "Adler.h"
#include "pbkdf2-sha256.h"
#include "Schema_builder.h"
#include "SerialUtils.h"

struct DAOFolder {
  uint16_t folder_id;
  uint16_t parent_folder_id;
  char* folder_name;
};

int IV_SIZE = 16;
int ADLER_32_CHECKSUM_SIZE = 4;
int BLOCK_TYPE_SIZE = 1;
int DATA_LENGTH_SIZE = 2;
int DATA_BLOCK_SIZE = FLASH_SECTOR_SIZE - (IV_SIZE + ADLER_32_CHECKSUM_SIZE + BLOCK_TYPE_SIZE + DATA_LENGTH_SIZE);
int ENCRYPTED_BLOCK_SIZE_NO_ADLER = FLASH_SECTOR_SIZE - (IV_SIZE + ADLER_32_CHECKSUM_SIZE);
int ENCRYPTED_BLOCK_SIZE = FLASH_SECTOR_SIZE - IV_SIZE;

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
    readDbBlockFromFlashBank(bank_number, block_number, (void*)out_db_block);
    return inPlaceDecryptAndValidateBlock(out_db_block, block_size, aes_key, aes_iv_mask);
}

void saveBlockToFlash(uint8_t bank_number, uint16_t block_number, uint8_t* encrypted_block, uint32_t block_size) {
    writeDbBlockToFlashBank(bank_number, block_number, encrypted_block);
}

bool throw_block_back(uint16_t block_number) {
  return NULL;
}

// -------------------- FLATBUF -------------------- 

flatbuffers_ref_t str(flatcc_builder_t* builder, const char* s) {
  return flatbuffers_string_create_str(builder, s);
}

flatbuffers_uint16_vec_ref_t vec_uint16(flatcc_builder_t* builder, uint16_t* arr, uint16_t arr_len) {
  return flatbuffers_uint16_vec_create(builder, arr, arr_len);
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

void phraseTemplatesBlock_wordTemplate(flatcc_builder_t* builder, uint16_t word_template_id, uint8_t permissions, phraser_Icon_enum_t icon, 
  uint16_t min_length, uint16_t max_length, const char* word_template_name, uint16_t* symbol_set_ids, uint16_t symbol_set_ids_length) {
  phraser_PhraseTemplatesBlock_word_templates_push_create(builder, word_template_id, permissions, icon, min_length, max_length, 
    str(builder, word_template_name), vec_uint16(builder, symbol_set_ids, symbol_set_ids_length));
}

void phraseTemplatesBlock_wordTemplate(flatcc_builder_t* builder, phraser_WordTemplate_table_t word_template_fb) {
  flatbuffers_uint16_vec_t symbol_set_ids = phraser_WordTemplate_symbol_set_ids(word_template_fb);
  phraser_PhraseTemplatesBlock_word_templates_push_create(builder, 
    phraser_WordTemplate_word_template_id(word_template_fb),
    phraser_WordTemplate_permissions(word_template_fb), 
    phraser_WordTemplate_icon(word_template_fb), 
    phraser_WordTemplate_min_length(word_template_fb), 
    phraser_WordTemplate_max_length(word_template_fb), 
    str(builder, phraser_WordTemplate_word_template_name(word_template_fb)), 
    vec_uint16(builder, (uint16_t*)symbol_set_ids, flatbuffers_vec_len(symbol_set_ids))
  );
}

void phraseTemplatesBlock_wordTemplateVec(flatcc_builder_t* builder, phraser_WordTemplate_vec_t old_word_templates) {
  size_t word_templates_vec_length = flatbuffers_vec_len(old_word_templates);
  for (int i = 0; i < word_templates_vec_length; i++) {
    phraseTemplatesBlock_wordTemplate(builder, phraser_WordTemplate_vec_at(old_word_templates, i));
  }
}

void phraseTemplatesBlock_phraseTemplate(flatcc_builder_t* builder, uint16_t phrase_template_id, const char* phrase_template_name, 
  uint16_t* word_template_ids, uint8_t* word_template_ordinals, uint16_t word_templates_length) {
  phraser_WordTemplateRef_vec_start(builder);
  for (int i = 0; i < word_templates_length; i++) {
    phraser_WordTemplateRef_vec_push_create(builder, word_template_ids[i], word_template_ordinals[i]);
  }
  phraser_WordTemplateRef_vec_ref_t word_template_refs = phraser_WordTemplateRef_vec_end(builder);

  phraser_PhraseTemplatesBlock_phrase_templates_push_create(builder, phrase_template_id, str(builder, phrase_template_name), word_template_refs);
}

void phraseTemplatesBlock_phraseTemplate(flatcc_builder_t* builder, phraser_PhraseTemplate_table_t phrase_template_fb) {
  phraser_WordTemplateRef_vec_t word_template_ref_vec = phraser_PhraseTemplate_word_template_refs(phrase_template_fb);
  size_t word_templates_length = flatbuffers_vec_len(word_template_ref_vec);

  phraser_WordTemplateRef_vec_start(builder);
  for (int i = 0; i < word_templates_length; i++) {
    phraser_WordTemplateRef_table_t word_template_fb = phraser_WordTemplateRef_vec_at(word_template_ref_vec, i);
    phraser_WordTemplateRef_vec_push_create(builder, phraser_WordTemplateRef_word_template_id(word_template_fb), 
          phraser_WordTemplateRef_word_template_ordinal(word_template_fb));
  }
  phraser_WordTemplateRef_vec_ref_t word_template_refs = phraser_WordTemplateRef_vec_end(builder);

  phraser_PhraseTemplatesBlock_phrase_templates_push_create(builder, 
    phraser_PhraseTemplate_phrase_template_id(phrase_template_fb),
    str(builder, phraser_PhraseTemplate_phrase_template_name(phrase_template_fb)), 
    word_template_refs);
}

void phraseTemplatesBlock_phraseTemplateVec(flatcc_builder_t* builder, phraser_PhraseTemplate_vec_t old_phrase_templates_vec) {
  size_t old_phrase_templates_vec_length = flatbuffers_vec_len(old_phrase_templates_vec);
  for (int i = 0; i < old_phrase_templates_vec_length; i++) {
    phraseTemplatesBlock_phraseTemplate(builder, phraser_PhraseTemplate_vec_at(old_phrase_templates_vec, i));
  }
}

void phraseBlock_history(flatcc_builder_t* builder, uint16_t word_template_id, int8_t word_template_ordinal,
  char* name, char* word, int8_t permissions, phraser_Icon_enum_t icon) {
  phraser_Word_vec_push_create(builder, 
    word_template_id,
    word_template_ordinal,
    str(builder, name),
    str(builder, word),
    permissions,
    icon
  );
}

void phraseBlock_history(flatcc_builder_t* builder, phraser_PhraseHistory_table_t old_phrase_history) {
  uint16_t history_phrase_template_id = phraser_PhraseHistory_phrase_template_id(old_phrase_history);
  phraser_Word_vec_t words_vec = phraser_PhraseHistory_phrase(old_phrase_history);
  size_t words_vec_length = flatbuffers_vec_len(words_vec);

  phraser_Word_vec_start(builder);
  for (int i = 0; i < words_vec_length; i++) {
    phraser_Word_table_t word_fb = phraser_Word_vec_at(words_vec, i);
    phraser_Word_vec_push_create(builder, 
      phraser_Word_word_template_id(word_fb),
      phraser_Word_word_template_ordinal(word_fb),
      str(builder, phraser_Word_name(word_fb)),
      str(builder, phraser_Word_word(word_fb)),
      phraser_Word_permissions(word_fb),
      phraser_Word_icon(word_fb)
    );
  }
  phraser_Word_vec_ref_t word_refs = phraser_Word_vec_end(builder);

  phraser_PhraseBlock_history_push_create(builder, 
    history_phrase_template_id, 
    word_refs);
}

void phraseBlock_historyVec(flatcc_builder_t* builder, phraser_PhraseHistory_vec_t old_phrase_history_vec) {
  size_t old_phrase_history_vec_length = flatbuffers_vec_len(old_phrase_history_vec);
  for (int i = 0; i < old_phrase_history_vec_length; i++) {
    phraseBlock_history(builder, phraser_PhraseHistory_vec_at(old_phrase_history_vec, i));
  }
}

// -------------------- PHRASER BLOCKS -------------------- 

// TODO: Version bump could be a bit easier if Version was in the block header outside flatbuf structure.
//  That would also fit the common DB structure better and make Complementary Copy (throwback) operation agnostic to flatbuf structure
//  or more generally, to content format.
//  With that said, we choose to update phraser-specific `entropy` field at the same time, which requires to deserialize flatBuffer.
//  This consideration makes updating the structure-related code ASAP impractical (or at all in this project). 

UpdateResponse wrapUpBlock(uint8_t block_type, flatcc_builder_t* builder, uint8_t* block, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  void *block_buffer;
  size_t block_buffer_size;
  block_buffer = flatcc_builder_finalize_aligned_buffer(builder, &block_buffer_size);

  if (block_buffer_size > DATA_BLOCK_SIZE) {
    return BLOCK_SIZE_EXCEEDED;
  }

  wrapDataBufferInBlock(block_type, block, aes_key, aes_iv_mask, block_buffer, block_buffer_size);

  flatcc_builder_aligned_free(block_buffer);
  flatcc_builder_clear(builder);

  return OK;
}

UpdateResponse updateVersionAndEntropyKeyBlock(uint8_t* block, uint16_t block_size, uint8_t* key_block_key, uint8_t* key_block_mask) {
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

  wrapUpBlock(phraser_BlockType_KeyBlock, &builder, block, key_block_key, key_block_mask);
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

  wrapUpBlock(phraser_BlockType_SymbolSetsBlock, &builder, block, aes_key, aes_iv_mask);
  return OK;
}

//arraylist<DAOFolder> new_folders
UpdateResponse updateVersionAndEntropyFoldersBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask, 
  arraylist* new_folders) {
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
  if (new_folders == NULL) {
    foldersBlock_folderVec(&builder, old_folders);
  } else {
    for (int i = 0; i < arraylist_size(new_folders); i++) {
      DAOFolder* folder = (DAOFolder*)arraylist_get(new_folders, i);
      foldersBlock_folder(&builder, folder->folder_id, folder->parent_folder_id, folder->folder_name);
    }
  }
  phraser_FoldersBlock_folders_end(&builder);

  phraser_StoreBlock_t* store_block = phraser_FoldersBlock_block_start(&builder);
  storeBlockNewVersionAndEntropy(store_block, old_store_block);
  phraser_FoldersBlock_block_end(&builder);

  phraser_FoldersBlock_end_as_root(&builder);

  wrapUpBlock(phraser_BlockType_FoldersBlock, &builder, block, aes_key, aes_iv_mask);
  return OK;
}

UpdateResponse updateVersionAndEntropyFoldersBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  return updateVersionAndEntropyFoldersBlock(block, block_size, aes_key, aes_iv_mask, NULL);
}

UpdateResponse updateVersionAndEntropyPhraseTemplatesBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  initRandomIfNeeded();

  phraser_PhraseTemplatesBlock_table_t phrase_templates_block;
  if (!(phrase_templates_block = phraser_PhraseTemplatesBlock_as_root(block + DATA_OFFSET))) {
    return ERROR;
  }

  phraser_StoreBlock_struct_t old_store_block = phraser_PhraseTemplatesBlock_block(phrase_templates_block);
  phraser_WordTemplate_vec_t word_templates_vec = phraser_PhraseTemplatesBlock_word_templates(phrase_templates_block);
  phraser_PhraseTemplate_vec_t phrase_template_vec = phraser_PhraseTemplatesBlock_phrase_templates(phrase_templates_block);

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_PhraseTemplatesBlock_start_as_root(&builder);

  phraser_PhraseTemplatesBlock_word_templates_start(&builder);
  phraseTemplatesBlock_wordTemplateVec(&builder, word_templates_vec);
  phraser_PhraseTemplatesBlock_word_templates_end(&builder);

  phraser_PhraseTemplatesBlock_phrase_templates_start(&builder);
  phraseTemplatesBlock_phraseTemplateVec(&builder, phrase_template_vec);
  phraser_PhraseTemplatesBlock_phrase_templates_end(&builder);

  phraser_StoreBlock_t* store_block = phraser_PhraseTemplatesBlock_block_start(&builder);
  storeBlockNewVersionAndEntropy(store_block, old_store_block);
  phraser_PhraseTemplatesBlock_block_end(&builder);

  phraser_PhraseTemplatesBlock_end_as_root(&builder);

  wrapUpBlock(phraser_BlockType_PhraseTemplatesBlock, &builder, block, aes_key, aes_iv_mask);
  return OK;
}

UpdateResponse updateVersionAndEntropyPhraseBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  initRandomIfNeeded();
 
  phraser_PhraseBlock_table_t phrase_block;
  if (!(phrase_block = phraser_PhraseBlock_as_root(block + DATA_OFFSET))) {
    return ERROR;
  }

  phraser_StoreBlock_struct_t old_store_block = phraser_PhraseBlock_block(phrase_block);
  phraser_PhraseHistory_vec_t phrase_history_vec = phraser_PhraseBlock_history(phrase_block);

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_PhraseBlock_start_as_root(&builder);

  phraser_PhraseBlock_phrase_template_id_add(&builder, phraser_PhraseBlock_phrase_template_id(phrase_block));
  phraser_PhraseBlock_folder_id_add(&builder, phraser_PhraseBlock_folder_id(phrase_block));
  phraser_PhraseBlock_is_tombstone_add(&builder, phraser_PhraseBlock_is_tombstone(phrase_block));
  phraser_PhraseBlock_phrase_name_add(&builder, str(&builder, phraser_PhraseBlock_phrase_name(phrase_block)));

  phraser_PhraseBlock_history_start(&builder);
  phraseBlock_historyVec(&builder, phrase_history_vec);
  phraser_PhraseBlock_history_end(&builder);

  phraser_StoreBlock_t* store_block = phraser_PhraseBlock_block_start(&builder);
  storeBlockNewVersionAndEntropy(store_block, old_store_block);
  phraser_PhraseBlock_block_end(&builder);

  phraser_PhraseBlock_end_as_root(&builder);

  wrapUpBlock(phraser_BlockType_PhraseBlock, &builder, block, aes_key, aes_iv_mask);
  return OK;
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
      return updateVersionAndEntropyPhraseTemplatesBlock(block, block_size, aes_key, aes_iv_mask);
    case phraser_BlockType_PhraseBlock: 
      return updateVersionAndEntropyPhraseBlock(block, block_size, aes_key, aes_iv_mask);
  }
  return ERROR;
}

void throwBlockBack(uint8_t bank_number, uint16_t block_number_from, uint16_t block_number_to, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  // TODO: implement
}

uint16_t throwbackCopy() {
  //TODO implement - return block_number to which B0 shold go
  return -1;
}

// -------------------- FOLDERS -------------------- 

UpdateResponse addNewFolder(char* new_folder_name, uint16_t parent_folder_id) {
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

  uint16_t max_folder_id = 0;
  arraylist* dao_folders = arraylist_create();
  phraser_Folder_vec_t folders_vec = phraser_FoldersBlock_folders(folders_block);
  size_t folders_vec_length = flatbuffers_vec_len(folders_vec);
  for (int i = 0; i < folders_vec_length; i++) {
    phraser_Folder_table_t folder_fb = phraser_Folder_vec_at(folders_vec, i);

    DAOFolder* daoFolder = (DAOFolder*)malloc(sizeof(DAOFolder));
    daoFolder->folder_id = phraser_Folder_folder_id(folder_fb);
    daoFolder->parent_folder_id = phraser_Folder_parent_folder_id(folder_fb);
    daoFolder->folder_name = (char*)phraser_Folder_folder_name(folder_fb);

    if (daoFolder->folder_id > max_folder_id) {
      max_folder_id = daoFolder->folder_id;
    }

    arraylist_add(dao_folders, daoFolder);
  }

  DAOFolder* daoFolder = (DAOFolder*)malloc(sizeof(DAOFolder));
  daoFolder->folder_id = max_folder_id+1;
  daoFolder->parent_folder_id = parent_folder_id;
  daoFolder->folder_name = new_folder_name;

  arraylist_add(dao_folders, daoFolder);

  UpdateResponse block_response = updateVersionAndEntropyFoldersBlock(block, block_size, main_key, main_iv_mask, dao_folders);
  
  for (int i = 0; i < arraylist_size(dao_folders); i++) {
    free(arraylist_get(dao_folders, i)); 
  }
  free(dao_folders);

  if (OK != block_response) {
    return block_response;
  }
  
  uint16_t main_block_number = throwbackCopy();
  saveBlockToFlash(bank_number, main_block_number, block, block_size);

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
