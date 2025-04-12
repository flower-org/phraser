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

void releaseFullPhraseWord(Word* word) {
  // free all related memory
  free(word->name);
  free(word->word);
  free(word);
}

void releaseFullPhraseHistory(PhraseHistory* phrase_history) {
  for (int i = 0; i < arraylist_size(phrase_history->phrase); i++) {
    releaseFullPhraseWord((Word*)arraylist_get(phrase_history->phrase, i));
  }
  arraylist_destroy(phrase_history->phrase);
  free(phrase_history);
}

void releaseFullPhrase(FullPhrase* full_phrase) {
  free(full_phrase->phrase_name);
  for (int i = 0; i < arraylist_size(full_phrase->history); i++) {
    releaseFullPhraseHistory((PhraseHistory*)arraylist_get(full_phrase->history, i));
  }
  arraylist_destroy(full_phrase->history);
  free(full_phrase);
}

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
  // It might be possible to deduce some facts about random number generator settings
  // from sequences it produces. Since IV Part (block_iv) is stored as is, we're obfuscating
  // the actual numbers produced by the number generator by hashing them with sha256.
  uint8_t digest[32];
  sha2_context ctx;
  sha2_starts(&ctx, 0);
  for (int i = 0; i < IV_MASK_LEN; i++) {
    int rnd = rand();
    sha2_update(&ctx, (uint8_t*)&rnd, sizeof(rnd));
  }
  sha2_finish(&ctx, digest);

  uint8_t* block_iv = xorByteArrays(digest, digest + IV_MASK_LEN, IV_MASK_LEN);
  for (int i = 0; i < IV_MASK_LEN; i++) {
    main_buffer[FLASH_SECTOR_SIZE - IV_MASK_LEN + i] = block_iv[i];
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
  free(block_iv);
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

void phraseBlock_historyArray_history(flatcc_builder_t* builder, PhraseHistory* phrase_history) {
  uint16_t history_phrase_template_id = phrase_history->phrase_template_id;
  size_t words_vec_length = arraylist_size(phrase_history->phrase);

  phraser_Word_vec_start(builder);
  for (int i = 0; i < words_vec_length; i++) {
    Word* word = (Word*)arraylist_get(phrase_history->phrase, i);
    phraser_Word_vec_push_create(builder, 
      word->word_template_id,
      word->word_template_ordinal,
      str(builder, word->name),
      str(builder, word->word),
      word->permissions,
      word->icon
    );
  }
  phraser_Word_vec_ref_t word_refs = phraser_Word_vec_end(builder);

  phraser_PhraseBlock_history_push_create(builder, 
    history_phrase_template_id, 
    word_refs);
}

void phraseBlock_historyArray_add_all(flatcc_builder_t* builder, 
                                      //arraylist<PhraseHistory>
                                      arraylist* new_history) {
  size_t phrase_history_length = arraylist_size(new_history);
  for (int i = 0; i < phrase_history_length; i++) {
    PhraseHistory* phrase_history = (PhraseHistory*)arraylist_get(new_history, i);
    phraseBlock_historyArray_history(builder, phrase_history);
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
    flatcc_builder_aligned_free(block_buffer);
    flatcc_builder_clear(builder);
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

  return wrapUpBlock(phraser_BlockType_KeyBlock, &builder, block, key_block_key, key_block_mask);
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

  return wrapUpBlock(phraser_BlockType_SymbolSetsBlock, &builder, block, aes_key, aes_iv_mask);
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

  return wrapUpBlock(phraser_BlockType_FoldersBlock, &builder, block, aes_key, aes_iv_mask);
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

  return wrapUpBlock(phraser_BlockType_PhraseTemplatesBlock, &builder, block, aes_key, aes_iv_mask);
}

UpdateResponse updateVersionAndEntropyPhraseBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask, uint32_t new_version,
  uint16_t* phrase_template_id, uint16_t* folder_id, char* phrase_name, bool* is_tombstone, arraylist* new_history//arraylist<PhraseHistory>
) {
  initRandomIfNeeded();
 
  phraser_PhraseBlock_table_t phrase_block;
  if (!(phrase_block = phraser_PhraseBlock_as_root(block + DATA_OFFSET))) {
    return ERROR;
  }

  phraser_StoreBlock_struct_t old_store_block = phraser_PhraseBlock_block(phrase_block);

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_PhraseBlock_start_as_root(&builder);

  if (phrase_template_id == NULL) {
    phraser_PhraseBlock_phrase_template_id_add(&builder, phraser_PhraseBlock_phrase_template_id(phrase_block));
  } else {
    phraser_PhraseBlock_phrase_template_id_add(&builder, *phrase_template_id);
  }
  if (folder_id == NULL) {
    phraser_PhraseBlock_folder_id_add(&builder, phraser_PhraseBlock_folder_id(phrase_block));
  } else {
    phraser_PhraseBlock_folder_id_add(&builder, *folder_id);
  }
  if (phrase_name == NULL) {
    phraser_PhraseBlock_phrase_name_add(&builder, str(&builder, phraser_PhraseBlock_phrase_name(phrase_block)));
  } else {
    phraser_PhraseBlock_phrase_name_add(&builder, str(&builder, phrase_name));
  }
  if (is_tombstone == NULL) {
    phraser_PhraseBlock_is_tombstone_add(&builder, phraser_PhraseBlock_is_tombstone(phrase_block));
  } else {
    phraser_PhraseBlock_is_tombstone_add(&builder, *is_tombstone);
  }
  phraser_PhraseBlock_history_start(&builder);
  if (new_history == NULL) {
    phraser_PhraseHistory_vec_t phrase_history_vec = phraser_PhraseBlock_history(phrase_block);
    phraseBlock_historyVec(&builder, phrase_history_vec);
  } else {
    //arraylist<PhraseHistory>
    phraseBlock_historyArray_add_all(&builder, new_history);
  }
  phraser_PhraseBlock_history_end(&builder);

  phraser_StoreBlock_t* store_block = phraser_PhraseBlock_block_start(&builder);
  storeBlockNewVersionAndEntropy(store_block, old_store_block, new_version);
  phraser_PhraseBlock_block_end(&builder);

  phraser_PhraseBlock_end_as_root(&builder);

  return wrapUpBlock(phraser_BlockType_PhraseBlock, &builder, block, aes_key, aes_iv_mask);
}

UpdateResponse updateVersionAndEntropyPhraseBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask, uint32_t new_version) {
  return updateVersionAndEntropyPhraseBlock(block, block_size, aes_key, aes_iv_mask, new_version, NULL, NULL, NULL, NULL, NULL);
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

static bool perNode(data_t val) { serialDebugPrintf("%u ", val); return false; }
// returns block_number to which the main update shold go
uint16_t throwbackCopy(uint32_t new_version) {
  serialDebugPrintf("t1.\r\n");
  traverse_inorder(occupied_block_numbers(), perNode);

  serialDebugPrintf("t1.\r\n");
  uint32_t border_block_number = last_block_number();
  serialDebugPrintf("t2 border_block_number %d.\r\n", border_block_number);
  uint32_t b1_block_number = get_valid_block_number_on_the_right_of(occupied_block_numbers(), border_block_number);
  serialDebugPrintf("t3 b1_block_number %d.\r\n", b1_block_number);

  // ---- find B2 and B3 ----

  uint32_t r1_block_number = (border_block_number + 1) % db_block_count();
  uint32_t r2_block_number = (border_block_number + 2) % db_block_count();
  node_t* r1_node = tree_search(occupied_block_numbers(), r1_block_number);
  bool r1_occupied = (r1_node != NULL);
  node_t* r2_node = tree_search(occupied_block_numbers(), r2_block_number);
  bool r2_occupied = (r2_node != NULL);

  serialDebugPrintf("t3 r1_block_number %d.\r\n", r1_block_number);
  serialDebugPrintf("t3 r1_occupied %d.\r\n", r1_occupied);
  serialDebugPrintf("t3 r2_block_number %d.\r\n", r2_block_number);
  serialDebugPrintf("t3 r2_occupied %d.\r\n", r2_occupied);

  uint32_t b2_block_number;
  uint32_t b3_block_number;

  if (!r1_occupied && !r2_occupied) {
    // Happy path - whenever R1 and R2 are free, use them as B2 and B3
    b2_block_number = r1_block_number;
    b3_block_number = r2_block_number;
  } else {
    b2_block_number = get_free_block_number_on_the_left_of(occupied_block_numbers(), border_block_number, db_block_count());
    serialDebugPrintf("t3 calculated b2_block_number %d.\r\n", b2_block_number);

    bool b2_is_closer_to_border_from_the_right_than_b1 = false;

    uint32_t df = db_block_count() - border_block_number - 1;
    uint32_t b1_block_number_adj = (b1_block_number + df) % db_block_count();
    uint32_t b2_block_number_adj = (b2_block_number + df) % db_block_count();

    b2_is_closer_to_border_from_the_right_than_b1 = b2_block_number_adj < b1_block_number_adj;

    serialDebugPrintf("t3 b2_is_closer_to_border_from_the_right_than_b1 %d.\r\n", b2_is_closer_to_border_from_the_right_than_b1);
  
    if (b2_is_closer_to_border_from_the_right_than_b1) {
      // Calculated B2 is closer to the right of the Last Block than B1
      b2_block_number = r1_block_number;
      b3_block_number = r2_block_number;
    } else {
      // Fall back to throwback copy
      b3_block_number = r1_block_number;
    }
  }

  serialDebugPrintf("t3 b2_block_number %d.\r\n", b2_block_number);
  serialDebugPrintf("t3 b3_block_number %d.\r\n", b3_block_number);

  // -------------------------------------------------------------------------------------------------

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

  // 2. Update B1 version - block getting encrypted here
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
  // B1-related logic, new version of which replaced B2
  serialDebugPrintf("t12.\r\n");

  // 5. Re-Load new block version from flash, to make sure our cache is consistent with persisted content
  if (!loadBlockFromFlash(bank_number, b2_block_number, block_size, aes_key, aes_iv_mask, block)) {
    return ERROR;
  }
  serialDebugPrintf("5.\r\n");

  // 6. Register new version in cache
  registerBlockInBlockCache(block, b2_block_number);

  // ---- Return block_number to which Main Update shold go ----
  serialDebugPrintf("t13 b3_block_number %d.\r\n", b3_block_number);
  return b3_block_number;
}

// -------------------- FOLDERS --------------------

UpdateResponse folderMutation(arraylist* (*func)(phraser_Folder_vec_t* folders_vec)) {
  initRandomIfNeeded();

  if (!db_has_free_blocks()) {
    // turn tombstoned blocks into free blocks
    nuke_tombstone_blocks();
  }
  if (!db_has_free_blocks()) {
    // DB integriy Error - at least one free or tombstoned block should be present at all times
    return ERROR;
  }

  // 1. load folders block
  uint16_t folder_block_number = get_folders_block_number();
  uint16_t block_size = FLASH_SECTOR_SIZE;
  uint8_t block[block_size];
  serialDebugPrintf("2. loading bank_number %d, folder_block_number %d\r\n", bank_number, folder_block_number);
  if (!loadBlockFromFlash(bank_number, folder_block_number, block_size,
    main_key, main_iv_mask, 
    block)) {
    return ERROR;
  }

  // 2. deserialize folders flatbuf block
  phraser_FoldersBlock_table_t folders_block;
  if (!(folders_block = phraser_FoldersBlock_as_root(block + DATA_OFFSET))) {
    return ERROR;
  }
  serialDebugPrintf("3.\r\n");

  // 3. Form arraylist of existing block folders
  phraser_Folder_vec_t folders_vec = phraser_FoldersBlock_folders(folders_block);

  arraylist* dao_folders = func(&folders_vec);

  serialDebugPrintf("4.\r\n");
  serialDebugPrintf("5.\r\n");

  uint32_t throwback_copy_version = increment_and_get_next_block_version();//throwback version comes first
  uint32_t new_block_version = increment_and_get_next_block_version();

  // 4. Form updated block with updated list of folders
  UpdateResponse block_response = updateVersionAndEntropyFoldersBlock(block, block_size, main_key, main_iv_mask, new_block_version, dao_folders);
  serialDebugPrintf("6.\r\n");

  // 5. Free arraylist of folders
  for (int i = 0; i < arraylist_size(dao_folders); i++) {
    free(arraylist_get(dao_folders, i)); 
  }
  free(dao_folders);

  if (OK != block_response) {
    return block_response;
  }
  serialDebugPrintf("7.\r\n");

  // 6. Perform Throwback Copy, according to flash preservation algorithm
  uint16_t b3_block_number = throwbackCopy(throwback_copy_version);
  if (b3_block_number == -1) {
    return ERROR;
  }
  serialDebugPrintf("8.\r\n");

  // 7. Save the updated block to flash
  saveBlockUpdateToFlash(bank_number, b3_block_number, block, block_size);
  serialDebugPrintf("9.\r\n");

  // 8. Re-Load new block version from flash, to make sure our cache is consistent with persisted content
  if (!loadBlockFromFlash(bank_number, b3_block_number, block_size,
    main_key, main_iv_mask, 
    block)) {
    return ERROR;
  }
  serialDebugPrintf("10.\r\n");

  // 9. Update DB indices

  // B3-related logic, it was replaced at B3 block_number by B0 blockId
  // 10. Register new version in cache
  registerBlockInBlockCache(block, b3_block_number);
  serialDebugPrintf("12.\r\n");

  return OK;
}

uint16_t add_f_m_out_new_folder_id;
uint16_t add_f_m_parent_folder_id;
char* add_f_m_new_folder_name;
arraylist* addFolderMutation(phraser_Folder_vec_t* folders_vec) {
  uint16_t max_folder_id = 0;
  arraylist* dao_folders = arraylist_create();
  size_t folders_vec_length = flatbuffers_vec_len(*folders_vec);
  for (int i = 0; i < folders_vec_length; i++) {
    phraser_Folder_table_t folder_fb = phraser_Folder_vec_at(*folders_vec, i);

    DAOFolder* daoFolder = (DAOFolder*)malloc(sizeof(DAOFolder));
    daoFolder->folder_id = phraser_Folder_folder_id(folder_fb);
    daoFolder->parent_folder_id = phraser_Folder_parent_folder_id(folder_fb);
    daoFolder->folder_name = (char*)phraser_Folder_folder_name(folder_fb);

    if (daoFolder->folder_id > max_folder_id) {
      max_folder_id = daoFolder->folder_id;
    }

    arraylist_add(dao_folders, daoFolder);
  }

  // 5. Add new folder to the arraylist
  DAOFolder* daoFolder = (DAOFolder*)malloc(sizeof(DAOFolder));
  daoFolder->folder_id = max_folder_id + 1;
  add_f_m_out_new_folder_id = max_folder_id + 1;
  daoFolder->parent_folder_id = add_f_m_parent_folder_id;
  daoFolder->folder_name = add_f_m_new_folder_name;

  arraylist_add(dao_folders, daoFolder);

  return dao_folders;
}

UpdateResponse addNewFolder(char* new_folder_name, uint16_t parent_folder_id, uint16_t* out_new_folder_id) {
  add_f_m_new_folder_name = new_folder_name;
  add_f_m_parent_folder_id = parent_folder_id;
  UpdateResponse update_response = folderMutation(addFolderMutation);

  *out_new_folder_id = add_f_m_out_new_folder_id;
  return update_response;
}

uint16_t mov_f_m_folder_id;
uint16_t mov_f_m_to_folder_id;
arraylist* moveFolderMutation(phraser_Folder_vec_t* folders_vec) {
  arraylist* dao_folders = arraylist_create();
  size_t folders_vec_length = flatbuffers_vec_len(*folders_vec);
  for (int i = 0; i < folders_vec_length; i++) {
    phraser_Folder_table_t folder_fb = phraser_Folder_vec_at(*folders_vec, i);

    uint16_t folder_id = phraser_Folder_folder_id(folder_fb);
    DAOFolder* daoFolder = (DAOFolder*)malloc(sizeof(DAOFolder));
    daoFolder->folder_id = folder_id;
    daoFolder->parent_folder_id = mov_f_m_folder_id == folder_id ? mov_f_m_to_folder_id : phraser_Folder_parent_folder_id(folder_fb);
    daoFolder->folder_name = (char*)phraser_Folder_folder_name(folder_fb);

    arraylist_add(dao_folders, daoFolder);
  }

  return dao_folders;
}

UpdateResponse moveFolder(uint16_t move_folder_id, uint16_t to_folder_id) {
  mov_f_m_folder_id = move_folder_id;
  mov_f_m_to_folder_id = to_folder_id;
  return folderMutation(moveFolderMutation);
}

uint16_t ren_f_m_folder_id;
char* ren_f_m_new_folder_name;
arraylist* renameFolderMutation(phraser_Folder_vec_t* folders_vec) {
  arraylist* dao_folders = arraylist_create();
  size_t folders_vec_length = flatbuffers_vec_len(*folders_vec);
  for (int i = 0; i < folders_vec_length; i++) {
    phraser_Folder_table_t folder_fb = phraser_Folder_vec_at(*folders_vec, i);

    uint16_t folder_id = phraser_Folder_folder_id(folder_fb);
    DAOFolder* daoFolder = (DAOFolder*)malloc(sizeof(DAOFolder));
    daoFolder->folder_id = folder_id;
    daoFolder->parent_folder_id = phraser_Folder_parent_folder_id(folder_fb);
    daoFolder->folder_name = ren_f_m_folder_id == folder_id ? ren_f_m_new_folder_name : (char*)phraser_Folder_folder_name(folder_fb);

    arraylist_add(dao_folders, daoFolder);
  }

  return dao_folders;
}

UpdateResponse renameFolder(uint16_t folder_id, char* new_folder_name) {
  ren_f_m_new_folder_name = new_folder_name;
  ren_f_m_folder_id = folder_id;
  return folderMutation(renameFolderMutation);
}

uint16_t del_f_m_folder_id;
arraylist* deleteFolderMutation(phraser_Folder_vec_t* folders_vec) {
  arraylist* dao_folders = arraylist_create();
  size_t folders_vec_length = flatbuffers_vec_len(*folders_vec);
  for (int i = 0; i < folders_vec_length; i++) {
    phraser_Folder_table_t folder_fb = phraser_Folder_vec_at(*folders_vec, i);

    uint16_t folder_id = phraser_Folder_folder_id(folder_fb);
    if (del_f_m_folder_id != folder_id) {
      DAOFolder* daoFolder = (DAOFolder*)malloc(sizeof(DAOFolder));
      daoFolder->folder_id = folder_id;
      daoFolder->parent_folder_id = phraser_Folder_parent_folder_id(folder_fb);
      daoFolder->folder_name = (char*)phraser_Folder_folder_name(folder_fb);
  
      arraylist_add(dao_folders, daoFolder);
    }
  }

  return dao_folders;
}

UpdateResponse deleteFolder(uint16_t folder_id) {
  del_f_m_folder_id = folder_id;
  return folderMutation(deleteFolderMutation);
}

// -------------------- PHRASES -------------------- 

char* generateWordOrReturnEmptyStr(WordTemplate* word_template) {
  if (isGenerateable(word_template->permissions)) {
    int symbol_set_size = arraylist_size(word_template->symbolSetIds);
    if (symbol_set_size > 0) {
      // Create a unique character set
      char uniqueSet[256] = {0}; // Assuming ASCII characters
      size_t uniqueCount = 0;

      for (int j = 0; j < symbol_set_size; j++) {
        uint16_t symbol_set_id = (uint32_t)arraylist_get(word_template->symbolSetIds, j);
        SymbolSet* symbolSet = getSymbolSet(symbol_set_id);
        for (const char* p = symbolSet->symbolSet; *p != '\0'; p++) {
          if (!uniqueSet[(unsigned char)*p]) {
            uniqueSet[(unsigned char)*p] = 1;
            uniqueCount++;
          }
        }
      }

      if (uniqueCount > 0) {
        char* combinedSet = (char*)malloc(uniqueCount);
        size_t index = 0;
        for (int i = 0; i < 256; i++) {
            if (uniqueSet[i]) {
                combinedSet[index++] = (char)i;
            }
        }

        uint32_t s1 = word_template->minLength;
        uint32_t s2 = word_template->maxLength;
        int resultLength = s1 + random(s2 - s1 + 1);

        char* resultString = (char*)malloc(resultLength + 1);
        for (int i = 0; i < resultLength; i++) {
          int randomIndex = random(uniqueCount);
          resultString[i] = combinedSet[randomIndex];
        }
        resultString[resultLength] = '\0';

        free(combinedSet);

        return resultString;
      }
    }
  }
  
  char* ret_val = (char*)malloc(sizeof(char));
  ret_val[0] = '\0';
  return ret_val;
}

// fill buffer with encrypted block ready for persistence on flash
UpdateResponse initDefaultPhraseBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask, 
                                      uint16_t phrase_template_id, uint16_t folder_id, char* phraseName,
                                      uint16_t block_id, uint32_t version) {
  // get phrase template from BlockCache
  PhraseTemplate* phrase_template = getPhraseTemplate(phrase_template_id);
  if (phrase_template == NULL) {
    return ERROR;
  }

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_PhraseBlock_start_as_root(&builder);

  phraser_PhraseBlock_phrase_template_id_add(&builder, phrase_template_id);
  phraser_PhraseBlock_folder_id_add(&builder, folder_id);
  phraser_PhraseBlock_phrase_name_add(&builder, str(&builder, phraseName));
  phraser_PhraseBlock_is_tombstone_add(&builder, false);

  phraser_PhraseBlock_history_start(&builder);
  phraser_Word_vec_start(&builder);

  // get word templates list for phrase template
  for (int i = 0; i < arraylist_size(phrase_template->wordTemplateIds); i++) {
    uint16_t word_template_id = (uint32_t)arraylist_get(phrase_template->wordTemplateIds, i);
    uint16_t word_template_ordinal = (uint32_t)arraylist_get(phrase_template->wordTemplateOrdinals, i);

    WordTemplate* word_template = getWordTemplate(word_template_id);
    if (word_template == NULL) {
      return ERROR;
    }

    // generate generateable words, create non-generateable words empty
    char* word_value = generateWordOrReturnEmptyStr(word_template);
    phraseBlock_history(&builder, word_template_id, word_template_ordinal, word_template->wordTemplateName, 
      word_value, word_template->permissions, word_template->icon);
    free(word_value);
  }

  phraser_Word_vec_ref_t word_refs = phraser_Word_vec_end(&builder);
  phraser_PhraseBlock_history_push_create(&builder, 1, word_refs);
  phraser_PhraseBlock_history_end(&builder);

  uint32_t entropy = random_uint32();
  phraser_StoreBlock_t* store_block;
  store_block = phraser_PhraseBlock_block_start(&builder);
  store_block->block_id = block_id;
  store_block->version = version;
  store_block->entropy = entropy;
  phraser_PhraseBlock_block_end(&builder);

  phraser_PhraseBlock_end_as_root(&builder);

  void *block_buffer;
  size_t block_buffer_size;
  block_buffer = flatcc_builder_finalize_aligned_buffer(&builder, &block_buffer_size);

  wrapDataBufferInBlock(phraser_BlockType_PhraseBlock, buffer, aes_key, aes_iv_mask, block_buffer, block_buffer_size);

  flatcc_builder_aligned_free(block_buffer);
  flatcc_builder_clear(&builder);

  return OK;
}

