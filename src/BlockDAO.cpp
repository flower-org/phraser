#include "BlockDAO.h"

#include <Thumby.h>
#include "PhraserUtils.h"
#include "BlockCache.h"
#include "Adler.h"
#include "pbkdf2-sha256.h"
#include "Schema_builder.h"
#include "SerialUtils.h"
#include "rbtree.h"
#include "hashtable.h"

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

void saveBlockUpdateToFlash(uint8_t bank_number, uint16_t block_number, uint8_t* encrypted_block, uint32_t block_size) {
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

void storeBlockNewVersionAndEntropy(phraser_StoreBlock_t* store_block, phraser_StoreBlock_struct_t old_store_block, uint32_t new_version) {
  store_block->block_id = phraser_StoreBlock_block_id(old_store_block);
  store_block->version = new_version;//increment_and_get_next_block_version();
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

UpdateResponse updateVersionAndEntropyKeyBlock(uint8_t* block, uint16_t block_size, uint8_t* key_block_key, uint8_t* key_block_mask, uint32_t new_version) {
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
  storeBlockNewVersionAndEntropy(store_block, old_store_block, new_version);
  phraser_KeyBlock_block_end(&builder);

  phraser_KeyBlock_end_as_root(&builder);

  wrapUpBlock(phraser_BlockType_KeyBlock, &builder, block, key_block_key, key_block_mask);
  return OK;
}

UpdateResponse updateVersionAndEntropySymbolSetsBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask, uint32_t new_version) {
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
  storeBlockNewVersionAndEntropy(store_block, old_store_block, new_version);
  phraser_SymbolSetsBlock_block_end(&builder);

  phraser_SymbolSetsBlock_end_as_root(&builder);

  wrapUpBlock(phraser_BlockType_SymbolSetsBlock, &builder, block, aes_key, aes_iv_mask);
  return OK;
}

//arraylist<DAOFolder> new_folders
UpdateResponse updateVersionAndEntropyFoldersBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask, uint32_t new_version, 
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
  storeBlockNewVersionAndEntropy(store_block, old_store_block, new_version);
  phraser_FoldersBlock_block_end(&builder);

  phraser_FoldersBlock_end_as_root(&builder);

  wrapUpBlock(phraser_BlockType_FoldersBlock, &builder, block, aes_key, aes_iv_mask);
  return OK;
}

UpdateResponse updateVersionAndEntropyFoldersBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask, uint32_t new_version) {
  return updateVersionAndEntropyFoldersBlock(block, block_size, aes_key, aes_iv_mask, new_version, NULL);
}

UpdateResponse updateVersionAndEntropyPhraseTemplatesBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask, uint32_t new_version) {
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
  storeBlockNewVersionAndEntropy(store_block, old_store_block, new_version);
  phraser_PhraseTemplatesBlock_block_end(&builder);

  phraser_PhraseTemplatesBlock_end_as_root(&builder);

  wrapUpBlock(phraser_BlockType_PhraseTemplatesBlock, &builder, block, aes_key, aes_iv_mask);
  return OK;
}

UpdateResponse updateVersionAndEntropyPhraseBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask, uint32_t new_version) {
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
  storeBlockNewVersionAndEntropy(store_block, old_store_block, new_version);
  phraser_PhraseBlock_block_end(&builder);

  phraser_PhraseBlock_end_as_root(&builder);

  wrapUpBlock(phraser_BlockType_PhraseBlock, &builder, block, aes_key, aes_iv_mask);
  return OK;
}

UpdateResponse updateVersionAndEntropyBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask, uint32_t new_version, boolean decrypt) {
  initRandomIfNeeded();
  if (decrypt) {
    if (!inPlaceDecryptAndValidateBlock(block, block_size, aes_key, aes_iv_mask)) {
      return ERROR;
    }
  }

  switch (block[0]) {
    case phraser_BlockType_KeyBlock: 
      return updateVersionAndEntropyKeyBlock(block, block_size, aes_key, aes_iv_mask, new_version);
    case phraser_BlockType_SymbolSetsBlock: 
      return updateVersionAndEntropySymbolSetsBlock(block, block_size, aes_key, aes_iv_mask, new_version);
    case phraser_BlockType_FoldersBlock: 
      return updateVersionAndEntropyFoldersBlock(block, block_size, aes_key, aes_iv_mask, new_version);
    case phraser_BlockType_PhraseTemplatesBlock: 
      return updateVersionAndEntropyPhraseTemplatesBlock(block, block_size, aes_key, aes_iv_mask, new_version);
    case phraser_BlockType_PhraseBlock: 
      return updateVersionAndEntropyPhraseBlock(block, block_size, aes_key, aes_iv_mask, new_version);
  }
  return ERROR;
}

