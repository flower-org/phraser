#include "Registry.h"

int blockCount = BLOCK_COUNT;
uint32_t maxVersionIndex = -1;
uint32_t maxVersion = -1;

FolderTreeNode* createFolderTreeNode(Folder* folder, hashtable_t* subfolders, hashtable_t* childPhrases) {
  FolderTreeNode* node = (FolderTreeNode*)malloc(sizeof(FolderTreeNode));

  node->folder = folder;
  
  node->subfolders = subfolders;
  node->childPhrases = childPhrases;

  return node;
}

FolderTreeNode* createFolderTreeNode(Folder* folder) {
  return createFolderTreeNode(folder, ht_create(1024), ht_create(1024));
}

void deleteFolderTreeNode(FolderTreeNode* node) {
  //Folder are in FoldersBlock structure
  //node->folder
  ht_destroy(node->subfolders);
  ht_destroy(node->childPhrases);

  free(node);
}

FolderTreeNode* rootFolder = createFolderTreeNode(NULL);

//Integer, RegStoreBlockAndIndex
hashtable_t* blocksByIdMap = ht_create(1024);
//Integer, StoreBlock
hashtable_t* activeBlocksByIndexMap = ht_create(1024);

uint32_t getMaxVersion() {
  return maxVersion;
}

void setTestOverrideBlockCount(uint32_t newBlockCount) {
  blockCount = newBlockCount;
}

//Main blocks index

//uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
//uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
//                  0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };

uint8_t* aes256_iv;
uint8_t* aes256_key;

//folderId, FolderTreeNode
hashtable_t* foldersMap = ht_create(1024);
//setId, SymbolSet
hashtable_t* symbolSetsMap = ht_create(1024);
//templateId, PhraseTemplate
hashtable_t* phraseTemplatesMap = ht_create(1024);
//blockId, PhraseBlock
hashtable_t* phrasesMap = ht_create(1024);

FolderTreeNode* getRootFolder() {
  return rootFolder;
}

FolderTreeNode* getFolderById(uint32_t folderId) {
  return (FolderTreeNode*)ht_get(foldersMap, folderId);
}

SymbolSet* getSymbolSetById(uint32_t setId) {
  return (SymbolSet*)ht_get(symbolSetsMap, setId);
}

PhraseTemplate* getPhraseTemplateById(uint32_t templateId) {
  return (PhraseTemplate*)ht_get(phraseTemplatesMap, templateId);
}

PhraseBlock* getPhraseBlockById(uint32_t blockId) {
  return (PhraseBlock*)ht_get(phrasesMap, blockId);
}

void processKeyBlockUpdate(KeyBlock* oldBlock, KeyBlock* newBlock) {
  //KEY_BLOCK_TYPE
  //KeyBlock - Set Keys, full reset; unique 

  aes256_iv = newBlock->iv;
  aes256_key = newBlock->key;

  if (oldBlock) {
    sb_deleteKeyBlock(oldBlock);
  }
}

void refreshFolderTreeChildPhrases() {
  //Clear child phrases for all folders
  {
    ht_clear(rootFolder->childPhrases);

    hash_elem_it foldersMapIterator = HT_ITERATOR(foldersMap);
    FolderTreeNode* nextFolderNode = (FolderTreeNode*)ht_iterate_values(&foldersMapIterator);
    while (nextFolderNode != 0) {
      //Serial.printf("Folder %d parent %d %s\n", nextFolderNode->folder->folderId, nextFolderNode->folder->parentFolderId, nextFolderNode->folder->folderName);

      FolderTreeNode* parentFolderNode;
      if (nextFolderNode->folder->parentFolderId) {
        parentFolderNode = (FolderTreeNode*)ht_get(foldersMap, nextFolderNode->folder->parentFolderId);
      } else {
        parentFolderNode = rootFolder;
      }
      
      ht_put(parentFolderNode->subfolders, nextFolderNode->folder->folderId, nextFolderNode->folder->folderId);

      ht_clear(nextFolderNode->childPhrases);
      nextFolderNode = (FolderTreeNode*)ht_iterate_values(&foldersMapIterator);
    }
  }

  //Restore child phrases hierarchy structure from phrases map
  {
    hash_elem_it phrasesMapIterator = HT_ITERATOR(phrasesMap);
    PhraseBlock* nextPhrase = (PhraseBlock*)ht_iterate_values(&phrasesMapIterator);
    while (nextPhrase != 0) {
      FolderTreeNode* folder;
      if (nextPhrase->folderId) {
        folder = (FolderTreeNode*)ht_get(foldersMap, nextPhrase->folderId);
      } else {
        folder = rootFolder;
      }

      ht_put(folder->childPhrases, nextPhrase->block.blockId, nextPhrase->block.blockId);
      
      nextPhrase = (PhraseBlock*)ht_iterate_values(&phrasesMapIterator);
    }
    //ht_clear(foldersMap);
  }
}