PhraseHistory* convertPhraseHistory(phraser_PhraseHistory_table_t *history);
// Just copy without modifications
//arraylist<PhraseHistory>
arraylist* copyPhraseHistoryMutation(phraser_PhraseHistory_vec_t phrase_history_vec) {
  arraylist* full_phrase_history = arraylist_create();

  size_t old_phrase_history_vec_length = flatbuffers_vec_len(phrase_history_vec);
  for (int i = 0; i < old_phrase_history_vec_length; i++) {
    phraser_PhraseHistory_table_t history = phraser_PhraseHistory_vec_at(phrase_history_vec, i);
    PhraseHistory* phrase_history = convertPhraseHistory(&history);
    arraylist_add(full_phrase_history, phrase_history);
  }

  return full_phrase_history;
}

// If phrase_block_id = -1, it's a new phrase
UpdateResponse phraseMutation(int phrase_block_id, 
                              uint16_t (*phraseTemplateIdMutation)(uint16_t phrase_template_id),
                              uint16_t (*folderIdMutation)(uint16_t folder_id),
                              char* (*phraseNameMutation)(flatbuffers_string_t phraseName),
                              bool (*tombstoneMutation)(bool tombstone),
                              //arraylist<PhraseHistory>
                              arraylist* (*phraseHistoryMutation)(phraser_PhraseHistory_vec_t history_vec),
                              bool auto_truncate_history
                            ) {
  initRandomIfNeeded();
  bool is_new_phrase = phrase_block_id <= -1;

  // 2. if not a new phrase, load phrase block
  uint32_t throwback_copy_version;

  uint16_t block_size = FLASH_SECTOR_SIZE;
  uint8_t block[block_size];
  uint32_t old_phrase_block_number;

  if (!db_has_free_blocks()) {
    // turn tombstoned blocks into free blocks
    nuke_tombstone_blocks();
  }
  if (!db_has_free_blocks()) {
    // DB integriy Error - at least one free or tombstoned block should be present at all times
    return ERROR;
  }

  if (is_new_phrase) {
    // 1. check that db capacity is enough to add new block
    serialDebugPrintf("1.\r\n");
    if (db_full()) {
      return DB_FULL;
    }
    if (!db_has_non_tombstoned_space()) {
    // The first call to nuke_tombstone_blocks might fail to recover space due to 
    // `tombstoned block with 2 copies` phenomenon, so we will repeat this call after throwback copy,
    // because throwback copy will always overwrite the 2nd copy residing in the free block. 
    nuke_tombstone_blocks();
    }

    // Create new PhraseBlock
    uint16_t phrase_template_id = phraseTemplateIdMutation(-1);
    uint16_t folder_id = folderIdMutation(-1);
    char* phraseName = phraseNameMutation(NULL);

    throwback_copy_version = increment_and_get_next_block_version();
    uint32_t new_block_version = increment_and_get_next_block_version();
    phrase_block_id = increment_and_get_next_block_id();

    UpdateResponse block_init_response = initDefaultPhraseBlock(block, main_key, main_iv_mask, 
                                                    phrase_template_id, folder_id, phraseName,
                                                    phrase_block_id, new_block_version);

    if (block_init_response != OK) {
      return block_init_response;
    }
  } else {
    // Load existing PhraseBlock and apply mutations
    old_phrase_block_number = get_phrase_block_number(phrase_block_id);
    serialDebugPrintf("2. loading bank_number %d, phrase_block_id %d\r\n", bank_number, old_phrase_block_number);
    if (!loadBlockFromFlash(bank_number, old_phrase_block_number, block_size,
      main_key, main_iv_mask, 
      block)) {
      return ERROR;
    }

    // 3. deserialize phrase flatbuf block
    phraser_PhraseBlock_table_t phrase_block;
    if (!(phrase_block = phraser_PhraseBlock_as_root(block + DATA_OFFSET))) {
      return ERROR;
    }
    serialDebugPrintf("3.\r\n");

    // 4. Form params
    uint16_t phrase_template_id_raw;
    uint16_t folder_id_raw;
    bool is_tombstone_raw;
    uint16_t* phrase_template_id = NULL;
    uint16_t* folder_id = NULL;
    bool* is_tombstone = NULL;
    char* phrase_name = NULL;
    arraylist* new_history = NULL;
    
    if (phraseTemplateIdMutation != NULL) {
      phrase_template_id_raw = phraseTemplateIdMutation(phraser_PhraseBlock_phrase_template_id(phrase_block));
      phrase_template_id = &phrase_template_id_raw;
    }
    if (folderIdMutation != NULL) {
      folder_id_raw = folderIdMutation(phraser_PhraseBlock_folder_id(phrase_block));
      folder_id = &folder_id_raw;
    }
    if (phraseNameMutation != NULL) {
      phrase_name = phraseNameMutation(phraser_PhraseBlock_phrase_name(phrase_block));
    }
    if (tombstoneMutation != NULL) {
      is_tombstone_raw = tombstoneMutation(phraser_PhraseBlock_is_tombstone(phrase_block));
      is_tombstone = &is_tombstone_raw;
    }
    phraser_PhraseHistory_vec_t phrase_history_vec = phraser_PhraseBlock_history(phrase_block);
    if (phraseHistoryMutation != NULL) {
      new_history = phraseHistoryMutation(phrase_history_vec);
    } else {
      // We need this for auto-truncate
      new_history = copyPhraseHistoryMutation(phrase_history_vec);
    }

    // 4.1 Form versions
    throwback_copy_version = increment_and_get_next_block_version();//throwback version comes first
    uint32_t new_block_version = increment_and_get_next_block_version();
  
    // 6. Form updated block with updates
    UpdateResponse block_response;
    while (true) {
      block_response = updateVersionAndEntropyPhraseBlock(block, block_size, main_key, main_iv_mask, new_block_version,
        phrase_template_id, folder_id, phrase_name, is_tombstone, new_history);
      
      // Auto-truncate logic
      if (!auto_truncate_history) { break; } // auto-truncate not requested
      if (block_response != BLOCK_SIZE_EXCEEDED) { break; } // block size not exceeded
      unsigned int new_history_size = arraylist_size(new_history);
      if (new_history_size <= 1) { break; } // can't truncate current history

      // truncate oldest history
      PhraseHistory* phrase_history = (PhraseHistory*)arraylist_remove(new_history, new_history_size-1);
      releaseFullPhraseHistory(phrase_history);
    }
    
    serialDebugPrintf("6.\r\n");

    // 7. Release mutations resources
    if (new_history != NULL) {
      for (int i = 0; i < arraylist_size(new_history); i++) {
        PhraseHistory* phrase_history = (PhraseHistory*)arraylist_get(new_history, i);
        releaseFullPhraseHistory(phrase_history);
      }
      arraylist_destroy(new_history);
    }
    // phrase_name comes from ScreenKeyboard so free is not needed

    if (OK != block_response) {
      return block_response;
    }
    serialDebugPrintf("7.\r\n");
  }

  // 8. Perform Throwback Copy, according to flash preservation algorithm
  uint16_t b3_block_number = throwbackCopy(throwback_copy_version);
  if (b3_block_number == -1) {
    return ERROR;
  }
  serialDebugPrintf("8.\r\n");

  // Nuke tombstones again to workaround "tombstone with 2 copies" phenomenon
  if (!db_has_non_tombstoned_space()) {
    nuke_tombstone_blocks();
  }
  if (!db_has_non_tombstoned_space()) {
    return ERROR;
  }

  // 9. Save the updated block to flash
  saveBlockUpdateToFlash(bank_number, b3_block_number, block, block_size);
  serialDebugPrintf("9.\r\n");

  // 10. Re-Load new block version from flash, to make sure our cache is consistent with persisted content
  if (!loadBlockFromFlash(bank_number, b3_block_number, block_size,
    main_key, main_iv_mask, 
    block)) {
    return ERROR;
  }
  serialDebugPrintf("10.\r\n");

  // 11. Register new version in cache
  registerBlockInBlockCache(block, b3_block_number);
  serialDebugPrintf("11.\r\n");

  return OK;
}