void throwBlockBack(uint8_t bank_number, uint16_t block_number_from, uint16_t block_number_to, uint8_t* aes_key, uint8_t* aes_iv_mask) {
  // TODO: implement
}

uint32_t get_valid_block_number_on_the_right_of(uint32_t block_number) {
  node_t* valid_block_node = tree_higherKey(occupied_block_numbers(), block_number);
  if (valid_block_node == NULL) {
    valid_block_node = tree_minimum(occupied_block_numbers());
  }  
  return valid_block_node->_data;
}

bool left_lookup_missing_block_number_found;
uint32_t left_lookup_missing_block_number;
bool left_lookup(data_t next_block_number) {
  left_lookup_missing_block_number = left_lookup_missing_block_number - 1;
  if (next_block_number < left_lookup_missing_block_number) {
    left_lookup_missing_block_number_found = true;
    return true;
  }
  return false;
}

uint32_t get_free_block_number_on_the_left_of(uint32_t block_number) {
  left_lookup_missing_block_number_found = false;
  left_lookup_missing_block_number = block_number;
  traverse_left_excl(occupied_block_numbers(), block_number, left_lookup);
  if (left_lookup_missing_block_number_found) {
    return left_lookup_missing_block_number;
  }

  left_lookup_missing_block_number_found = false;
  left_lookup_missing_block_number = db_block_count();
  traverse_left_excl(occupied_block_numbers(), db_block_count(), left_lookup);
  if (left_lookup_missing_block_number_found) {
    return left_lookup_missing_block_number;
  }

  // If there are no free blocks, DB is in a bad state, offline repair needed.
  return -1;
}

void invalidateBlockIndices(uint32_t b1_block_number, uint32_t b2_block_number) {
  serialDebugPrintf("invalidateBlockIndices b1_block_number %d b2_block_number %d\r\n", b1_block_number, b2_block_number);
  // ---- B2-related logic, it was replaced at B2 block_number by B1 blockId ----
  uint32_t b1_block_id = (uint32_t)hashtable_get(blockIdByBlockNumber, b1_block_number);

  if (hashtable_exists(blockIdByBlockNumber, b2_block_number)) {
    // Remove B2 block number from blockIdByBlockNumber, since we've just replaced it with B1
    uint32_t b2_block_id = (uint32_t)hashtable_remove(blockIdByBlockNumber, b2_block_number);
    serialDebugPrintf("invalidateBlockIndices b1_block_id %d b2_block_id %d\r\n", b1_block_id, b2_block_id);
    
    // Get B2 block info and decrement copy count, since 1 copy of B2 blockId was just destroyed
    BlockNumberAndVersionAndCount* b2_info = 
      (BlockNumberAndVersionAndCount*)hashtable_get(blockInfos, b2_block_id);
    b2_info->copyCount--;

    if (b2_info->isTombstoned && b2_info->copyCount <= 1 && b2_block_id != b1_block_id) {
      // If we discover that B2 is Tombstoned and now has only 1 copy left,
      // and as long as B2 is not the same blockId as B1 (a copy of which we've just created)  
      // we can safely assume that disposing of that last copy of B2 won't result in incorect 
      // tombstone revival, due to the fact that it's the last and only copy left.
      // Therefore, it's safe to remove B2 from the indexes entirely and dispose of B2 blockId.
      hashtable_remove(blockInfos, b2_block_id);
      hashtable_remove(tombstonedBlockIds, b2_block_id);
    }
    // In the context of B2 removal, B2 is no longer occupied
    removeFromOccupiedBlockNumbers(b2_block_number);
  }
}

void updateBlockIndicesPostThrowbackMove(uint32_t b1_block_number, uint32_t b2_block_number) {
  serialDebugPrintf("updateBlockIndicesPostMove b1_block_number %d b2_block_number %d\r\n", b1_block_number, b2_block_number);
  // ---- B1-related logic, new version of which replaced B2 ---
  uint32_t b1_block_id = (uint32_t)hashtable_get(blockIdByBlockNumber, b1_block_number);
  uint32_t b1_block_new_version = last_block_version();
  serialDebugPrintf("updateBlockIndicesPostMove b1_block_id %d\r\n", b1_block_id);

  // B2 block number now holds block with B1 blockId
  hashtable_set(blockIdByBlockNumber, b2_block_number, (void*)b1_block_id);
  
  // We've just created a new copy of B1 blockId, so we increment copyCount
  // And we also update version to the new one, and block number to B2 block number
  BlockNumberAndVersionAndCount* b1_info = 
    (BlockNumberAndVersionAndCount*)hashtable_get(blockInfos, b1_block_id);
  b1_info->copyCount++;
  b1_info->blockVersion = b1_block_new_version;
  b1_info->blockNumber = b2_block_number;

  // Since B1 was essentially copied with a version bump, there is no need to update
  // tombstone stats, they didn't change. 

  // In the context of B1 creation, B2 block number is now occupied by B1 blockId.
  addToOccupiedBlockNumbers(b2_block_number);

  // B1 block number becomes "free", since the latest version of B1 blockId 
  // was moved to B2 block number
  removeFromOccupiedBlockNumbers(b1_block_number);
}