void processFoldersBlockUpdate(FoldersBlock* oldBlock, FoldersBlock* newBlock) {
  //FOLDERS_BLOCK_TYPE
  //This is a main explorer tree structure
  //FoldersBlock - Set Folders, full reset; folderId -> special structure

  //Reset rootFolder node tree
  ht_clear(rootFolder->subfolders);
  ht_clear(rootFolder->childPhrases);

  //Clear Folders map
  {
    hash_elem_it foldersMapIterator = HT_ITERATOR(foldersMap);
    FolderTreeNode* nextFolderNode = (FolderTreeNode*)ht_iterate_values(&foldersMapIterator);
    while (nextFolderNode != 0) {
      deleteFolderTreeNode(nextFolderNode);
      
      nextFolderNode = (FolderTreeNode*)ht_iterate_values(&foldersMapIterator);
    }
    ht_clear(foldersMap);
  }

  //Add new Folders to the map
  //Serial.printf("Folders:::\n");
  for (int i = 0; i < newBlock->foldersLength; i++) {
    Folder* folder = newBlock->folders + i;
    //Serial.printf("PUT Folder id %d\n", folder->folderId);
    ht_put(foldersMap, folder->folderId, (uint32_t)createFolderTreeNode(folder));
  }

  //Restore subfolder tree structure from folders map
  {
    hash_elem_it foldersMapIterator = HT_ITERATOR(foldersMap);
    FolderTreeNode* nextFolderNode = (FolderTreeNode*)ht_iterate_values(&foldersMapIterator);
    while (nextFolderNode != 0) {
      FolderTreeNode* folder;
      if (nextFolderNode->folder->parentFolderId) {
        folder = (FolderTreeNode*)ht_get(foldersMap, nextFolderNode->folder->parentFolderId);
      } else {
        folder = rootFolder;
      }

      nextFolderNode = (FolderTreeNode*)ht_iterate_values(&foldersMapIterator);
    }
//    ht_clear(foldersMap);
  }

  if (oldBlock) {
    sb_deleteFoldersBlock(oldBlock);
  }
}

void processSymbolSetsBlockUpdate(SymbolSetsBlock* oldBlock, SymbolSetsBlock* newBlock) {
  //SYMBOL_SETS_BLOCK_TYPE
  //SymbolSetsBlock - Symbol Sets, full reset; setId -> SymbolSet

  ht_clear(symbolSetsMap);

  for (int i = 0; i < newBlock->symbolSetsLength; i++) {
    SymbolSet* set = newBlock->symbolSets + i;
    ht_put(symbolSetsMap, set->setId, (uint32_t)set); 
  }

  if (oldBlock) {
    sb_deleteSymbolSetsBlock(oldBlock);
  }
}

void processPhraseTemplatesBlockUpdate(PhraseTemplatesBlock* oldBlock, PhraseTemplatesBlock* newBlock) {
  //PHRASE_TEMPLATES_BLOCK_TYPE
  //PhraseTemplatesBlock - Phrase Templates, full reset; templateId -> PhraseTemplate

  ht_clear(phraseTemplatesMap);

  for (int i = 0; i < newBlock->phraseTemplatesLength; i++) {
    PhraseTemplate* phraseTemplate = newBlock->phraseTemplates + i;
    ht_put(phraseTemplatesMap, phraseTemplate->templateId, (uint32_t)phraseTemplate); 
  }

  if (oldBlock) {
    sb_deletePhraseTemplatesBlock(oldBlock);
  }
}

void processPhraseBlockUpdate(PhraseBlock* oldBlock, PhraseBlock* newBlock) {
  //PHRASE_BLOCK_TYPE
  //PhraseBlock - Phrases, phrase per block; blockId -> PhraseBlock

  if (oldBlock) {
    ht_remove(phrasesMap, oldBlock->block.blockId);
    sb_deletePhraseBlock(oldBlock);
  }

  ht_put(phrasesMap, newBlock->block.blockId, (uint32_t)newBlock);
}