uint16_t new_phrase_template_id;
uint16_t phrase_template_id_mutation(uint16_t _phrase_template_id) {
  return new_phrase_template_id;
} 

uint16_t new_folder_id;
uint16_t folder_id_mutation(uint16_t _folder_id) {
  return new_folder_id;
}

char* new_phrase_name;
char* phrase_name_mutation(flatbuffers_string_t _phraseName) {
  return new_phrase_name;
}

bool new_prase_tombstone;
bool tombstone_mutation(bool tombstone) {
  return new_prase_tombstone;
}

UpdateResponse addNewPhrase(char* phrase_name, uint16_t phrase_template_id, uint16_t folder_id, uint16_t* created_phrase_id) {
  new_phrase_name = phrase_name;
  new_phrase_template_id = phrase_template_id;
  new_folder_id = folder_id;

  UpdateResponse update_response = phraseMutation(-1, 
    phrase_template_id_mutation,
    folder_id_mutation,
    phrase_name_mutation,
    NULL,
    NULL,
    false
  );
  *created_phrase_id = last_block_id(); 
  return update_response;
}

UpdateResponse deletePhrase(uint16_t phrase_block_id) {
  new_prase_tombstone = true;

  return phraseMutation(phrase_block_id, 
    NULL,
    NULL,
    NULL,
    tombstone_mutation,
    NULL,
    false
  );
  return OK;
}

