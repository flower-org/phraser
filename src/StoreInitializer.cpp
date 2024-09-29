#include "StoreInitializer.h"

//INFORMATIONAL:
//
//Averages:
//Fill 4096-byte RAM block with garbage ~ 7 ms
//Persist 4096-byte block (min. erase sector) on Flash (erase + program) ~ 40 ms
//Sample total save time for 384 blocks - 17'983 ms (clean, without measurements)
//384 * 47 = 18'048 ms (theoretical time)
//It takes less than 20 seconds to reset all user blocks on the device.
//
//Load 4096-byte block from Flash (copy to RAM) ~ 531 us
//Sample total load time for 384 blocks - 186'561 us (clean, without measurements)
//384 * 531 = 204 ms (theoretical time)
//Therefore, checks that involve loading all user blocks are feasible on startup.
//
//Flash endurance:
//Flash can only be erased in relatively large chunks and wears out perhaps after ~20k cycles.
//While another source gives "Min. 100K Program-Erase cycles per sector".
//In any case, this should cover many years (if not decades) with normal usage pattern (mostly reads).

uint8_t* cpy(const char* str) {
  uint8_t* cpy = (uint8_t*)malloc(strlen(str)+1);
  strcpy((char*)cpy, str);

  return cpy;
}

uint8_t* cpy_(uint32_t size, uint8_t* str) {
  uint8_t* cpy = (uint8_t*)malloc(size);

  memcpy((void*)cpy, (const void *)str, size);

  return cpy;
}

void createFoldersBlock() {
  uint8_t* block4096 = (uint8_t*)malloc(4096);

  const char* folder1Name = "Websites";
  const char* folder2Name = "Banking";
  const char* folder3Name = "Terminal";

  Folder* folders = (Folder*)malloc(sizeof(Folder)*3);
  folders[0] = sb_createFolder(1, ROOT_FOLDER_ID, strlen(folder1Name), cpy(folder1Name));
  folders[1] = sb_createFolder(2, 1, strlen(folder2Name), cpy(folder2Name));
  folders[2] = sb_createFolder(3, ROOT_FOLDER_ID, strlen(folder3Name), cpy(folder3Name));

  StoreBlock foldersStoreBlock = sb_createStoreBlock(3, 1, FOLDERS_BLOCK_TYPE);
  FoldersBlock foldersBlock = sb_createFoldersBlock(foldersStoreBlock, 3, folders);

/*
  Serial.printf("Hey hey hye blockId %d version %d type %d foldersLength %d\n", foldersBlock.block.blockId, foldersBlock.block.version, foldersBlock.block.type, foldersBlock.foldersLength);
  
  for (int i = 0; i < foldersBlock.foldersLength; i++) {
    Folder folder = foldersBlock.folders[i];
    Serial.printf("Folder folderId %d parentFolderId %d folderName %s\n", folder.folderId, folder.parentFolderId, folder.folderName);
  }
*/
  
  saveFoldersBlock(&foldersBlock, block4096);

  inPlaceEncryptBlock4096(getStoreKey(), block4096);
  writeBlock4096DataToFlash(2, block4096);
  
  sb_cleanFoldersBlock(&foldersBlock);
  free(block4096);
}

