#include "DefaultDbInitializer.h"
#include "DbCreate.h"
#include "Schema_builder.h"
#include "PhraserUtils.h"
#include "Adler.h"
#include "SerialUtils.h"
#include "BlockDAO.h"

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

void initDefaultSymbolSetsBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask) {
  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_SymbolSetsBlock_start_as_root(&builder);

  phraser_SymbolSetsBlock_symbol_sets_start(&builder);
  symbolSetsBlock_symbolSet(&builder, 1, "Digits", "0123456789");
  symbolSetsBlock_symbolSet(&builder, 2, "Letters", "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
  symbolSetsBlock_symbolSet(&builder, 3, "Uppercase", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  symbolSetsBlock_symbolSet(&builder, 4, "Lowercase", "abcdefghijklmnopqrstuvwxyz");
  symbolSetsBlock_symbolSet(&builder, 5, "Special", "%#!*^@$&");
  symbolSetsBlock_symbolSet(&builder, 6, "Min special", "#!?");
  symbolSetsBlock_symbolSet(&builder, 7, "Ext special", "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
  symbolSetsBlock_symbolSet(&builder, 8, "Space", " ");
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

void initDefaultFoldersBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask) {
  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_FoldersBlock_start_as_root(&builder);

  phraser_FoldersBlock_folders_start(&builder);
  foldersBlock_folder(&builder, 1, 0, "Websites");
  foldersBlock_folder(&builder, 2, 0, "Computers");
  foldersBlock_folder(&builder, 3, 1, "Social");
  foldersBlock_folder(&builder, 4, 1, "Finance");
  foldersBlock_folder(&builder, 5, 2, "Laptops");
  foldersBlock_folder(&builder, 6, 2, "Servers");
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


void initDefaultPhraseTemplatesBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask) {
  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_PhraseTemplatesBlock_start_as_root(&builder);
  
  phraser_PhraseTemplatesBlock_word_templates_start(&builder);
  phraseTemplatesBlock_wordTemplate(&builder, 1, getWordPermissions(false, true, true, true), phraser_Icon_Login, 4, 256, "username", {}, 0);
  uint16_t password_symbol_sets[] = {1,2,7};
  phraseTemplatesBlock_wordTemplate(&builder, 2, getWordPermissions(true, false, true, false), phraser_Icon_Key, 24, 32, "password", password_symbol_sets, 3);
  phraseTemplatesBlock_wordTemplate(&builder, 3, getWordPermissions(false, true, false, true), phraser_Icon_Question, 0, 256, "question", {}, 0);
  uint16_t answer_symbol_sets[] = {1,2,8};
  phraseTemplatesBlock_wordTemplate(&builder, 4, getWordPermissions(true, true, true, true), phraser_Icon_Message, 24, 32, "answer", answer_symbol_sets, 3);
  uint16_t drive_password_symbol_sets[] = {1,2,5};
  phraseTemplatesBlock_wordTemplate(&builder, 5, getWordPermissions(true, true, true, true), phraser_Icon_Lock, 8, 12, "drive password", drive_password_symbol_sets, 3);
  uint16_t generated_login_symbol_sets[] = {1,2};
  phraseTemplatesBlock_wordTemplate(&builder, 6, getWordPermissions(true, true, true, true), phraser_Icon_Login, 8, 16, "generated login", generated_login_symbol_sets, 2);
  uint16_t bios_password_symbol_sets[] = {1,2,6};
  phraseTemplatesBlock_wordTemplate(&builder, 7, getWordPermissions(true, true, true, true), phraser_Icon_Settings, 6, 10, "bios password", bios_password_symbol_sets, 3);
  phraseTemplatesBlock_wordTemplate(&builder, 8, getWordPermissions(false, true, true, true), phraser_Icon_Email, 5, 255, "email", {}, 0);
  phraseTemplatesBlock_wordTemplate(&builder, 9, getWordPermissions(false, true, true, true), phraser_Icon_Key, 1, 4000, "key", {}, 0);
  phraseTemplatesBlock_wordTemplate(&builder, 10, getWordPermissions(false, true, true, true), phraser_Icon_Asterisk, 1, 4000, "private key", {}, 0);
  uint16_t generated_linux_login_symbol_sets[] = {1,4};
  phraseTemplatesBlock_wordTemplate(&builder, 11, getWordPermissions(true, true, true, true), phraser_Icon_Login, 8, 16, "login (lowercase) generated", generated_linux_login_symbol_sets, 2);
  phraser_PhraseTemplatesBlock_word_templates_end(&builder);
  
  phraser_PhraseTemplatesBlock_phrase_templates_start(&builder);

  uint16_t login_pass_word_template_ids[] {    1, 2};
  uint8_t login_pass_word_template_ordinals[] {1, 1};
  phraseTemplatesBlock_phraseTemplate(&builder, 1, "Login/Pass", login_pass_word_template_ids, login_pass_word_template_ordinals, 2);

  uint16_t computer_word_template_ids[] {    1, 2, 5, 7};
  uint8_t computer_word_template_ordinals[] {1, 1, 1, 1};
  phraseTemplatesBlock_phraseTemplate(&builder, 2, "Computer", computer_word_template_ids, computer_word_template_ordinals, 4);

  uint16_t security_questions_word_template_ids[] {    1, 2, 3, 4, 3, 4, 3, 4};
  uint8_t security_questions_word_template_ordinals[] {1, 1, 1, 1, 2, 2, 3, 3};
  phraseTemplatesBlock_phraseTemplate(&builder, 3, "3 Security questions", security_questions_word_template_ids, security_questions_word_template_ordinals, 8);

  uint16_t generated_login_pass_word_template_ids[] {    6, 2};
  uint8_t generated_login_pass_word_template_ordinals[] {1, 1};
  phraseTemplatesBlock_phraseTemplate(&builder, 4, "Generated Login/Pass", generated_login_pass_word_template_ids, generated_login_pass_word_template_ordinals, 2);

  uint16_t login_email_pass_word_template_ids[] {    1, 2, 8};
  uint8_t login_email_pass_word_template_ordinals[] {1, 1, 1};
  phraseTemplatesBlock_phraseTemplate(&builder, 5, "Login/Pass/Email", login_email_pass_word_template_ids, login_email_pass_word_template_ordinals, 3);

  uint16_t key_word_template_ids[] {    9};
  uint8_t key_word_template_ordinals[] {1};
  phraseTemplatesBlock_phraseTemplate(&builder, 6, "Key", key_word_template_ids, key_word_template_ordinals, 1);

  uint16_t key_pair_word_template_ids[] {    9, 10};
  uint8_t key_pair_word_template_ordinals[] {1, 1};
  phraseTemplatesBlock_phraseTemplate(&builder, 7, "Key Pair", key_pair_word_template_ids, key_pair_word_template_ordinals, 2);

  uint16_t computer_plus_word_template_ids[] {    11, 2, 5, 7};
  uint8_t computer_plus_word_template_ordinals[] {1, 1, 1, 1};
  phraseTemplatesBlock_phraseTemplate(&builder, 8, "Computer+", computer_plus_word_template_ids, computer_plus_word_template_ordinals, 4);

  uint16_t windows_word_template_ids[] {    1, 2, 5, 7, 3, 4, 3, 4, 3, 4};
  uint8_t windows_word_template_ordinals[] {1, 1, 1, 1, 1, 1, 2, 2, 3, 3};
  phraseTemplatesBlock_phraseTemplate(&builder, 9, "Windows", windows_word_template_ids, windows_word_template_ordinals, 4);

  uint16_t windows_plus_word_template_ids[] {    11, 2, 5, 7, 3, 4, 3, 4, 3, 4};
  uint8_t windows_plus_word_template_ordinals[] {1, 1, 1, 1, 1, 1, 2, 2, 3, 3};
  phraseTemplatesBlock_phraseTemplate(&builder, 10, "Windows+", windows_plus_word_template_ids, windows_plus_word_template_ordinals, 4);
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

void initDefaultPhraseBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask) {
  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_PhraseBlock_start_as_root(&builder);

  phraser_PhraseBlock_phrase_template_id_add(&builder, 1);
  phraser_PhraseBlock_folder_id_add(&builder, 3);
  phraser_PhraseBlock_is_tombstone_add(&builder, false);
  phraser_PhraseBlock_phrase_name_add(&builder, str(&builder, "MyAccount"));

  phraser_PhraseBlock_history_start(&builder);
  phraser_Word_vec_start(&builder);
  phraseBlock_history(&builder, 1, 1, "username", "admin", getWordPermissions(false, true, true, true), phraser_Icon_Login);
  phraseBlock_history(&builder, 2, 1, "password", "admin", getWordPermissions(true, false, true, false), phraser_Icon_Key);
  phraser_Word_vec_ref_t word_refs = phraser_Word_vec_end(&builder);
  phraser_PhraseBlock_history_push_create(&builder, 1, word_refs);

  phraser_PhraseBlock_history_end(&builder);

  uint32_t entropy = random_uint32();
  phraser_StoreBlock_t* store_block;
  store_block = phraser_PhraseBlock_block_start(&builder);
  store_block->block_id = 5;
  store_block->version = 5;
  store_block->entropy = entropy;
  phraser_PhraseBlock_block_end(&builder);

  phraser_PhraseBlock_end_as_root(&builder);

  void *block_buffer;
  size_t block_buffer_size;
  block_buffer = flatcc_builder_finalize_aligned_buffer(&builder, &block_buffer_size);

  wrapDataBufferInBlock(phraser_BlockType_PhraseBlock, buffer, aes_key, aes_iv_mask, block_buffer, block_buffer_size);

  flatcc_builder_aligned_free(block_buffer);
  flatcc_builder_clear(&builder);
}