UpdateResponse movePhrase(uint16_t phrase_block_id, uint16_t move_to_folder_id) {
  new_folder_id = move_to_folder_id;

  return phraseMutation(phrase_block_id, 
    NULL,
    folder_id_mutation,
    NULL,
    NULL,
    NULL,
    false
  );
}

UpdateResponse renamePhrase(uint16_t phrase_block_id, char* update_phrase_name) {
  new_phrase_name = update_phrase_name;

  return phraseMutation(phrase_block_id, 
    NULL,
    NULL,
    phrase_name_mutation,
    NULL,
    NULL,
    false
  );
}

UpdateResponse changePhraseTemplate(uint16_t phrase_block_id, uint16_t phrase_template_id) {
  new_phrase_template_id = phrase_template_id;

  UpdateResponse update_response = phraseMutation(phrase_block_id, 
    phrase_template_id_mutation,
    NULL,
    NULL,
    NULL,
    NULL,
    false
  );
  return update_response;
}

PhraseHistory* convertPhraseHistory(phraser_PhraseHistory_table_t *history);
Word* createWord(uint16_t word_template_id, uint8_t word_template_ordinal, 
  char* name, int name_length,  char* word, int word_length, uint8_t permissions, phraser_Icon_enum_t icon);

PhraseHistory* createNewPhraseHistory(phraser_PhraseHistory_table_t *history,
  uint16_t new_phrase_template_id, uint16_t new_word_template_id, uint8_t new_word_template_ordinal, char* new_word, uint16_t new_word_length) {
  // Deserialize supplied history
  PhraseHistory* old_history0 = convertPhraseHistory(history);

  PhraseHistory* new_history = (PhraseHistory*)malloc(sizeof(PhraseHistory));
  new_history->phrase_template_id = new_phrase_template_id;
  new_history->phrase = arraylist_create();

  PhraseTemplate* phrase_template = getPhraseTemplate(new_phrase_template_id);
  for (int i = 0; i < arraylist_size(phrase_template->wordTemplateIds); i++) {
    uint16_t word_template_id = (uint32_t)arraylist_get(phrase_template->wordTemplateIds, i);
    uint8_t word_template_ordinal = (uint32_t)arraylist_get(phrase_template->wordTemplateOrdinals, i);
    WordTemplate* word_template = getWordTemplate(word_template_id);
    if (word_template == NULL) {
      //TODO: error handling
      continue;
    }

    Word* matching_word = NULL;
    if (word_template_id == new_word_template_id && word_template_ordinal == new_word_template_ordinal) {
      matching_word = createWord(word_template_id, word_template_ordinal, word_template->wordTemplateName, strlen(word_template->wordTemplateName),
        new_word, new_word_length, word_template->permissions, word_template->icon);
    } else {
      //TODO: optimizable with hashtable
      int matching_word_id = -1;
      for (int j = 0; j < arraylist_size(old_history0->phrase); j++) {
        Word* word = (Word*)arraylist_get(old_history0->phrase, j);
        if (word->word_template_id == word_template_id && word->word_template_ordinal == word_template_ordinal) {
          matching_word_id = j;
        }
      }

      if (matching_word_id != -1) { matching_word = (Word*)arraylist_remove(old_history0->phrase, matching_word_id); }
    }

    if (matching_word == NULL) {
      // generate generateable words, create non-generateable words empty
      char* word_value = generateWordOrReturnEmptyStr(word_template);
      matching_word = createWord(word_template_id, word_template_ordinal, word_template->wordTemplateName, strlen(word_template->wordTemplateName),
        word_value, strlen(word_value), word_template->permissions, word_template->icon);
      free(word_value);
    }

    arraylist_add(new_history->phrase, matching_word);
  }

  releaseFullPhraseHistory(old_history0);

  return new_history;
}

