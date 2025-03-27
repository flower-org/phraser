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
  // Inputs
  //buffer[4096];
  //aes_key[AES256_KEY_LENGTH];
  //aes_iv_mask[AES256_IV_LENGTH];

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

/*  Serial.printf("AES Key\r\n");
  Serial.printf("%s\r\n", bytesToHexString((unsigned char*)out_new_aes_key, AES256_KEY_LENGTH));
  Serial.printf("AES IV\r\n");
  Serial.printf("%s\r\n", bytesToHexString((unsigned char*)out_new_aes_iv_mask, AES256_IV_LENGTH));
  Serial.printf("DB Name\r\n");
  Serial.printf("%s\r\n", bytesToHexString((unsigned char*)db_name, strlen(db_name)));
  Serial.printf("Entropy\r\n");
  Serial.printf("%u\r\n", entropy);
  Serial.printf("Buffer\r\n");
  Serial.printf("%s\r\n", bytesToHexString((unsigned char*)block_buffer, block_buffer_size));*/

  flatcc_builder_aligned_free(block_buffer);
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
