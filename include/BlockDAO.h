#pragma once

#include <Thumby.h>
#include "Schema_builder.h"
#include "rbtree.h"
#include "arraylist.h"

struct Word {
  uint16_t word_template_id;
  uint8_t word_template_ordinal;
  char* name;
  char* word;
  uint8_t permissions;
  phraser_Icon_enum_t icon;
};

struct PhraseHistory {
  uint16_t phrase_template_id;
  arraylist* phrase; //arraylist<Word>
};

struct FullPhrase {
  uint16_t phrase_block_id;
  uint16_t phrase_template_id;
  uint16_t folder_id;
  uint8_t is_tombstone;
  char* phrase_name;
  //history[0] - current value (eg. current password), 1 - previous, 2 - the one before that, etc.
  arraylist* history; //arraylist<PhraseHistory>
};

enum UpdateResponse {
  DB_FULL,
  BLOCK_SIZE_EXCEEDED,
  OK,
  ERROR
};

enum CopyType {
  REGULAR,
  DOUBLE_COPY_PREVENTION
};

bool inPlaceDecryptAndValidateBlock(uint8_t *in_out_db_block, uint32_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask);

bool loadBlockFromFlash(uint8_t bank_number, uint16_t block_number, uint32_t block_size, 
  uint8_t* aes_key, uint8_t* aes_iv_mask, 
  uint8_t *out_db_block);

UpdateResponse updateVersionAndEntropyBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask, uint32_t new_version, boolean decrypt);

flatbuffers_ref_t str(flatcc_builder_t* builder, const char* s);

void wrapDataBufferInBlock(uint8_t block_type, uint8_t* main_buffer, const uint8_t* aes_key, 
  const uint8_t* aes_iv_mask, void *block_buffer, size_t block_buffer_size);

void foldersBlock_folder(flatcc_builder_t* builder, uint16_t folder_id, uint16_t parent_folder_id, const char* name);
void symbolSetsBlock_symbolSet(flatcc_builder_t* builder, uint16_t symbol_set_id, const char* name, const char* set);
flatbuffers_uint16_vec_ref_t vec_uint16(flatcc_builder_t* builder, uint16_t* arr, uint16_t arr_len);
void phraseTemplatesBlock_wordTemplate(flatcc_builder_t* builder, uint16_t word_template_id, uint8_t permissions, phraser_Icon_enum_t icon, 
  uint16_t min_length, uint16_t max_length, const char* word_template_name, uint16_t* symbol_set_ids, uint16_t symbol_set_ids_length);
void phraseTemplatesBlock_phraseTemplate(flatcc_builder_t* builder, uint16_t phrase_template_id, const char* phrase_template_name, 
  uint16_t* word_template_ids, uint8_t* word_template_ordinals, uint16_t word_templates_length);
void phraseBlock_history(flatcc_builder_t* builder, uint16_t word_template_id, int8_t word_template_ordinal,
  char* name, char* word, int8_t permissions, phraser_Icon_enum_t icon);
  
UpdateResponse addNewFolder(char* new_folder_name, uint16_t parent_folder_id, uint16_t* out_new_folder_id);
UpdateResponse renameFolder(uint16_t folder_id, char* new_folder_name);
UpdateResponse deleteFolder(uint16_t folder_id);
UpdateResponse moveFolder(uint16_t move_folder_id, uint16_t to_folder_id);

UpdateResponse addNewPhrase(char* phrase_name, uint16_t phrase_template_id, uint16_t folder_id, uint16_t* created_phrase_id);
UpdateResponse renamePhrase(uint16_t phrase_block_id, char* update_phrase_name);
UpdateResponse deletePhrase(uint16_t phrase_block_id);
UpdateResponse movePhrase(uint16_t phrase_block_id, uint16_t new_folder_id);

UpdateResponse changePhraseTemplate(uint16_t phrase_block_id, uint16_t new_phrase_template_id);

UpdateResponse generatePhraseWord(uint16_t phrase_block_id, uint16_t phrase_template_id, uint16_t word_template_id, uint8_t word_template_ordinal, bool auto_truncate_history);
UpdateResponse userEditPhraseWord(uint16_t phrase_block_id, uint16_t phrase_template_id, uint16_t word_template_id, uint8_t word_template_ordinal, char* new_word, uint16_t new_word_length, bool auto_truncate_history);

UpdateResponse deletePhraseHistory(uint16_t phrase_block_id, uint16_t phrase_history_index);
UpdateResponse makePhraseHistoryCurrent(uint16_t phrase_block_id, uint16_t phrase_history_index);
UpdateResponse clearPhraseHistory(uint16_t phrase_block_id);

uint32_t get_valid_block_number_on_the_right_of(node_t* root, uint32_t block_number);
uint32_t get_free_block_number_on_the_left_of(node_t* root, uint32_t block_number, uint32_t db_block_count);

FullPhrase* getFullPhrase(uint16_t full_phrase_id);
void releaseFullPhrase(FullPhrase* full_phrase);