uint16_t new_history_phrase_template_id;
uint16_t new_history_word_template_id;
uint8_t new_history_word_template_ordinal;
char* new_history_new_word;
uint16_t new_history_new_word_length;
//arraylist<PhraseHistory>
arraylist* new_phrase_history_mutation(phraser_PhraseHistory_vec_t phrase_history_vec) {
  arraylist* full_phrase_history = arraylist_create();

  // TODO: add truncate logic
  size_t old_phrase_history_vec_length = flatbuffers_vec_len(phrase_history_vec);
  for (int i = 0; i < old_phrase_history_vec_length; i++) {
    phraser_PhraseHistory_table_t history = phraser_PhraseHistory_vec_at(phrase_history_vec, i);

    if (i == 0) {
      // add new history first
      PhraseHistory* new_phrase_history = createNewPhraseHistory(&history, new_history_phrase_template_id, new_history_word_template_id, new_history_word_template_ordinal, 
        new_history_new_word, new_history_new_word_length);
      arraylist_add(full_phrase_history, new_phrase_history);
    }

    PhraseHistory* phrase_history = convertPhraseHistory(&history);
    arraylist_add(full_phrase_history, phrase_history);
  }

  return full_phrase_history;
}

UpdateResponse updatePhraseWord(uint16_t phrase_block_id, uint16_t phrase_template_id, uint16_t word_template_id, uint8_t word_template_ordinal, char* new_word, uint16_t new_word_length, bool auto_truncate_history) {
  initRandomIfNeeded();

  new_history_phrase_template_id = phrase_template_id;
  new_history_word_template_id = word_template_id; 
  new_history_word_template_ordinal =word_template_ordinal;
  new_history_new_word = new_word;
  new_history_new_word_length = new_word_length;
  
  UpdateResponse update_response = phraseMutation(phrase_block_id, 
    NULL,
    NULL,
    NULL,
    NULL,
    new_phrase_history_mutation,
    auto_truncate_history
  );
  return update_response;
}

