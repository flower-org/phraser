#include "DefaultDbInitializer.h"
#include "DbCreate.h"
#include "Schema_builder.h"
#include "PhraserUtils.h"

void initDefaultKeyBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask,
                          uint8_t* new_aes_key, uint8_t* new_aes_iv_mask, uint16_t block_count) {
  // Inputs
  //buffer[4096];
  //aes_key[AES256_KEY_LENGTH];
  //aes_iv_mask[AES256_IV_LENGTH];

  // Generate random AES key
  for (int i = 0; i < AES256_KEY_LENGTH; i++) {
    new_aes_key[i] = (uint8_t)random(256); // Generate random byte
  }

  // Generate random IV
  for (int i = 0; i < AES256_IV_LENGTH; i++) {
    new_aes_iv_mask[i] = (uint8_t)random(256); // Generate random byte
  }

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  phraser_KeyBlock_start_as_root(&builder);
  phraser_KeyBlock_block_count_add(&builder, block_count);
  phraser_KeyBlock_key_create(&builder, (int8_t*)new_aes_key, AES256_KEY_LENGTH);
  phraser_KeyBlock_iv_create(&builder, (int8_t*)new_aes_iv_mask, AES256_IV_LENGTH);
  const char* db_name = "TokenGenerated";
  phraser_KeyBlock_db_name_create(&builder, (int8_t*)db_name, strlen(db_name));

  phraser_StoreBlock_t* store_block;
  store_block = phraser_KeyBlock_block_start(&builder);
  store_block->block_id = 1;
  store_block->version = 1;
  store_block->entropy = random_uint32();
  phraser_KeyBlock_block_end(&builder);

  phraser_KeyBlock_end_as_root(&builder);

  void *block_buffer;
  size_t size;
  block_buffer = flatcc_builder_get_direct_buffer(&builder, &size);

  Serial.printf("%s\r\n", bytesToHexString((unsigned char*)block_buffer, size));

  flatcc_builder_clear(&builder);
}

void initDefaultSymbolSetsBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask) {
  //
}

void initDefaultFoldersBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask) {
  //
}

void initDefaultPhraseTemplatesBlock(uint8_t* buffer, const uint8_t* aes_key, const uint8_t* aes_iv_mask) {
  //
}