void createPhraseTemplatesBlock() {
  uint8_t* block4096 = (uint8_t*)malloc(4096);
  
  const char* simpleWordName = "Word";
  const char* passwordName = "Password";
  const char* loginName = "Login";
  const char* url = "Url";

  uint8_t* symbolSetIds = (uint8_t*)malloc(sizeof(uint8_t)*4);
  symbolSetIds[0] = 1;
  symbolSetIds[1] = 2;
  symbolSetIds[2] = 3;
  symbolSetIds[3] = 6;

  uint8_t* symbolSetIds2 = (uint8_t*)malloc(sizeof(uint8_t)*4);
  symbolSetIds2[0] = 1;
  symbolSetIds2[1] = 2;
  symbolSetIds2[2] = 3;
  symbolSetIds2[3] = 6;

  const char* simpleWordPhraseName = "Word";
  const char* loginPasswordPhraseName = "Login/Pass";
  const char* urlLoginPasswordPhraseName = "URL/Login/Pass";

  PhraseTemplate* phraseTemplates = (PhraseTemplate*)malloc(sizeof(PhraseTemplate)*3);

  Word* simpleWordPhraseWords = (Word*)malloc(sizeof(Word)*1);
  //simpleWord
  simpleWordPhraseWords[0] = sb_createWord(1, FLAGS_VIEW | FLAGS_INPUT | FLAGS_TYPE, ICON_TEXT_OUT, 0, 65535, strlen(simpleWordName), cpy(simpleWordName), 0, NULL);
  phraseTemplates[0] = sb_createPhraseTemplate(SIMPLE_WORD_TEMPLATE_ID, strlen(simpleWordPhraseName), cpy(simpleWordPhraseName), 1, simpleWordPhraseWords);

  Word* loginPasswordPhraseWords = (Word*)malloc(sizeof(Word)*2);
  //login
  loginPasswordPhraseWords[0] = sb_createWord(1, FLAGS_VIEW | FLAGS_INPUT | FLAGS_TYPE, ICON_LOGIN, 0, 65535, strlen(loginName), cpy(loginName), 0, NULL);
  //password
  loginPasswordPhraseWords[1] = sb_createWord(2, FLAGS_GENERATE | FLAGS_TYPE, ICON_KEY, 16, 128, strlen(passwordName), cpy(passwordName), 4, symbolSetIds);
  phraseTemplates[1] = sb_createPhraseTemplate(LOGIN_PASSWORD_TEMPLATE_ID, strlen(loginPasswordPhraseName), cpy(loginPasswordPhraseName), 2, loginPasswordPhraseWords);
  
  Word* urlLoginPasswordPhraseWords = (Word*)malloc(sizeof(Word)*3);
  //login
  urlLoginPasswordPhraseWords[0] = sb_createWord(1, FLAGS_VIEW | FLAGS_INPUT | FLAGS_TYPE, ICON_LOGIN, 0, 65535, strlen(loginName), cpy(loginName), 0, NULL);
  //password
  urlLoginPasswordPhraseWords[1] = sb_createWord(2, FLAGS_GENERATE | FLAGS_TYPE, ICON_KEY, 16, 128, strlen(passwordName), cpy(passwordName), 4, symbolSetIds2);
  //url
  urlLoginPasswordPhraseWords[2] = sb_createWord(3, FLAGS_VIEW | FLAGS_INPUT | FLAGS_TYPE, ICON_MESSAGE, 0, 65535, strlen(url), cpy(url), 0, NULL);
  phraseTemplates[2] = sb_createPhraseTemplate(URL_LOGIN_PASSWORD_TEMPLATE_ID, strlen(urlLoginPasswordPhraseName), cpy(urlLoginPasswordPhraseName), 3, urlLoginPasswordPhraseWords);

  StoreBlock phraseTemplatesStoreBlock = sb_createStoreBlock(2, 1, PHRASE_TEMPLATES_BLOCK_TYPE);
  
  PhraseTemplatesBlock phraseTemplatesBlock = sb_createPhraseTemplatesBlock(phraseTemplatesStoreBlock, 3, phraseTemplates);

/*
  Serial.printf("Hey hey hye blockId %d version %d type %d phraseTemplatesLength %d\n", phraseTemplatesBlock.block.blockId, phraseTemplatesBlock.block.version, phraseTemplatesBlock.block.type, phraseTemplatesBlock.phraseTemplatesLength);
  
  for (int i = 0; i < phraseTemplatesBlock.phraseTemplatesLength; i++) {
    PhraseTemplate phraseTemplate = phraseTemplatesBlock.phraseTemplates[i];
    Serial.printf("PhraseTemplate templateId %d templateNameLength %d templateName %s words count %d\n", phraseTemplate.templateId, phraseTemplate.templateNameLength, phraseTemplate.templateName, phraseTemplate.wordsLength);

    for (int j = 0; j < phraseTemplate.wordsLength; j++) {
      Word wrd = phraseTemplate.words[j];
      Serial.printf("Word wordId %d flags %d icon %d minLength %d maxLength %d wordNameLength %d wordName %s symbolSetIdsLength %d\n", wrd.wordId, wrd.flags, wrd.icon, wrd.minLength, wrd.maxLength, wrd.wordNameLength, wrd.wordName, wrd.symbolSetIdsLength);

      for (int k = 0; k < wrd.symbolSetIdsLength; k++) {
        uint8_t symbolSetId = wrd.symbolSetIds[k];
        Serial.printf("SymbolSet: symbolSetId %d\n", symbolSetId);
      }
    }
  }
*/

  savePhraseTemplatesBlock(&phraseTemplatesBlock, block4096);
  inPlaceEncryptBlock4096(getStoreKey(), block4096);
  writeBlock4096DataToFlash(1, block4096);

  sb_cleanPhraseTemplatesBlock(&phraseTemplatesBlock);
  free(block4096);
}