UpdateResponse generatePhraseWord(uint16_t phrase_block_id, uint16_t phrase_template_id, uint16_t word_template_id, uint8_t word_template_ordinal, bool auto_truncate_history) {
  WordTemplate* word_template = getWordTemplate(word_template_id);
  if (word_template == NULL) {
    return ERROR;
  }

  if (!isGenerateable(word_template->permissions)) {
    return ERROR;
  }

  // generate generateable word, create non-generateable words empty
  char* new_word = generateWordOrReturnEmptyStr(word_template);
  UpdateResponse update_response = updatePhraseWord(phrase_block_id, phrase_template_id, word_template_id, word_template_ordinal, new_word, strlen(new_word), auto_truncate_history);
  free(new_word);
  return update_response;
}

UpdateResponse userEditPhraseWord(uint16_t phrase_block_id, uint16_t phrase_template_id, uint16_t word_template_id, uint8_t word_template_ordinal, char* new_word, uint16_t new_word_length, bool auto_truncate_history) {
  WordTemplate* word_template = getWordTemplate(word_template_id);
  if (word_template == NULL) {
    return ERROR;
  }

  if (!isUserEditable(word_template->permissions)) {
    return ERROR;
  }

  return updatePhraseWord(phrase_block_id, phrase_template_id, word_template_id, word_template_ordinal, new_word, new_word_length, auto_truncate_history);
}