//Map<folderId, Folder>
//Map<>
//Map<>
//Map<>
void processBlockUpdate(StoreBlock* oldBlock, StoreBlock* newBlock) {
  if (!oldBlock) {
    //Serial.printf("New block: id/version/type [%d/%d/%s] \n", newBlock->blockId, newBlock->version, getBlockTypeStr(newBlock->type));
  } else {
    //Serial.printf("Replacing block: id/version/type [%d/%d/%s -> %d/%d/%s] \n", oldBlock->blockId, oldBlock->version, getBlockTypeStr(oldBlock->type), newBlock->blockId, newBlock->version, getBlockTypeStr(newBlock->type));
  }

  switch (newBlock->type) {
    case KEY_BLOCK_TYPE: 
      processKeyBlockUpdate((KeyBlock*)oldBlock, (KeyBlock*)newBlock);
      break;
    case FOLDERS_BLOCK_TYPE: 
      processFoldersBlockUpdate((FoldersBlock*)oldBlock, (FoldersBlock*)newBlock);
      break;
    case SYMBOL_SETS_BLOCK_TYPE: 
      processSymbolSetsBlockUpdate((SymbolSetsBlock*)oldBlock, (SymbolSetsBlock*)newBlock);
      break;
    case PHRASE_TEMPLATES_BLOCK_TYPE: 
      processPhraseTemplatesBlockUpdate((PhraseTemplatesBlock*)oldBlock, (PhraseTemplatesBlock*)newBlock);
      break;
    case PHRASE_BLOCK_TYPE: 
      processPhraseBlockUpdate((PhraseBlock*)oldBlock, (PhraseBlock*)newBlock);
      break;
  }
}

uint8_t addBlock(uint32_t index, StoreBlock* blockToAdd) {
  if (index >= blockCount) {
    return 0;
  }
  
  uint32_t newVersion = blockToAdd->version;
  uint32_t newBlockId = blockToAdd->blockId;

  RegStoreBlockAndIndex* oldBlockAndIndex = (RegStoreBlockAndIndex*)ht_get(blocksByIdMap, newBlockId);

  //Serial.printf("oldBlockAndIndex %d %s \n", oldBlockAndIndex, !oldBlockAndIndex ? "IS NOT" : "IS");

  //Ignore old versions
  if (!oldBlockAndIndex || oldBlockAndIndex->block.version < newVersion) {
    //Update maxVersion if needed
    if (maxVersion == -1 || maxVersion < newVersion) {
      maxVersion = newVersion;
      maxVersionIndex = index;
    }

    //Create a new block record
    RegStoreBlockAndIndex* regStoreBlockAndIndex = (RegStoreBlockAndIndex*)malloc(sizeof(RegStoreBlockAndIndex));
    *(regStoreBlockAndIndex) = sb_createRegStoreBlockAndIndex(sb_createStoreBlock(newBlockId, newVersion, blockToAdd->type), index);

    if (oldBlockAndIndex) {
      //Serial.printf("oldBlockAndIndex->index %d \n", oldBlockAndIndex->index);
      ht_remove(activeBlocksByIndexMap, oldBlockAndIndex->index);
    }

    //Add new
    ht_put(activeBlocksByIndexMap, index, (uint32_t)regStoreBlockAndIndex);
    ht_put(blocksByIdMap, newBlockId, (uint32_t)regStoreBlockAndIndex);

    //Perform block update processing
    processBlockUpdate((StoreBlock*)oldBlockAndIndex, blockToAdd);
    //Free the old registry entry
    if (oldBlockAndIndex) {
      free(oldBlockAndIndex);
    }
  }
  
  return 1;
}

RegStoreBlockAndIndex* getBlockById(uint32_t blockId) {
  return (RegStoreBlockAndIndex*)ht_get(blocksByIdMap, blockId);
}

StoreBlock* getBlockByIndex(uint32_t index) {
  return (StoreBlock*)ht_get(activeBlocksByIndexMap, index);
}

uint32_t getNextFreeIndex() {
  uint32_t cursor = maxVersionIndex;
  for (uint32_t i = 0; i < blockCount; i++) {
    uint32_t index = cursor % blockCount;
    if (!ht_get(activeBlocksByIndexMap, index)) {
      return index;
    }
    cursor++;
  }
  return -1;
}

uint32_t getFreeBlocksCount() {
  return blockCount - ht_element_count(activeBlocksByIndexMap);
}