void createPhraseBlock(uint32_t blockId, uint16_t blockIndex, uint8_t phraseTemplateId, uint8_t folderId, const char* phraseName, StorePhraseHistory* history) {
  uint8_t* block4096 = (uint8_t*)malloc(4096);

  StoreBlock phraseStoreBlock = sb_createStoreBlock(blockId, 1, PHRASE_BLOCK_TYPE);
  PhraseBlock phraseBlock = sb_createPhraseBlock(phraseStoreBlock, phraseTemplateId, folderId, strlen(phraseName), cpy(phraseName), 1, history);

/*
  StorePhraseHistory* storePhraseHistory = phraseBlock.history;
  Serial.printf("Hey hey hye blockId %d version %d type %d historyLength %d templateId %d folderId %d phraseName %s wordValuesLength %d\n", phraseBlock.block.blockId, phraseBlock.block.version, phraseBlock.block.type, phraseBlock.historyLength, phraseBlock.phraseTemplateId, phraseBlock.folderId, phraseBlock.phraseName, storePhraseHistory->wordValuesLength);
  for (int i = 0; i < storePhraseHistory->wordValuesLength; i++) {
    StoreWordValue storeWordValue = storePhraseHistory->wordValues[i];
    Serial.printf("StoreWordValue wordId %d wordValue %s\n", storeWordValue.wordId, storeWordValue.wordValue);
  }
*/
  
  savePhraseBlock(&phraseBlock, block4096);

  inPlaceEncryptBlock4096(getStoreKey(), block4096);
  writeBlock4096DataToFlash(blockIndex, block4096);
  
  sb_cleanPhraseBlock(&phraseBlock);
  free(block4096);
}

StorePhraseHistory* createHistory(uint8_t num, ...) {
  StoreWordValue* wordValues = (StoreWordValue*)malloc(sizeof(StoreWordValue)*num);

  va_list valist;
  va_start(valist, num);

  for (uint8_t i = 0; i < num; i++) {
    const char* str = va_arg(valist, const char*);
    wordValues[i] = sb_createStoreWordValue(i, strlen(str), cpy(str));
  }
  
  va_end(valist);

  StorePhraseHistory* history = (StorePhraseHistory*)malloc(sizeof(StorePhraseHistory)*1);
  history[0] = sb_createStorePhraseHistory(num, wordValues);
  return history;
}

void createPhraseBlocks() {
  const char* testBankName = "Test Bank";
  const char* testWebsiteName = "Test Website";
  const char* passGenOpenSSLName = "PassGen OpenSSL";
  const char* passGenMD5Name = "PassGen MD5";
  const char* passGenSHA256Name = "PassGen SHA256";
  const char* testLoginPassName = "Test Login/Pass";

  createPhraseBlock(4, 3, LOGIN_PASSWORD_TEMPLATE_ID, BANKING_FOLDER_ID, testBankName, createHistory(2, "bankLogin", "bankPassword"));
  createPhraseBlock(5, 4, URL_LOGIN_PASSWORD_TEMPLATE_ID, WEBSITES_FOLDER_ID, testWebsiteName, createHistory(3, "webLogin", "webPassword", "https://my.website"));
  createPhraseBlock(6, 5, SIMPLE_WORD_TEMPLATE_ID, TERMINAL_FOLDER_ID, passGenOpenSSLName, createHistory(1, "openssl rand -base64 32"));
  createPhraseBlock(7, 6, SIMPLE_WORD_TEMPLATE_ID, TERMINAL_FOLDER_ID, passGenMD5Name, createHistory(1, "date | md5sum"));
  createPhraseBlock(8, 7, SIMPLE_WORD_TEMPLATE_ID, TERMINAL_FOLDER_ID, passGenSHA256Name, createHistory(1, "date +s | sha256sum | base64 | head -c 32 ; echo"));
  createPhraseBlock(9, 8, LOGIN_PASSWORD_TEMPLATE_ID, ROOT_FOLDER_ID, testLoginPassName, createHistory(2, "login", "password"));
}

