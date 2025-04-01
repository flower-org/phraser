#pragma once

#include <Thumby.h>
#include "Schema_builder.h"

enum UpdateResponse {
  DB_FULL,
  BLOCK_SIZE_EXCEEDED,
  OK,
  ERROR
};

bool inPlaceDecryptAndValidateBlock(uint8_t *in_out_db_block, uint32_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask);

bool loadBlockFromFlash(uint8_t bank_number, uint16_t block_number, uint32_t block_size, 
  uint8_t* aes_key, uint8_t* aes_iv_mask, 
  uint8_t *out_db_block);

UpdateResponse updateVersionAndEntropyBlock(uint8_t* block, uint16_t block_size, uint8_t* aes_key, uint8_t* aes_iv_mask);

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