void loadRegistryFromFlash() {
  void* checkBlock4096 = (void*)malloc(4096);
  StoreBlock storeBlock;

  //Serial.printf("blocksByIdMap.get(1) %d\n", ht_get(blocksByIdMap, 1));

  //1st pass - find unencrypted blocks
  //         - decrypt and validate the Adler checksum
  //         - initialize AES Key/IV
  //Serial.printf("loadRegistryFromFlash: block count %d\n", BLOCK_COUNT);
  for (uint16_t i = 0; i < BLOCK_COUNT; i++) {
    readBlock4096DataFromFlash(i, checkBlock4096);
    //Serial.printf("Resolving store block; index: %d\r\n", i);

    uint8_t sbInitSuccess = initStoreBlock(checkBlock4096, &storeBlock);
    if (sbInitSuccess) {
      //Serial.printf("Resolving store block success; index: %d; blockId %d; version %d; type %d;\n", i, storeBlock.blockId, storeBlock.version, storeBlock.type);
      
      if (storeBlock.type == KEY_BLOCK_TYPE) {
        KeyBlock* keyBlock = (KeyBlock*)malloc(sizeof(KeyBlock));
        uint8_t keyBlockInitSuccess = initKeyBlock(checkBlock4096, keyBlock);
        //Serial.printf("Resolving KeyBlock success %d\n", keyBlockInitSuccess);

        addBlock(i, (StoreBlock*)keyBlock);
      }
    }
  }

  //2nd pass - find blocks that can be decrypted with said key
  //         - decrypt and validate the Adler checksum
  //         - recognize block type. init a block from buffer and add to the registry.
  for (uint16_t i = 0; i < BLOCK_COUNT; i++) {
    //Serial.printf("Resolving encrypted store block; index: %d\n", i);
    readBlock4096DataFromFlash(i, checkBlock4096);
    inPlaceDecryptBlock4096(getStoreKey(), (uint8_t*)checkBlock4096);

    uint8_t sbInitSuccess = initStoreBlock(checkBlock4096, &storeBlock);
    if (sbInitSuccess) {
      //Serial.printf("Resolving encrypted store block success; index: %d; blockId %d; version %d; type %d;\n", i, storeBlock.blockId, storeBlock.version, storeBlock.type);

      if (storeBlock.type == FOLDERS_BLOCK_TYPE) {
        FoldersBlock* foldersBlock = (FoldersBlock*)malloc(sizeof(FoldersBlock));
        uint8_t foldersBlockInitSuccess = initFoldersBlock(checkBlock4096, foldersBlock);
        //Serial.printf("Resolving FoldersBlock success %d\n", foldersBlockInitSuccess);

        addBlock(i, (StoreBlock*)foldersBlock);
      } else if (storeBlock.type == SYMBOL_SETS_BLOCK_TYPE) {
        SymbolSetsBlock* symbolSetsBlock = (SymbolSetsBlock*)malloc(sizeof(SymbolSetsBlock));
        uint8_t symbolSetsBlockInitSuccess = initSymbolSetsBlock(checkBlock4096, symbolSetsBlock);
        //Serial.printf("Resolving SymbolSetsBlock success %d\n", symbolSetsBlockInitSuccess);

        addBlock(i, (StoreBlock*)symbolSetsBlock);
      } else if (storeBlock.type == PHRASE_TEMPLATES_BLOCK_TYPE) {
        PhraseTemplatesBlock* phraseTemplatesBlock = (PhraseTemplatesBlock*)malloc(sizeof(PhraseTemplatesBlock));
        uint8_t phraseTemplatesBlockInitSuccess = initPhraseTemplatesBlock(checkBlock4096, phraseTemplatesBlock);
        //Serial.printf("Resolving PhraseTemplatesBlock success %d\n", phraseTemplatesBlockInitSuccess);

        addBlock(i, (StoreBlock*)phraseTemplatesBlock);
      } else if (storeBlock.type == PHRASE_BLOCK_TYPE) {
        PhraseBlock* phraseBlock = (PhraseBlock*)malloc(sizeof(PhraseBlock));
        uint8_t phraseBlockInitSuccess = initPhraseBlock(checkBlock4096, phraseBlock);
        //Serial.printf("Resolving PhraseBlock success %d\n", phraseBlockInitSuccess);

        addBlock(i, (StoreBlock*)phraseBlock);
      }
    }
  }

  //Restore child phrases hierarchy structure from phrases map
  refreshFolderTreeChildPhrases();

  free(checkBlock4096);
}