uint16_t delete_phrase_history_index;
//arraylist<PhraseHistory>
arraylist* delete_phrase_history_mutation(phraser_PhraseHistory_vec_t phrase_history_vec) {
  arraylist* full_phrase_history = arraylist_create();

  size_t old_phrase_history_vec_length = flatbuffers_vec_len(phrase_history_vec);
  for (int i = 0; i < old_phrase_history_vec_length; i++) {
    phraser_PhraseHistory_table_t history = phraser_PhraseHistory_vec_at(phrase_history_vec, i);

    // Add all history except for the deleted
    if (i != delete_phrase_history_index) {
      PhraseHistory* phrase_history = convertPhraseHistory(&history);
      arraylist_add(full_phrase_history, phrase_history);
    }
  }

  return full_phrase_history;
}

UpdateResponse deletePhraseHistory(uint16_t phrase_block_id, uint16_t phrase_history_index) {
  if (phrase_history_index == 0) { return ERROR; }

  initRandomIfNeeded();

  delete_phrase_history_index = phrase_history_index;
  
  UpdateResponse update_response = phraseMutation(phrase_block_id, 
    NULL,
    NULL,
    NULL,
    NULL,
    delete_phrase_history_mutation,
    false
  );
  return update_response;
}

uint16_t make_current_phrase_history_index;
//arraylist<PhraseHistory>
arraylist* make_phrase_history_current_mutation(phraser_PhraseHistory_vec_t phrase_history_vec) {
  arraylist* full_phrase_history = arraylist_create();

  size_t old_phrase_history_vec_length = flatbuffers_vec_len(phrase_history_vec);
  if (make_current_phrase_history_index < old_phrase_history_vec_length) {
    phraser_PhraseHistory_table_t history = phraser_PhraseHistory_vec_at(phrase_history_vec, make_current_phrase_history_index);

    // First add the specified history
    PhraseHistory* phrase_history = convertPhraseHistory(&history);
    arraylist_add(full_phrase_history, phrase_history);
  }

  for (int i = 0; i < old_phrase_history_vec_length; i++) {
    phraser_PhraseHistory_table_t history = phraser_PhraseHistory_vec_at(phrase_history_vec, i);

    // Then add all history except for the one moved up
    if (i != make_current_phrase_history_index) {
      PhraseHistory* phrase_history = convertPhraseHistory(&history);
      arraylist_add(full_phrase_history, phrase_history);
    }
  }

  return full_phrase_history;
}

