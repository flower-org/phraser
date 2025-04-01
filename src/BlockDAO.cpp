#include "BlockDAO.h"

#include <Thumby.h>
#include "PhraserUtils.h"
#include "BlockCache.h"
#include "Adler.h"

// -------------------- DB block load -------------------- 

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

void throw_block_back(uint16_t block_number) {
  //
}

// -------------------- FOLDERS -------------------- 

void addNewFolder(char* new_folder_name) {
  //
}

void renameFolder(uint16_t folder_id, char* new_folder_name) {
  //
}

void deleteFolder(uint16_t folder_id) {
  //
}

// -------------------- PHRASES -------------------- 

void addNewPhrase(char* new_phrase_name, uint16_t phrase_template_id, uint16_t folder_id) {
  //
}

void deletePhrase(uint16_t phrase_block_id) {
  //
}

void renamePhrase(uint16_t phrase_block_id, char* new_phrase_name) {
  //
}

void changePhraseTemplate(uint16_t phrase_block_id, uint16_t new_phrase_template_id) {
  //
}

void changePhraseFolder(uint16_t phrase_block_id, uint16_t new_folder_id) {
  //
}

void generatePhraseWord(uint16_t phrase_block_id, uint16_t word_template_id, uint8_t word_template_ordinal) {
  //
}

void userEditPhraseWord(uint16_t phrase_block_id, uint16_t word_template_id, uint8_t word_template_ordinal, char* new_word, uint16_t new_word_length) {
  //
}

void deletePhraseHistory(uint16_t phrase_block_id, uint16_t phrase_history_index) {
  //
}

void makePhraseHistoryCurrent(uint16_t phrase_block_id, uint16_t phrase_history_index) {
  //
}
