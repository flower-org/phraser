#include "DefaultDbInitializer.h"
#include "DbCreate.h"
#include "Schema_builder.h"
#include "PhraserUtils.h"
#include "Adler.h"

void wrapDataBufferInBlock(uint8_t block_type, uint8_t* main_buffer, const uint8_t* aes_key, 
  const uint8_t* aes_iv_mask, void *block_buffer, size_t block_buffer_size) {
  // Generate block IV
  uint8_t block_iv[AES256_IV_LENGTH];
  for (int i = 0; i < AES256_IV_LENGTH; i++) {
    main_buffer[FLASH_SECTOR_SIZE - AES256_IV_LENGTH + i] =
      block_iv[i] = (uint8_t)random(256); // Generate random byte
  }

  uint8_t* iv = xorByteArrays(block_iv, (uint8_t*)aes_iv_mask, AES256_IV_LENGTH);

  main_buffer[0] = block_type;
  uInt16ToBytes(block_buffer_size, main_buffer+1);

  memcpy(main_buffer + 3, block_buffer, block_buffer_size);
  
  uint32_t length_without_adler = FLASH_SECTOR_SIZE - AES256_IV_LENGTH - 4;
  for (int i = 3 + block_buffer_size; i < length_without_adler; i++) {
    main_buffer[i] = (uint8_t)random(256);
  }
  reverseInPlace(main_buffer, length_without_adler);
  
  uint32_t adler32_checksum = adler32(main_buffer, length_without_adler);
  uInt32ToBytes(adler32_checksum, main_buffer+length_without_adler);

  inPlaceEncryptBlock4096((uint8_t*)aes_key, iv, main_buffer);

  free(iv);
}

void initDefaultKeyBlock(uint8_t* out_buffer, const uint8_t* in_aes_key, const uint8_t* in_aes_iv_mask,
  uint8_t* out_new_aes_key, uint8_t* out_new_aes_iv_mask, const uint16_t in_block_count) {
  // Generate random AES key
  for (int i = 0; i < AES256_KEY_LENGTH; i++) {
    out_new_aes_key[i] = (uint8_t)random(256); // Generate random byte
  }

  // Generate random IV
  for (int i = 0; i < AES256_IV_LENGTH; i++) {
    out_new_aes_iv_mask[i] = (uint8_t)random(256); // Generate random byte
  }

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_KeyBlock_start_as_root(&builder);
  phraser_KeyBlock_block_count_add(&builder, in_block_count);
  phraser_KeyBlock_key_create(&builder, (int8_t*)out_new_aes_key, AES256_KEY_LENGTH);
  phraser_KeyBlock_iv_create(&builder, (int8_t*)out_new_aes_iv_mask, AES256_IV_LENGTH);
  const char* db_name = "TokenGenerated";
  phraser_KeyBlock_db_name_create(&builder, (int8_t*)db_name, strlen(db_name));

  uint32_t entropy = random_uint32();
  phraser_StoreBlock_t* store_block;
  store_block = phraser_KeyBlock_block_start(&builder);
  store_block->block_id = 1;
  store_block->version = 1;
  store_block->entropy = entropy;
  phraser_KeyBlock_block_end(&builder);

  phraser_KeyBlock_end_as_root(&builder);

  void *block_buffer;
  size_t block_buffer_size;
  block_buffer = flatcc_builder_finalize_aligned_buffer(&builder, &block_buffer_size);

  wrapDataBufferInBlock(phraser_BlockType_KeyBlock, out_buffer, in_aes_key, in_aes_iv_mask, block_buffer, block_buffer_size);

  flatcc_builder_aligned_free(block_buffer);
  flatcc_builder_clear(&builder);
}

flatbuffers_ref_t str(flatcc_builder_t* builder, const char* s) {
  return flatbuffers_string_create_str(builder, s);
}

void symbolSet(flatcc_builder_t* builder, uint16_t symbol_set_id, const char* name, const char* set) {
  phraser_SymbolSetsBlock_symbol_sets_push_create(builder, symbol_set_id, str(builder, name), str(builder, set));
}

void initDefaultSymbolSetsBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask) {
  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_SymbolSetsBlock_start_as_root(&builder);

  phraser_SymbolSetsBlock_symbol_sets_start(&builder);
  symbolSet(&builder, 1, "Digits", "0123456789");
  symbolSet(&builder, 2, "Letters", "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
  symbolSet(&builder, 3, "Uppercase", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  symbolSet(&builder, 4, "Lowercase", "abcdefghijklmnopqrstuvwxyz");
  symbolSet(&builder, 5, "Special", "%#!*^@$&");
  symbolSet(&builder, 6, "Min special", "#!?");
  symbolSet(&builder, 7, "Ext special", "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
  symbolSet(&builder, 8, "Space", " ");
  phraser_SymbolSetsBlock_symbol_sets_end(&builder);

  uint32_t entropy = random_uint32();
  phraser_StoreBlock_t* store_block;
  store_block = phraser_SymbolSetsBlock_block_start(&builder);
  store_block->block_id = 2;
  store_block->version = 2;
  store_block->entropy = entropy;
  phraser_SymbolSetsBlock_block_end(&builder);

  phraser_SymbolSetsBlock_end_as_root(&builder);

  void *block_buffer;
  size_t block_buffer_size;
  block_buffer = flatcc_builder_finalize_aligned_buffer(&builder, &block_buffer_size);

  wrapDataBufferInBlock(phraser_BlockType_SymbolSetsBlock, buffer, aes_key, aes_iv_mask, block_buffer, block_buffer_size);

  flatcc_builder_aligned_free(block_buffer);
  flatcc_builder_clear(&builder);
}

void folder(flatcc_builder_t* builder, uint16_t folder_id, uint16_t parent_folder_id, const char* name) {
  phraser_FoldersBlock_folders_push_create(builder, folder_id, parent_folder_id, str(builder, name));
}

void initDefaultFoldersBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask) {
  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_FoldersBlock_start_as_root(&builder);

  phraser_FoldersBlock_folders_start(&builder);
  folder(&builder, 1, 0, "Websites");
  folder(&builder, 2, 0, "Computers");
  folder(&builder, 3, 1, "Social");
  folder(&builder, 4, 1, "Finance");
  folder(&builder, 5, 2, "Laptops");
  folder(&builder, 6, 2, "Servers");
  phraser_FoldersBlock_folders_end(&builder);

  uint32_t entropy = random_uint32();
  phraser_StoreBlock_t* store_block;
  store_block = phraser_FoldersBlock_block_start(&builder);
  store_block->block_id = 3;
  store_block->version = 3;
  store_block->entropy = entropy;
  phraser_FoldersBlock_block_end(&builder);

  phraser_FoldersBlock_end_as_root(&builder);

  void *block_buffer;
  size_t block_buffer_size;
  block_buffer = flatcc_builder_finalize_aligned_buffer(&builder, &block_buffer_size);

  wrapDataBufferInBlock(phraser_BlockType_FoldersBlock, buffer, aes_key, aes_iv_mask, block_buffer, block_buffer_size);

  flatcc_builder_aligned_free(block_buffer);
  flatcc_builder_clear(&builder);
}

/*void phraseTemplate(flatcc_builder_t* builder, uint16_t phrase_template_id, const char* phrase_template_name) {
  v8_ref = flatbuffers_uint8_vec_create(B, 0, 0);
  phraser_PhraseTemplatesBlock_phrase_templates_push_create(builder, phrase_template_id, str(builder, phrase_template_name), );
}*/

flatbuffers_uint16_vec_ref_t vec_uint16(flatcc_builder_t* builder, uint16_t* arr, uint16_t arr_len) {
  return flatbuffers_uint16_vec_create(builder, arr, arr_len);
}

void wordTemplate(flatcc_builder_t* builder, uint16_t word_template_id, uint8_t permissions, phraser_Icon_enum_t icon, 
    uint16_t min_length, uint16_t max_length, const char* word_template_name, uint16_t* symbol_set_ids, uint16_t symbol_set_ids_length) {
  phraser_PhraseTemplatesBlock_word_templates_push_create(builder, word_template_id, permissions, icon, min_length, max_length, 
    str(builder, word_template_name), vec_uint16(builder, symbol_set_ids, symbol_set_ids_length));
}

const uint8_t GENERATEABLE = 1;
const uint8_t TYPEABLE = 2;
const uint8_t VIEWABLE = 4;
const uint8_t USER_EDITABLE = 8;
uint8_t getWordPermissions(bool is_generateable, bool is_user_editable, bool is_typeable, bool is_viewable) {
  uint8_t permissions = 0;
  if (is_generateable) { permissions = permissions | GENERATEABLE; }
  if (is_typeable) { permissions = permissions | TYPEABLE; }
  if (is_viewable) { permissions = permissions | VIEWABLE; }
  if (is_user_editable) { permissions = permissions | USER_EDITABLE; }
  return permissions;
}

void phraseTemplate(flatcc_builder_t* builder, uint16_t phrase_template_id, const char* phrase_template_name, 
  uint16_t* word_template_ids, uint8_t* word_template_ordinals, uint16_t word_templates_length) {
    phraser_WordTemplateRef_vec_start(builder);
    for (int i = 0; i < word_templates_length; i++) {
      phraser_WordTemplateRef_vec_push_create(builder, word_template_ids[i], word_template_ordinals[i]);
    }
    phraser_WordTemplateRef_vec_ref_t word_template_refs = phraser_WordTemplateRef_vec_end(builder);

    phraser_PhraseTemplatesBlock_phrase_templates_push_create(builder, phrase_template_id, str(builder, phrase_template_name), word_template_refs);
  }

void initDefaultPhraseTemplatesBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask) {
  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_PhraseTemplatesBlock_start_as_root(&builder);

  phraser_PhraseTemplatesBlock_word_templates_start(&builder);
  wordTemplate(&builder, 1, getWordPermissions(false, true, true, true), phraser_Icon_Login, 4, 256, "username", {}, 0);
  uint16_t password_symbol_sets[] = {1,2,7};
  wordTemplate(&builder, 2, getWordPermissions(true, false, true, false), phraser_Icon_Key, 24, 64, "password", password_symbol_sets, 3);
  wordTemplate(&builder, 3, getWordPermissions(false, true, false, true), phraser_Icon_Question, 0, 256, "question", {}, 0);
  uint16_t answer_symbol_sets[] = {1,2,8};
  wordTemplate(&builder, 4, getWordPermissions(true, true, true, true), phraser_Icon_Message, 24, 64, "answer", answer_symbol_sets, 3);
  uint16_t drive_password_symbol_sets[] = {1,2,5};
  wordTemplate(&builder, 5, getWordPermissions(true, true, true, true), phraser_Icon_Lock, 24, 64, "drive password", drive_password_symbol_sets, 3);
  uint16_t generated_login_symbol_sets[] = {1,2};
  wordTemplate(&builder, 6, getWordPermissions(true, true, true, true), phraser_Icon_Login, 8, 24, "generated login", generated_login_symbol_sets, 2);
  uint16_t bios_password_symbol_sets[] = {1,2,6};
  wordTemplate(&builder, 7, getWordPermissions(true, true, true, true), phraser_Icon_Settings, 8, 10, "bios password", bios_password_symbol_sets, 3);
  wordTemplate(&builder, 8, getWordPermissions(false, true, true, true), phraser_Icon_Email, 5, 255, "email", {}, 0);
  wordTemplate(&builder, 9, getWordPermissions(false, true, true, true), phraser_Icon_Key, 1, 4000, "key", {}, 0);
  wordTemplate(&builder, 10, getWordPermissions(false, true, true, true), phraser_Icon_Asterisk, 1, 4000, "private key", {}, 0);
  phraser_PhraseTemplatesBlock_word_templates_end(&builder);

  phraser_PhraseTemplatesBlock_phrase_templates_start(&builder);
  uint16_t login_pass_word_template_ids[] {1, 2};
  uint8_t login_pass_word_template_ordinals[] {1, 1};
  phraseTemplate(&builder, 1, "Login/Pass", login_pass_word_template_ids, login_pass_word_template_ordinals, 2);
  uint16_t computer_word_template_ids[] {1, 2, 5, 7};
  uint8_t computer_word_template_ordinals[] {1, 1, 1, 1};
  phraseTemplate(&builder, 2, "Computer", computer_word_template_ids, computer_word_template_ordinals, 4);
  uint16_t security_questions_word_template_ids[] {1, 2, 3, 4, 3, 4, 3, 4};
  uint8_t security_questions_word_template_ordinals[] {1, 1, 1, 1, 2, 2, 3, 3};
  phraseTemplate(&builder, 3, "3 Security questions", security_questions_word_template_ids, security_questions_word_template_ordinals, 8);
  uint16_t generated_login_pass_word_template_ids[] {6, 2};
  uint8_t generated_login_pass_word_template_ordinals[] {1, 1};
  phraseTemplate(&builder, 4, "Generated Login/Pass", generated_login_pass_word_template_ids, generated_login_pass_word_template_ordinals, 2);
  uint16_t login_email_pass_word_template_ids[] {1, 8, 2};
  uint8_t login_email_pass_word_template_ordinals[] {1, 1, 1};
  phraseTemplate(&builder, 5, "Login/Email/Pass", login_email_pass_word_template_ids, login_email_pass_word_template_ordinals, 3);
  uint16_t key_word_template_ids[] {9};
  uint8_t key_word_template_ordinals[] {1};
  phraseTemplate(&builder, 6, "Key", key_word_template_ids, key_word_template_ordinals, 1);
  uint16_t key_pair_word_template_ids[] {9, 10};
  uint8_t key_pair_word_template_ordinals[] {1, 1};
  phraseTemplate(&builder, 7, "Key Pair", key_pair_word_template_ids, key_pair_word_template_ordinals, 2);
  phraser_PhraseTemplatesBlock_phrase_templates_end(&builder);

  uint32_t entropy = random_uint32();
  phraser_StoreBlock_t* store_block;
  store_block = phraser_PhraseTemplatesBlock_block_start(&builder);
  store_block->block_id = 4;
  store_block->version = 4;
  store_block->entropy = entropy;
  phraser_PhraseTemplatesBlock_block_end(&builder);

  phraser_PhraseTemplatesBlock_end_as_root(&builder);

  void *block_buffer;
  size_t block_buffer_size;
  block_buffer = flatcc_builder_finalize_aligned_buffer(&builder, &block_buffer_size);

  wrapDataBufferInBlock(phraser_BlockType_PhraseTemplatesBlock, buffer, aes_key, aes_iv_mask, block_buffer, block_buffer_size);

  flatcc_builder_aligned_free(block_buffer);
  flatcc_builder_clear(&builder);
}