void updateBlockIndicesPostBlockMove(uint32_t b1_block_number, bool b1_tombstoned, uint32_t b2_block_number) {
  // Only update what's not going to be updated by `registerBlockInBlockCache` call

  uint32_t b1_block_id = (uint32_t)hashtable_get(blockIdByBlockNumber, b1_block_number);
  // Tombstone indices 
  if (b1_tombstoned) {
    hashtable_set(tombstonedBlockIds, b1_block_id, (void*)b1_block_id);
  }

  // In the context of B1 creation, B2 block number is now occupied by B1 blockId.
  addToOccupiedBlockNumbers(b2_block_number);

  // B1 block number becomes "free", since the latest version of B1 blockId 
  // was moved to B2 block number
  removeFromOccupiedBlockNumbers(b1_block_number);
}

//static bool perNode(data_t val) { serialDebugPrintf("%u ", val); return false; }
// returns block_number to which the main update shold go
uint16_t throwbackCopy(uint32_t b0_block_number, uint32_t new_version) {
  serialDebugPrintf("t1.\r\n");
//  traverse_inorder(occupied_block_numbers(), perNode);

  serialDebugPrintf("t1.\r\n");
  uint32_t border_block_number = last_block_number();
  serialDebugPrintf("t2 border_block_number %d.\r\n", border_block_number);
  uint32_t b1_block_number = get_valid_block_number_on_the_right_of(border_block_number);
  serialDebugPrintf("t3 b1_block_number %d.\r\n", b1_block_number);
  uint32_t b2_block_number = get_free_block_number_on_the_left_of(border_block_number);
  serialDebugPrintf("t4 b2_block_number %d.\r\n", b2_block_number);
  uint32_t b3_block_number;
  serialDebugPrintf("t5.\r\n");

  if (last_block_left()) {
    // In case we have only 1 free block, initially the only free block is B2.
    // Since B1 overwrites B2, after Throwback Copy B1 becomes the only free block.
    // Therefore the only place the main update can go to is B1

    // Main updates goes to B1
    serialDebugPrintf("t6.\r\n");
    b3_block_number = b1_block_number;
  } else {
    // If we have more than 1 free block initially, then after Throwback Copy is done,
    // the block immediately to the right of the border will always be free.
    //
    // This boils down to 2 possibilities:
    // 1) B2 is actually the block immediately to the right of the border, and after 
    //  throwback copy is done, it will become free.
    //
    // 2) The block immediately to the right of the border is not B2.
    //  That would mean this block is free, because B2 is first non-free block to the right.
    //  This block is also not B1, because B1 is the first free block to the left, and since 
    //  we have more than 1 free block, B1 has to be some other block than the "rightest" 
    //  free block.
    //  In this case Throwback Copy updates B2 and B1, while the block immediately 
    //  to the right of the border stays intact, i.e. it's free.

    // Main update goes to the block immediately to the right of the border
    serialDebugPrintf("t7.\r\n");
    b3_block_number = (border_block_number + 1) % db_block_count();
  }

  // 1. Load B1 (use keyBlockKey for key block)
  uint16_t block_size = FLASH_SECTOR_SIZE;
  uint8_t block[block_size];
  uint8_t* aes_key; 
  uint8_t* aes_iv_mask;

  if (b1_block_number == key_block_number()) {
    aes_key = key_block_key;
    aes_iv_mask = HARDCODED_IV_MASK;
  } else {
    aes_key = main_key;
    aes_iv_mask = main_iv_mask;
  }

  serialDebugPrintf("t8. bank_number %d b1_block_number %d\r\n", bank_number, b1_block_number);
  bool load_success = loadBlockFromFlash(bank_number, b1_block_number, block_size, aes_key, aes_iv_mask, block);
  if (!load_success) {
    return -1;
  }

  // 2. Update B1 version
  serialDebugPrintf("t9.\r\n");
  UpdateResponse update_response = updateVersionAndEntropyBlock(block, block_size, aes_key, aes_iv_mask, new_version, false);
  if (update_response != OK) {
    return -1;
  }

  // 3. Save B1 with bumped version to B2
  serialDebugPrintf("t10 bank_number %d b2_block_number %d.\r\n", bank_number, b2_block_number);
  saveBlockUpdateToFlash(bank_number, b2_block_number, block, block_size);

  // 4. Update DB indices

  // B2-related logic, it was replaced at B2 block_number by B1 blockId
  serialDebugPrintf("t11.\r\n");
  invalidateBlockIndices(b1_block_number, b2_block_number);
  // B1-related logic, new version of which replaced B2
  serialDebugPrintf("t12.\r\n");
  updateBlockIndicesPostThrowbackMove(b1_block_number, b2_block_number);

  // ---- Return block_number to which Main Update shold go ----
  serialDebugPrintf("t13 b3_block_number %d.\r\n", b3_block_number);
  return b3_block_number;
}