void createSymbolSetsBlock() {
  uint8_t* block4096 = (uint8_t*)malloc(4096);

  const char* capitalLettersName = "A-Z";
  const char* capitalLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char* smallLettersName = "a-z";
  const char* smallLetters = "abcdefghijklmnopqrstuvwxyz";
  const char* digitsName = "0-9";
  const char* digits = "0123456789";
  const char* minSymbolsName = "#!?";
  const char* minSymbols = "#!?";
  const char* symbolsName = "Common symbols";
  const char* symbols = "%#!*^@$&";
  const char* allSymbolsName = "All symbols";
  const char* allSymbols = "\\,._@#$&-+()/*\"':;!?<>~`|^={}[]%";

  uint8_t symbolSetsLength = 6;
  SymbolSet* symbolSets = (SymbolSet*)malloc(symbolSetsLength*sizeof(SymbolSet));
  //We copy strings because they should be releasable (free(...)) on structure cleanup/deletion.
  symbolSets[0] = sb_createSymbolSet(1, strlen(capitalLettersName), cpy(capitalLettersName), strlen(capitalLetters), cpy(capitalLetters));
  symbolSets[1] = sb_createSymbolSet(2, strlen(smallLettersName), cpy(smallLettersName), strlen(smallLetters), cpy(smallLetters));
  symbolSets[2] = sb_createSymbolSet(3, strlen(digitsName), cpy(digitsName), strlen(digits), cpy(digits));
  symbolSets[3] = sb_createSymbolSet(4, strlen(minSymbolsName), cpy(minSymbolsName), strlen(minSymbols), cpy(minSymbols));
  symbolSets[4] = sb_createSymbolSet(5, strlen(symbolsName), cpy(symbolsName), strlen(symbols), cpy(symbols));
  symbolSets[5] = sb_createSymbolSet(6, strlen(allSymbolsName), cpy(allSymbolsName), strlen(allSymbols), cpy(allSymbols));

  StoreBlock symbolSetStoreBlock = sb_createStoreBlock(1, 1, SYMBOL_SETS_BLOCK_TYPE);
  
  SymbolSetsBlock symbolSetsBlock = sb_createSymbolSetsBlock(symbolSetStoreBlock, symbolSetsLength, symbolSets);

/*
  Serial.printf("Hey hey hye blockId %d version %d type %d symbolSetsLength %d\n", symbolSetsBlock.block.blockId, symbolSetsBlock.block.version, symbolSetsBlock.block.type, symbolSetsBlock.symbolSetsLength);
  
  for (int i = 0; i < symbolSetsBlock.symbolSetsLength; i++) {
    SymbolSet symbolSet = symbolSetsBlock.symbolSets[i];
    Serial.printf("SymbolSet setId %d symbolSetNameLength %d symbolSetName %s symbolSetLength %d symbolSetArr %s\n", symbolSet.setId, symbolSet.symbolSetNameLength, symbolSet.symbolSetName, symbolSet.symbolSetLength, symbolSet.symbolSet);
  }
*/

  saveSymbolSetsBlock(&symbolSetsBlock, block4096);
  inPlaceEncryptBlock4096(getStoreKey(), block4096);
  writeBlock4096DataToFlash(0, block4096);
  
  sb_cleanSymbolSetsBlock(&symbolSetsBlock);
  free(block4096);
}

void createKeyBlock() {
  uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

  //AES-256 key
  uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                  0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };

  uint8_t* block4096 = (uint8_t*)malloc(4096);

  StoreBlock keyStoreBlock = sb_createStoreBlock(10, 1, KEY_BLOCK_TYPE);
  KeyBlock keyBlock = sb_createKeyBlock(keyStoreBlock, 32, cpy_(32, key), 16, cpy_(16, iv));

  saveKeyBlock(&keyBlock, block4096);

//DO NOT ENCRYPT
//  inPlaceEncryptBlock4096(getStoreKey(), block4096);
  writeBlock4096DataToFlash(9, block4096);
  
  sb_cleanKeyBlock(&keyBlock);
  free(block4096);
}

//Initialize default block set
void createAndSaveStandardInitialBlocks() {
  createSymbolSetsBlock();
  createFoldersBlock();
  createPhraseTemplatesBlock();
  createPhraseBlocks();
  createKeyBlock();
}

void resetStoreAndFillBlocksWithRandomBytes() {
  uint8_t* block4096 = (uint8_t*)malloc(4096);

  for (int i = 0; i < BLOCK_COUNT; i++) {
    void* cursor = (void*)block4096;
    fillBufferWithRandomBytes(&cursor, 4096);
    writeBlock4096DataToFlash(i, block4096);
  }
  free(block4096);
}
