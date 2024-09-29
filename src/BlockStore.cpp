#include "BlockStore.h"

const char foldersBlockType[] = "FOLDERS_BLOCK_TYPE";
const char symbolSetsBlockType[] = "SYMBOL_SETS_BLOCK_TYPE";
const char phraseTemplatesBlockType[] = "PHRASE_TEMPLATES_BLOCK_TYPE";
const char phraseBlockType[] = "PHRASE_BLOCK_TYPE";
const char keyBlockType[] = "KEY_BLOCK_TYPE";
const char dogadkaBlockType[] = "DOGADKA_BLOCK_TYPE";
const char napryamkaBlockType[] = "NAPRYAMKA_BLOCK_TYPE";
const char smychkaBlockType[] = "SMYCHKA_BLOCK_TYPE";
const char unknownBlockType[] = "BlockTypeUnknown";

uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

//AES-256 key
uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                  0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };


//------------------ Block Utils ------------------

const char* getBlockTypeStr(uint32_t blockType) {
  switch (blockType) {
    case FOLDERS_BLOCK_TYPE: return foldersBlockType;
    case SYMBOL_SETS_BLOCK_TYPE: return symbolSetsBlockType;
    case PHRASE_TEMPLATES_BLOCK_TYPE: return phraseTemplatesBlockType;
    case PHRASE_BLOCK_TYPE: return phraseBlockType;
    case KEY_BLOCK_TYPE: return keyBlockType;
    case DOGADKA_BLOCK_TYPE: return dogadkaBlockType;
    case NAPRYAMKA_BLOCK_TYPE: return napryamkaBlockType;
    case SMYCHKA_BLOCK_TYPE: return smychkaBlockType;
    default: return unknownBlockType;
  }
}

//-- load from flash --

uint8_t validateBlockChecksum(void* block4096) {
  uint16_t calculatedAdlerChecksum = adler16((uint8_t*)block4096, DATA_WO_CHECKSUM_SIZE);
  void* adler16ChecksumPtr = block4096 + DATA_WO_CHECKSUM_SIZE;
  uint16_t adler16Checksum = streamLoadUint16_t(&adler16ChecksumPtr);

  return calculatedAdlerChecksum == adler16Checksum;
}

//-- save to flash --

void updateStoreBlockChecksum(void* block4096) {
  uint16_t calculatedAdlerChecksum = adler16((uint8_t*)block4096, DATA_WO_CHECKSUM_SIZE);
  void* adler16ChecksumPtr = block4096 + DATA_WO_CHECKSUM_SIZE;
  streamSaveUint16_t(&adler16ChecksumPtr, calculatedAdlerChecksum);
}

void fillBufferWithRandomBytes(void** cursor, uint32_t size) {
  for (uint32_t i = 0; i < size; i++) {
    streamSaveUint8_t(cursor, random(256));
  }
}

void fillRemainderWithRandom(void* block4096, void** cursor) {
  //fill the remainder of the Block with random bytes
  uint32_t remainder = ((uint8_t*)block4096 - (uint8_t*)*cursor) + DATA_WO_CHECKSUM_SIZE;
  fillBufferWithRandomBytes(cursor, remainder);
}

void finalizeBlock(void* toBlock4096, void** cursor) {
  //fill the remainder of the Block with random bytes
  fillRemainderWithRandom(toBlock4096, cursor);

  //update block checksum
  updateStoreBlockChecksum(toBlock4096);
}

//------ StoreBlock ------

uint8_t initStoreBlock(void* fromBlock4096, StoreBlock* toStoreBlock) {
  if (!validateBlockChecksum(fromBlock4096)) {
    return false;
  }

  return sb_initStoreBlock(&fromBlock4096, toStoreBlock);
}

//------ KeyBlock ------

uint8_t initKeyBlock(void* fromBlock4096, KeyBlock* toKeyBlock) {
  if (!validateBlockChecksum(fromBlock4096)) {
    return false;
  }

  return sb_initKeyBlock(&fromBlock4096, toKeyBlock);
}

void saveKeyBlock(KeyBlock* keyBlock, void* toBlock4096) {
  void* cursor = toBlock4096;

  sb_saveKeyBlock(&cursor, keyBlock);
  finalizeBlock(toBlock4096, &cursor);
}

void deleteKeyBlock(KeyBlock* keyBlock) {
  sb_deleteKeyBlock(keyBlock);
}

//------ FoldersBlock ------

uint8_t initFoldersBlock(void* fromBlock4096, FoldersBlock* toFoldersBlock) {
  if (!validateBlockChecksum(fromBlock4096)) {
    return false;
  }

  return sb_initFoldersBlock(&fromBlock4096, toFoldersBlock);
}

void saveFoldersBlock(FoldersBlock* foldersBlock, void* toBlock4096) {
  void* cursor = toBlock4096;

  sb_saveFoldersBlock(&cursor, foldersBlock);
  finalizeBlock(toBlock4096, &cursor);
}

void deleteFoldersBlock(FoldersBlock* foldersBlock) {
  sb_deleteFoldersBlock(foldersBlock);
}

//------ SymbolSetsBlock ------