UpdateResponse makePhraseHistoryCurrent(uint16_t phrase_block_id, uint16_t phrase_history_index) {
  if (phrase_history_index == 0) { return ERROR; }

  initRandomIfNeeded();

  make_current_phrase_history_index = phrase_history_index;
  
  UpdateResponse update_response = phraseMutation(phrase_block_id, 
    NULL,
    NULL,
    NULL,
    NULL,
    make_phrase_history_current_mutation,
    false
  );
  return update_response;
}

//arraylist<PhraseHistory>
arraylist* clear_phrase_history_mutation(phraser_PhraseHistory_vec_t phrase_history_vec) {
  size_t old_phrase_history_vec_length = flatbuffers_vec_len(phrase_history_vec);
  if (old_phrase_history_vec_length > 0) {
    arraylist* full_phrase_history = arraylist_create();
    phraser_PhraseHistory_table_t history = phraser_PhraseHistory_vec_at(phrase_history_vec, 0);

    // Add history 0
    PhraseHistory* phrase_history = convertPhraseHistory(&history);
    arraylist_add(full_phrase_history, phrase_history);
    return full_phrase_history;
  }
  return NULL;
}

UpdateResponse clearPhraseHistory(uint16_t phrase_block_id) {
  initRandomIfNeeded();
  
  UpdateResponse update_response = phraseMutation(phrase_block_id, 
    NULL,
    NULL,
    NULL,
    NULL,
    clear_phrase_history_mutation,
    false
  );
  return update_response;
}

// ------------------------------------------------------

Word* createWord(uint16_t word_template_id, uint8_t word_template_ordinal, 
  char* name, int name_length,  char* word, int word_length, uint8_t permissions, phraser_Icon_enum_t icon) {
  Word* word_obj = (Word*)malloc(sizeof(Word));

  word_obj->word_template_id = word_template_id;
  word_obj->word_template_ordinal = word_template_ordinal;
  word_obj->name = copyString(name, name_length);
  word_obj->word = copyString(word, word_length);
  word_obj->permissions = permissions;
  word_obj->icon = icon;

  return word_obj;
}

Word* convertWord(phraser_Word_table_t *word_fb) {
  flatbuffers_string_t word_name = phraser_Word_name(*word_fb);
  flatbuffers_string_t word_word = phraser_Word_word(*word_fb);

  Word* word = createWord(phraser_Word_word_template_id(*word_fb), phraser_Word_word_template_ordinal(*word_fb), (char*)word_name, 
        flatbuffers_string_len(word_name), (char*)word_word, flatbuffers_string_len(word_word), phraser_Word_permissions(*word_fb), 
        phraser_Word_icon(*word_fb));

  return word;
}

PhraseHistory* convertPhraseHistory(phraser_PhraseHistory_table_t *history) {
  PhraseHistory* phrase_history = (PhraseHistory*)malloc(sizeof(PhraseHistory));
  phrase_history->phrase_template_id = phraser_PhraseHistory_phrase_template_id(*history);
  phrase_history->phrase = arraylist_create();

  phraser_Word_vec_t words_vec = phraser_PhraseHistory_phrase(*history);
  size_t words_vec_length = flatbuffers_vec_len(words_vec);

  for (int i = 0; i < words_vec_length; i++) {
    phraser_Word_table_t word_fb = phraser_Word_vec_at(words_vec, i);

    Word* word = convertWord(&word_fb);
    arraylist_add(phrase_history->phrase, word);
  }

  return phrase_history;
}

arraylist* convertPhraseHistoryArray(phraser_PhraseHistory_vec_t phrase_history_vec) {
  arraylist* full_phrase_history = arraylist_create();

  size_t old_phrase_history_vec_length = flatbuffers_vec_len(phrase_history_vec);
  for (int i = 0; i < old_phrase_history_vec_length; i++) {
    phraser_PhraseHistory_table_t history = phraser_PhraseHistory_vec_at(phrase_history_vec, i);

    PhraseHistory* phrase_history = convertPhraseHistory(&history);
    arraylist_add(full_phrase_history, phrase_history);
  }

  return full_phrase_history;
}

FullPhrase* getFullPhrase(uint16_t full_phrase_id) {
  uint32_t block_number = get_phrase_block_number(full_phrase_id);
  uint16_t block_size = FLASH_SECTOR_SIZE;
  uint8_t block[block_size];

  if (!loadBlockFromFlash(bank_number, block_number, block_size, main_key, main_iv_mask, block)) {
    return NULL;
  }

  // 3. deserialize phrase flatbuf block
  phraser_PhraseBlock_table_t phrase_block;
  if (!(phrase_block = phraser_PhraseBlock_as_root(block + DATA_OFFSET))) {
    return NULL;
  }

  phraser_StoreBlock_struct_t old_store_block = phraser_PhraseBlock_block(phrase_block);
  uint16_t phrase_id = phraser_StoreBlock_block_id(old_store_block);
  if (phrase_id != full_phrase_id) {
    return NULL;
  }

  FullPhrase* full_phrase = (FullPhrase*)malloc(sizeof(FullPhrase));
  full_phrase->phrase_block_id = phrase_id;
  full_phrase->phrase_template_id = phraser_PhraseBlock_phrase_template_id(phrase_block);
  full_phrase->folder_id = phraser_PhraseBlock_folder_id(phrase_block);
  full_phrase->is_tombstone = phraser_PhraseBlock_is_tombstone(phrase_block);
  flatbuffers_string_t phrase_name = phraser_PhraseBlock_phrase_name(phrase_block);
  size_t phrase_name_length = flatbuffers_string_len(phrase_name);
  full_phrase->phrase_name = copyString((char*)phrase_name, phrase_name_length);

  phraser_PhraseHistory_vec_t phrase_history_vec = phraser_PhraseBlock_history(phrase_block);
  full_phrase->history = convertPhraseHistoryArray(phrase_history_vec);

  return full_phrase;
}