// -------------------- FOLDERS --------------------

UpdateResponse addNewFolder(char* new_folder_name, uint16_t parent_folder_id, uint16_t* out_new_folder_id) {
  initRandomIfNeeded();
  // 1. check that db capacity is enough to add new block
  serialDebugPrintf("1.\r\n");
  if (last_block_left()) {
    return DB_FULL;
  } 

  // 2. load folders block
  uint16_t folder_block_number = folders_block_number();
  uint16_t block_size = FLASH_SECTOR_SIZE;
  uint8_t block[block_size];
  serialDebugPrintf("2. loading bank_number %d, folder_block_number %d\r\n", bank_number, folder_block_number);
  if (!loadBlockFromFlash(bank_number, folder_block_number, block_size,
    main_key, main_iv_mask, 
    block)) {
    return ERROR;
  }

  // 3. deserialize folders flatbuf block
  phraser_FoldersBlock_table_t folders_block;
  if (!(folders_block = phraser_FoldersBlock_as_root(block + DATA_OFFSET))) {
    return ERROR;
  }
  serialDebugPrintf("3.\r\n");

  // 4. Form arraylist of existing block folders
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
  serialDebugPrintf("4.\r\n");

  // 5. Add new folder to the arraylist
  DAOFolder* daoFolder = (DAOFolder*)malloc(sizeof(DAOFolder));
  daoFolder->folder_id = max_folder_id + 1;
  *out_new_folder_id = max_folder_id + 1;
  daoFolder->parent_folder_id = parent_folder_id;
  daoFolder->folder_name = new_folder_name;

  arraylist_add(dao_folders, daoFolder);
  serialDebugPrintf("5.\r\n");

  uint32_t throwback_copy_version = increment_and_get_next_block_version();//throwback version comes first
  uint32_t new_block_version = increment_and_get_next_block_version();

  // 6. Form updated block with updated list of folders
  UpdateResponse block_response = updateVersionAndEntropyFoldersBlock(block, block_size, main_key, main_iv_mask, new_block_version, dao_folders);
  serialDebugPrintf("6.\r\n");

  // 7. Free arraylist of folders
  for (int i = 0; i < arraylist_size(dao_folders); i++) {
    free(arraylist_get(dao_folders, i)); 
  }
  free(dao_folders);

  if (OK != block_response) {
    return block_response;
  }
  serialDebugPrintf("7.\r\n");

  // 8. Perform Throwback Copy, according to flash preservation algorithm
  uint16_t b3_block_number = throwbackCopy(folder_block_number, throwback_copy_version);
  if (b3_block_number == -1) {
    return ERROR;
  }
  serialDebugPrintf("8.\r\n");

  // 9. Save the updated block to flash
  saveBlockUpdateToFlash(bank_number, b3_block_number, block, block_size);
  serialDebugPrintf("9.\r\n");

  // 10. Re-Load new block version from flash
  if (!loadBlockFromFlash(bank_number, b3_block_number, block_size,
    main_key, main_iv_mask, 
    block)) {
    return ERROR;
  }
  serialDebugPrintf("10.\r\n");

  // 11. Update DB indices

  // B3-related logic, it was replaced at B3 block_number by B0 blockId
  invalidateBlockIndices(folder_block_number, b3_block_number);
  // B0-related logic, new version of which replaced B3 (only partial index update, 
  //  to avoid interfering with subsequent registerBlockInBlockCache() call)
  updateBlockIndicesPostBlockMove(folder_block_number, false, b3_block_number);
  serialDebugPrintf("11.\r\n");

  // 12. Register new version in cache
  registerBlockInBlockCache(block, b3_block_number);
  serialDebugPrintf("12.\r\n");

  return OK;
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