uint8_t initSymbolSetsBlock(void* fromBlock4096, SymbolSetsBlock* toSymbolSetsBlock) {
  if (!validateBlockChecksum(fromBlock4096)) {
    return 0;
  }

  return sb_initSymbolSetsBlock(&fromBlock4096, toSymbolSetsBlock);
}

void saveSymbolSetsBlock(SymbolSetsBlock* symbolSetsBlock, void* toBlock4096) {
  void* cursor = toBlock4096;

  sb_saveSymbolSetsBlock(&cursor, symbolSetsBlock);
  finalizeBlock(toBlock4096, &cursor);
}

void deleteSymbolSetsBlock(SymbolSetsBlock* symbolSetsBlock) {
  sb_deleteSymbolSetsBlock(symbolSetsBlock);
}

//------ PhraseTemplateBlock ------

uint8_t initPhraseTemplatesBlock(void* fromBlock4096, PhraseTemplatesBlock* toPhraseTemplatesBlock) {
  if (!validateBlockChecksum(fromBlock4096)) {
    return false;
  }

  return sb_initPhraseTemplatesBlock(&fromBlock4096, toPhraseTemplatesBlock);
}

void savePhraseTemplatesBlock(PhraseTemplatesBlock* phraseTemplatesBlock, void* toBlock4096) {
  void* cursor = toBlock4096;

  sb_savePhraseTemplatesBlock(&cursor, phraseTemplatesBlock);
  finalizeBlock(toBlock4096, &cursor);
}

void deletePhraseTemplatesBlock(PhraseTemplatesBlock* phraseTemplatesBlock) {
  sb_deletePhraseTemplatesBlock(phraseTemplatesBlock);
}

//------ PhraseBlock ------

uint8_t initPhraseBlock(void* fromBlock4096, PhraseBlock* toPhraseBlock) {
  if (!validateBlockChecksum(fromBlock4096)) {
    return false;
  }

  return sb_initPhraseBlock(&fromBlock4096, toPhraseBlock);
}

void savePhraseBlock(PhraseBlock* phraseBlock, void* toBlock4096) {
  void* cursor = toBlock4096;

  sb_savePhraseBlock(&cursor, phraseBlock);
  finalizeBlock(toBlock4096, &cursor);
}

void deletePhraseBlock(PhraseBlock* phraseBlock) {
  sb_deletePhraseBlock(phraseBlock);
}

//------------------Block Encryption------------------

uint8_t* getStoreKey() {
  return key;
}

void inPlaceDecryptBlock4096(uint8_t* key, uint8_t* block4096) {
  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CBC_decrypt_buffer(&ctx, block4096, DATA_BLOCK_SIZE);
}

void inPlaceEncryptBlock4096(uint8_t* key, uint8_t* block4096) {
  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CBC_encrypt_buffer(&ctx, block4096, DATA_BLOCK_SIZE);
}

//------------------Block Load/Save from Flash------------------

//--4To6bl BepoRTHblu npoTuBHuK He goragaJlCR, HyMepyeM 6JloKu B O6paTHoM HanpaBJleHuu C KoHLLa--

uint8_t readBlock4096DataFromFlash(uint16_t blockIndex, void* toBlock4096) {
  if (blockIndex >= BLOCK_COUNT) {
    return 0;
  }

  uint32_t readAddr = READ_ADDR_OFFSET + LAST_DATA_BLOCK_OFFSET - (blockIndex * DATA_BLOCK_SIZE);
  void* readAddrPtr = (void*)readAddr;

  streamLoadData(&readAddrPtr, toBlock4096, DATA_BLOCK_SIZE);
  
  return 1;
}

uint8_t writeBlock4096DataToFlash(uint16_t blockIndex, uint8_t* block4096) {
  if (blockIndex >= BLOCK_COUNT) {
    return 0;
  }

  uint32_t dataOffset = LAST_DATA_BLOCK_OFFSET - (blockIndex * DATA_BLOCK_SIZE);

  //Disable interrupts for safe write Flash operation
  uint32_t ints = save_and_disable_interrupts();
  // Erase the sector of the flash
  flash_range_erase(dataOffset, DATA_BLOCK_SIZE);
  // Program block4096 into the sector
  flash_range_program(dataOffset, block4096, DATA_BLOCK_SIZE);
  //Restore interrupts
  restore_interrupts(ints);

  return 1;
}

//------------------Block Load/Save from Flash------------------

uint8_t* base64encode(uint32_t size, uint8_t* buffer) {
  uint32_t encodedSize = Base64.encodedLength(size);

  //zero-terminated, therefore (encodedSize + 1)
  uint8_t* encodedString = (uint8_t*)malloc((encodedSize + 1) * sizeof(uint8_t));
  Base64.encode((char*)encodedString, (char*)buffer, size);

  return encodedString;
}

uint8_t* base64decode(uint32_t size, uint8_t* buffer) {
  uint32_t decodedSize = Base64.decodedLength((char*)buffer, size);

  //zero-terminated, therefore (decodedSize + 1)
  uint8_t* decodedString = (uint8_t*)malloc((decodedSize + 1) * sizeof(uint8_t));

  Base64.decode((char*)decodedString, (char*)buffer, size);
  
  return decodedString;
}
