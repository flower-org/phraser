#include "BlockCache.h"

#include "rbtree.h"
#include "hashtable.h"
#include "arraylist.h"
#include "SerialUtils.h"

//TODO: comment out debug Serial output in this file

//SHA256 of string "PhraserPasswordManager"
uint8_t HARDCODED_SALT[] = {
  0xE9, 0x8A, 0xD5, 0x84, 0x33, 0xB6, 0xE9, 0xE3,
  0x03, 0x30, 0x6F, 0x29, 0xE0, 0x94, 0x43, 0x8B,
  0x13, 0xA5, 0x52, 0x22, 0xD2, 0x89, 0x0E, 0x5F,
  0x6E, 0x0E, 0xC4, 0x29, 0xFB, 0x40, 0xE2, 0x6D
};
const int HARDCODED_SALT_LEN = 32;

//MD5 of string "PhraserPasswordManager"
uint8_t HARDCODED_IV_MASK[] = {
  0x44, 0x75, 0xBB, 0x91, 0x5E, 0xA8, 0x40, 0xDB,
  0xCE, 0x22, 0xDA, 0x4E, 0x22, 0x4B, 0x8A, 0x3C
};
const int IV_MASK_LEN = 16;

const int PBKDF_INTERATIONS_COUNT = 10239;

const int DATA_OFFSET = 3;

struct BlockIdAndVersion {
  uint16_t blockId;
  uint32_t blockVersion;
  uint32_t entropy;
  bool isTombstoned;
};

BlockIdAndVersion BLOCK_NOT_UPDATED = { 0, 0, 0, false };

struct SymbolSet {
  uint32_t symbolSetId;
  char* symbolSetName;
  char* symbolSet;
};

struct PhraseTemplate {
  uint32_t phraseTemplateId;
  char* phraseTemplateName;

  arraylist* wordTemplateIds;
  arraylist* wordTemplateOrdinals;
};

struct WordTemplate {
  uint32_t wordTemplateId;
  uint8_t permissions;
  phraser_Icon_enum_t icon;
  uint32_t minLength;
  uint32_t maxLength;
  char* wordTemplateName;
  arraylist* symbolSetIds;
};

struct Word {
  uint16_t word_template_id;
  uint8_t word_template_ordinal;
  char* name;
  char* word;
  uint8_t permissions;
  phraser_Icon_enum_t icon;
};

struct Phrase {
  uint32_t phraseBlockId;
  uint16_t phrase_template_id;
  uint16_t folder_id;
  bool is_tombstone;
  char* phrase_name;

  arraylist* history_phrase_template_ids;
  arraylist* history;//Word
};

// DB data structures
uint16_t lastBlockId;
uint32_t lastBlockVersion;
uint32_t lastBlockNumber;
uint32_t lastEntropy;

node_t* occupiedBlockNumbers;
hashtable* blockIdByBlockNumber; //key BlockNumber, value BlockId
hashtable* blockInfos; //key BlockId, value BlockNumberAndVersionAndCount
hashtable* tombstonedBlockIds; //key BlockId, value BlockId 

// - Login data cache
uint8_t* key_block_key = NULL;
uint32_t key_block_key_length = 0;
uint8_t bank_number;

// - KeyBlock cache
uint16_t key_block_id = 0;

char* db_name = NULL;
uint32_t db_name_length = 0;

uint16_t max_block_count = 0;
uint8_t* main_key = NULL;
uint32_t main_key_length = 0;
uint8_t* main_iv_mask = NULL;
uint32_t main_iv_mask_length = 0;

// - SymbolSetsBlock cache
uint16_t symbol_sets_block_id = 0;
hashtable* symbol_sets = NULL;//uint32_t, SymbolSet

// - FoldersBlock cache
uint16_t folders_block_id = 0;
hashtable* folders;//uint32_t, Folder
hashtable* sub_folders_by_folder;//uint32_t, arraylist<uint32_t>

// - PhraseTemplatesBlock cache
uint16_t phrase_templates_block_id = 0;
hashtable* phrase_templates;//uint32_t, PhraseTemplate
hashtable* word_templates;//uint32_t, WordTemplate

// - Phrase Blocks (minimal info) cache
hashtable* phrases;//uint32_t, PhraseFolderAndName
hashtable* phrases_by_folder;//uint32_t, arraylist<uint32_t>

void setLoginData(uint8_t* key, uint32_t key_length) {
  if (key_block_key != NULL) {
    free(key_block_key);
  }

  if (key != NULL) {
    key_block_key_length = key_length;
    key_block_key = copyBuffer(key, key_length);
  } else {
    key_block_key_length = 0;
    key_block_key = NULL;
  }
}

uint32_t getBlockVersion(uint16_t block_id) {
  uint32_t block_version = 0;
  BlockNumberAndVersionAndCount* blockInfo = (BlockNumberAndVersionAndCount*)hashtable_get(blockInfos, block_id);
  if (blockInfo != NULL) {
    block_version = blockInfo->blockVersion;
  }
  return block_version;
}

BlockIdAndVersion setKeyBlock(uint8_t* block) {
  if (block != NULL) {
    phraser_KeyBlock_table_t key_block;
    if (!(key_block = phraser_KeyBlock_as_root(block + DATA_OFFSET))) {
      return BLOCK_NOT_UPDATED;
    }

    phraser_StoreBlock_struct_t storeblock = phraser_KeyBlock_block(key_block);
    key_block_id = phraser_StoreBlock_block_id(storeblock);
    serialDebugPrintf("key_block_id %d\r\n", key_block_id);

    uint32_t old_key_block_version = getBlockVersion(key_block_id);
    uint32_t new_key_block_version = phraser_StoreBlock_version(storeblock);
    serialDebugPrintf("new_key_block_version %d; old_key_block_version %d \r\n", new_key_block_version, old_key_block_version);
    if (new_key_block_version >= old_key_block_version) {
      if (main_key != NULL) { free(main_key); }
      if (main_iv_mask != NULL) { free(main_iv_mask); }
      if (db_name != NULL) { free(db_name); }
    
      uint32_t entropy = phraser_StoreBlock_entropy(storeblock);
      serialDebugPrintf("entropy %d\r\n", entropy);

      max_block_count = phraser_KeyBlock_block_count(key_block);
      serialDebugPrintf("max_block_count %d\r\n", max_block_count);

      flatbuffers_int8_vec_t db_name_str = phraser_KeyBlock_db_name(key_block);
      db_name_length = flatbuffers_vec_len(db_name_str);
      db_name = copyString((char*)db_name_str, db_name_length);
      serialDebugPrintf("db_name %s\r\n", db_name);

      flatbuffers_int8_vec_t main_key_vec = phraser_KeyBlock_key(key_block);
      main_key_length = flatbuffers_vec_len(main_key_vec);
      main_key = copyBuffer((uint8_t*)main_key_vec, main_key_length);
      serialDebugPrintf("main_key_length %d\r\n", main_key_length);

      flatbuffers_int8_vec_t aes256_iv_mask_vec = phraser_KeyBlock_iv(key_block);
      main_iv_mask_length = flatbuffers_vec_len(aes256_iv_mask_vec);
      main_iv_mask = copyBuffer((uint8_t*)aes256_iv_mask_vec, main_iv_mask_length);
      serialDebugPrintf("aes256_iv_mask_length %d\r\n", main_iv_mask_length);

      serialDebugPrintf("ZXC. Before RETURN\r\n");
      return { key_block_id, new_key_block_version, entropy, false };
    } else {
      return BLOCK_NOT_UPDATED;
    }
  } else {
    return BLOCK_NOT_UPDATED;
  }
}

void removeAllSymbolSets(hashtable *t, uint32_t key, void* value) {
  SymbolSet* removed_symbol_set = (SymbolSet*)hashtable_remove(t, key);
  if (removed_symbol_set != NULL) {
    free(removed_symbol_set->symbolSet);
    free(removed_symbol_set->symbolSetName);
    free(removed_symbol_set);
  }
}

BlockIdAndVersion setSymbolSetsBlock(uint8_t* block) {
  if (block != NULL) {
    phraser_SymbolSetsBlock_table_t symbol_sets_block;
    if (!(symbol_sets_block = phraser_SymbolSetsBlock_as_root(block + DATA_OFFSET))) {
      return BLOCK_NOT_UPDATED;
    }

    phraser_StoreBlock_struct_t storeblock = phraser_SymbolSetsBlock_block(symbol_sets_block);
    symbol_sets_block_id = phraser_StoreBlock_block_id(storeblock);
    serialDebugPrintf("symbol_sets_block_id %d\r\n", symbol_sets_block_id);

    uint32_t old_symbol_sets_block_version = getBlockVersion(symbol_sets_block_id);
    uint32_t new_symbol_sets_block_version = phraser_StoreBlock_version(storeblock);
    serialDebugPrintf("new_symbol_sets_block_version %d; old_symbol_sets_block_version %d\r\n", new_symbol_sets_block_version, old_symbol_sets_block_version);
    if (new_symbol_sets_block_version > old_symbol_sets_block_version) {
      if (symbol_sets != NULL) { hashtable_iterate_entries(symbol_sets, removeAllSymbolSets); }

      uint32_t entropy = phraser_StoreBlock_entropy(storeblock);
      serialDebugPrintf("entropy %d\r\n", entropy);

      phraser_SymbolSet_vec_t symbol_sets_vec = phraser_SymbolSetsBlock_symbol_sets(symbol_sets_block);
      size_t symbol_sets_vec_length = flatbuffers_vec_len(symbol_sets_vec);
      serialDebugPrintf("symbol_sets_vec_length %d\r\n", symbol_sets_vec_length);

      if (symbol_sets == NULL) {
        symbol_sets = hashtable_create();
      }

      for (size_t i = 0; i < symbol_sets_vec_length; i++) {
        phraser_SymbolSet_table_t symbol_set_fb = phraser_SymbolSet_vec_at(symbol_sets_vec, i);

        uint16_t symbol_set_id = phraser_SymbolSet_set_id(symbol_set_fb);

        flatbuffers_string_t symbol_set_name_str = phraser_SymbolSet_symbol_set_name(symbol_set_fb);
        size_t symbol_set_name_str_length = flatbuffers_string_len(symbol_set_name_str);

        flatbuffers_string_t symbol_set_str = phraser_SymbolSet_symbol_set(symbol_set_fb);
        size_t symbol_set_str_length = flatbuffers_string_len(symbol_set_str);

        SymbolSet* symbol_set = (SymbolSet*)malloc(sizeof(SymbolSet));
        symbol_set->symbolSetId = symbol_set_id;
        symbol_set->symbolSetName = copyString((char*)symbol_set_name_str, symbol_set_name_str_length);
        symbol_set->symbolSet = copyString((char*)symbol_set_str, symbol_set_str_length);

        hashtable_set(symbol_sets, symbol_set_id, symbol_set);

        serialDebugPrintf("symbol_set_id %d\r\n", symbol_set_id);
        serialDebugPrintf("symbolSetName %s\r\n", symbol_set->symbolSetName);
        serialDebugPrintf("symbolSet %s\r\n", symbol_set->symbolSet);
      }

      return { symbol_sets_block_id, new_symbol_sets_block_version, entropy, false };
    } else {
      return BLOCK_NOT_UPDATED;
    }
  } else {
    return BLOCK_NOT_UPDATED;
  }
}

void removeAllFolders(hashtable *t, uint32_t key, void* value) {
  Folder* removed_folder = (Folder*)hashtable_remove(t, key);
  if (removed_folder != NULL) {
    free(removed_folder->folderName);
    free(removed_folder);
  }
}

void removeAllSubFoldersByfolder(hashtable *t, uint32_t key, void* value) {
  arraylist* removed_sub_folder_list = (arraylist*)hashtable_remove(t, key);
  if (removed_sub_folder_list != NULL) {
    arraylist_destroy(removed_sub_folder_list);
  }
}

BlockIdAndVersion setFoldersBlock(uint8_t* block) {
  if (block != NULL) {
    phraser_FoldersBlock_table_t folders_block;
    if (!(folders_block = phraser_FoldersBlock_as_root(block + DATA_OFFSET))) {
      return BLOCK_NOT_UPDATED;
    }
  
    phraser_StoreBlock_struct_t storeblock = phraser_FoldersBlock_block(folders_block);
    folders_block_id = phraser_StoreBlock_block_id(storeblock);
    serialDebugPrintf("folders_block_id %d\r\n", folders_block_id);

    uint32_t old_folders_block_version = getBlockVersion(folders_block_id);
    uint32_t new_folders_block_version = phraser_StoreBlock_version(storeblock);
    serialDebugPrintf("new_folders_block_version %d; old_folders_block_version %d\r\n", new_folders_block_version, old_folders_block_version);
    if (new_folders_block_version > old_folders_block_version) {
      if (folders != NULL) { hashtable_iterate_entries(folders, removeAllFolders); }
      if (sub_folders_by_folder != NULL) { hashtable_iterate_entries(sub_folders_by_folder, removeAllSubFoldersByfolder); }
      
      uint32_t entropy = phraser_StoreBlock_entropy(storeblock);
      serialDebugPrintf("entropy %d\r\n", entropy);

      phraser_Folder_vec_t folders_vec = phraser_FoldersBlock_folders(folders_block);
      size_t folders_vec_length = flatbuffers_vec_len(folders_vec);
      serialDebugPrintf("folders_vec_length %d\r\n", folders_vec_length);

      if (folders == NULL) {
        folders = hashtable_create();
      }
      if (sub_folders_by_folder == NULL) {
        sub_folders_by_folder = hashtable_create();
      }

      for (size_t i = 0; i < folders_vec_length; i++) {
        phraser_Folder_table_t folder_fb = phraser_Folder_vec_at(folders_vec, i);

        uint16_t folder_id = phraser_Folder_folder_id(folder_fb);
        uint16_t parent_folder_id = phraser_Folder_parent_folder_id(folder_fb);

        flatbuffers_string_t folder_name_str = phraser_Folder_folder_name(folder_fb);
        size_t folder_name_length = flatbuffers_string_len(folder_name_str);

        Folder* folder = (Folder*)malloc(sizeof(Folder));
        folder->folderId = folder_id;
        folder->parentFolderId = parent_folder_id;
        folder->folderName = copyString((char*)folder_name_str, folder_name_length);

        hashtable_set(folders, folder_id, folder);

        serialDebugPrintf("folder_id %d\r\n", folder_id);
        serialDebugPrintf("parent_id %d\r\n", parent_folder_id);
        serialDebugPrintf("folderName %s\r\n", folder->folderName);

        arraylist* subfolder_list_of_parent_folder = (arraylist*)hashtable_get(sub_folders_by_folder, parent_folder_id);
        if (subfolder_list_of_parent_folder == NULL) {
          subfolder_list_of_parent_folder = arraylist_create();
          hashtable_set(sub_folders_by_folder, parent_folder_id, subfolder_list_of_parent_folder);
        }
        arraylist_add(subfolder_list_of_parent_folder, (void*)folder_id);
        
        serialDebugPrintf("subfolders of parent_id %d: ", parent_folder_id);
        for (int j = 0; j < arraylist_size(subfolder_list_of_parent_folder); j++) {
          uint32_t child_folder_id = (uint32_t)arraylist_get(subfolder_list_of_parent_folder, j);
          serialDebugPrintf("%d, ", child_folder_id);
        }
        serialDebugPrintf("\r\n");
      }

      return { folders_block_id, new_folders_block_version, entropy, false };
    } else {
      return BLOCK_NOT_UPDATED;
    }
  } else {
    return BLOCK_NOT_UPDATED;
  }
}

void removeWordTemplates(hashtable *t, uint32_t key, void* value) {
  WordTemplate* removed_word_template = (WordTemplate*)hashtable_remove(t, key);
  if (removed_word_template != NULL) {
    arraylist_destroy(removed_word_template->symbolSetIds);
    free(removed_word_template->wordTemplateName);
    free(removed_word_template);
  }
}

void removePhraseTemplates(hashtable *t, uint32_t key, void* value) {
  PhraseTemplate* removed_phrase_template = (PhraseTemplate*)hashtable_remove(t, key);
  if (removed_phrase_template != NULL) {
    free(removed_phrase_template->phraseTemplateName);
    arraylist_destroy(removed_phrase_template->wordTemplateIds);
    arraylist_destroy(removed_phrase_template->wordTemplateOrdinals);
    free(removed_phrase_template);
  }
}

BlockIdAndVersion setPhraseTemplatesBlock(uint8_t* block) {
  if (block != NULL) {
    phraser_PhraseTemplatesBlock_table_t phrase_templates_block;
    if (!(phrase_templates_block = phraser_PhraseTemplatesBlock_as_root(block + DATA_OFFSET))) {
      return BLOCK_NOT_UPDATED;
    }

    phraser_StoreBlock_struct_t storeblock = phraser_PhraseTemplatesBlock_block(phrase_templates_block);
    phrase_templates_block_id = phraser_StoreBlock_block_id(storeblock);
    serialDebugPrintf("phrase_templates_block_id %d\r\n", phrase_templates_block_id);
  
    uint32_t old_phrase_templates_block_version = getBlockVersion(phrase_templates_block_id);
    uint32_t new_phrase_templates_block_version = phraser_StoreBlock_version(storeblock);
    serialDebugPrintf("new_phrase_templates_block_version %d; old_phrase_templates_block_version %d\r\n", new_phrase_templates_block_version, old_phrase_templates_block_version);
    if (new_phrase_templates_block_version > old_phrase_templates_block_version) {
      if (word_templates != NULL) { hashtable_iterate_entries(word_templates, removeWordTemplates); }
      if (phrase_templates != NULL) { hashtable_iterate_entries(phrase_templates, removePhraseTemplates); }
    
      uint32_t entropy = phraser_StoreBlock_entropy(storeblock);
      serialDebugPrintf("entropy %d\r\n", entropy);

      phraser_PhraseTemplate_vec_t phrase_templates_vec = phraser_PhraseTemplatesBlock_phrase_templates(phrase_templates_block);
      size_t phrase_templates_vec_length = flatbuffers_vec_len(phrase_templates_vec);
      serialDebugPrintf("phrase_templates_vec_length %d\r\n", phrase_templates_vec_length);

      phraser_WordTemplate_vec_t word_templates_vec = phraser_PhraseTemplatesBlock_word_templates(phrase_templates_block);
      size_t word_templates_vec_length = flatbuffers_vec_len(word_templates_vec);
      serialDebugPrintf("word_templates_vec_length %d\r\n", word_templates_vec_length);

      if (word_templates == NULL) {
        word_templates = hashtable_create();
      }
      if (phrase_templates == NULL) {
        phrase_templates = hashtable_create();
      }

      serialDebugPrintf("phrase_templates of phrase_templates_block_id %d\r\n", phrase_templates_block_id);
      for (int i = 0; i < phrase_templates_vec_length; i++) {
        phraser_PhraseTemplate_table_t phrase_template_fb = phraser_PhraseTemplate_vec_at(phrase_templates_vec, i);

        uint16_t phrase_template_id = phraser_PhraseTemplate_phrase_template_id(phrase_template_fb);

        flatbuffers_string_t phrase_template_name_str = phraser_PhraseTemplate_phrase_template_name(phrase_template_fb);
        size_t phrase_template_name_length = flatbuffers_string_len(phrase_template_name_str);

        phraser_WordTemplateRef_vec_t word_template_refs_vec = phraser_PhraseTemplate_word_template_refs(phrase_template_fb);
        size_t word_template_refs_length = flatbuffers_vec_len(word_template_refs_vec);

        PhraseTemplate* phrase_template = (PhraseTemplate*)malloc(sizeof(PhraseTemplate));
        phrase_template->phraseTemplateId = phrase_template_id;
        phrase_template->phraseTemplateName = copyString((char*)phrase_template_name_str, phrase_template_name_length);

        hashtable_set(phrase_templates, phrase_template_id, phrase_template);
        phrase_template->wordTemplateIds = arraylist_create();
        phrase_template->wordTemplateOrdinals = arraylist_create();

        serialDebugPrintf("=============================================%d\r\n");
        serialDebugPrintf("phrase_template_id %d\r\n", phrase_template_id);
        serialDebugPrintf("phraseTemplateName %s\r\n", phrase_template->phraseTemplateName);

        serialDebugPrintf("word template refs of phrase_template_id %d: \r\n", phrase_template_id);
        for (int j = 0; j < word_template_refs_length; j++) {
          phraser_WordTemplateRef_table_t word_template_ref_fb = phraser_WordTemplateRef_vec_at(word_template_refs_vec, j);

          uint16_t word_ref_template_id = phraser_WordTemplateRef_word_template_id(word_template_ref_fb);
          uint16_t word_ref_template_ordinal = phraser_WordTemplateRef_word_template_ordinal(word_template_ref_fb);

          serialDebugPrintf("%d/%d, ", word_ref_template_id, word_ref_template_ordinal);

          arraylist_add(phrase_template->wordTemplateIds, (void*)word_ref_template_id);
          arraylist_add(phrase_template->wordTemplateOrdinals, (void*)word_ref_template_ordinal);
        }
        serialDebugPrintf("\r\n");
      }

      serialDebugPrintf("word_templates of phrase_templates_block_id %d\r\n", phrase_templates_block_id);
      for (int i = 0; i < word_templates_vec_length; i++) {
        phraser_WordTemplate_table_t word_template_fb = phraser_WordTemplate_vec_at(word_templates_vec, i);

        uint16_t word_template_id = phraser_WordTemplate_word_template_id(word_template_fb);
        int8_t permissions = phraser_WordTemplate_permissions(word_template_fb);
        phraser_Icon_enum_t icon = phraser_WordTemplate_icon(word_template_fb);
        uint16_t min_length = phraser_WordTemplate_min_length(word_template_fb);
        uint16_t max_length = phraser_WordTemplate_max_length(word_template_fb);

        flatbuffers_string_t word_template_name_str = phraser_WordTemplate_word_template_name(word_template_fb);
        size_t word_template_name_length = flatbuffers_string_len(word_template_name_str);

        flatbuffers_uint16_vec_t symbol_set_ids_vec = phraser_WordTemplate_symbol_set_ids(word_template_fb);
        size_t symbol_set_ids_length = flatbuffers_vec_len(symbol_set_ids_vec);

        serialDebugPrintf("=============================================%d\r\n");
        serialDebugPrintf("word_template_id %d\r\n", word_template_id);
        serialDebugPrintf("permissions %d\r\n", permissions);
        serialDebugPrintf("icon %d\r\n", icon);
        serialDebugPrintf("min_length %d\r\n", min_length);
        serialDebugPrintf("max_length %d\r\n", max_length);

        WordTemplate* word_template = (WordTemplate*)malloc(sizeof(WordTemplate));
        word_template->wordTemplateId = word_template_id;
        word_template->permissions = permissions;
        word_template->icon = icon;
        word_template->minLength = min_length;
        word_template->maxLength = max_length;
        word_template->wordTemplateName = copyString((char*)word_template_name_str, word_template_name_length);
        serialDebugPrintf("wordTemplateName %s\r\n", word_template->wordTemplateName);

        serialDebugPrintf("symbolset ids of word template %d: \r\n", word_template_id);
        word_template->symbolSetIds = arraylist_create();
        for (int j = 0; j < symbol_set_ids_length; j++) {
          uint16_t symbol_set_id = flatbuffers_uint16_vec_at(symbol_set_ids_vec, j);
          arraylist_add(word_template->symbolSetIds, (void*)symbol_set_id);
          serialDebugPrintf("symbol_set_id %d\r\n", symbol_set_id);
        }
      }

      return { phrase_templates_block_id, new_phrase_templates_block_version, entropy, false };
    } else {
      return BLOCK_NOT_UPDATED;
    }
  } else {
    return BLOCK_NOT_UPDATED;
  }
}

void deleteFromPhraseByFolderList(uint16_t folder_id, uint16_t phrase_block_id) {
  arraylist* folderPhrases = (arraylist*)hashtable_get(phrases_by_folder, folder_id);
  if (folderPhrases != NULL) {
    int i = 0;
    while (true) {
      if (i >= arraylist_size(folderPhrases)) { break; }
      if ((uint32_t)arraylist_get(folderPhrases, i) == phrase_block_id) {
        arraylist_remove(folderPhrases, i);
      } else {
        i++;
      }
    }
  }
}

void addToPhraseByFolderList(uint16_t folder_id, uint16_t phrase_block_id) {
  arraylist* folderPhrases = (arraylist*)hashtable_get(phrases_by_folder, folder_id);
  if (folderPhrases == NULL) {
    folderPhrases = arraylist_create();
    hashtable_set(phrases_by_folder, folder_id, folderPhrases);
  } else {
    for (int i = 0; i < arraylist_size(folderPhrases); i++) {
      if ((uint32_t)arraylist_get(folderPhrases, i) == phrase_block_id) {
        return;
      }
    }
  }

  arraylist_add(folderPhrases, (void*)phrase_block_id);
}

BlockIdAndVersion registerPhraseBlock(uint8_t* block) {
  if (block != NULL) {
    phraser_PhraseBlock_table_t phrase_block;
    if (!(phrase_block = phraser_PhraseBlock_as_root(block + DATA_OFFSET))) {
      return BLOCK_NOT_UPDATED;
    }
    
    phraser_StoreBlock_struct_t storeblock = phraser_PhraseBlock_block(phrase_block);
    uint16_t phrase_block_id = phraser_StoreBlock_block_id(storeblock);
    serialDebugPrintf("phrase_block_id %d\r\n", phrase_block_id);

    uint32_t old_phrase_block_version = 0;
    BlockNumberAndVersionAndCount* blockInfo = (BlockNumberAndVersionAndCount*)hashtable_get(blockInfos, phrase_block_id);
    if (blockInfo != NULL) {
      old_phrase_block_version = blockInfo->blockVersion;
    }

    uint32_t new_phrase_block_version = phraser_StoreBlock_version(storeblock);
    serialDebugPrintf("new_phrase_block_version %d, old_phrase_block_version %d \r\n", new_phrase_block_version, old_phrase_block_version);
    if (new_phrase_block_version > old_phrase_block_version) {
      uint32_t entropy = phraser_StoreBlock_entropy(storeblock);
      serialDebugPrintf("entropy %d\r\n", entropy);

      uint16_t folder_id = phraser_PhraseBlock_folder_id(phrase_block);
      serialDebugPrintf("folder_id %d\r\n", folder_id);
      flatbuffers_string_t phrase_name_str = phraser_PhraseBlock_phrase_name(phrase_block);
      flatbuffers_bool_t is_tombstone = phraser_PhraseBlock_is_tombstone(phrase_block);
      size_t phrase_name_length = flatbuffers_string_len(phrase_name_str);

      if (is_tombstone) {
        // Remove from `phrases` and `phrases_by_folder` is exists
        PhraseFolderAndName* removed_phrase_folder_and_name = (PhraseFolderAndName*)hashtable_remove(phrases, phrase_block_id);
        if (removed_phrase_folder_and_name != NULL) {
          serialDebugPrintf("Tombstoning phrase %s\r\n", removed_phrase_folder_and_name->name);

          free(removed_phrase_folder_and_name->name);
          free(removed_phrase_folder_and_name);
        }

        deleteFromPhraseByFolderList(folder_id, phrase_block_id);
      } else {
        PhraseFolderAndName* phrase_folder_and_name = (PhraseFolderAndName*)hashtable_get(phrases, phrase_block_id);
        if (phrase_folder_and_name == NULL) {
          // New block or blockInfo->isTombstoned
          // Add to `phrases` and `phrases_by_folder`
          phrase_folder_and_name = (PhraseFolderAndName*)malloc(sizeof(PhraseFolderAndName));
          phrase_folder_and_name->phraseBlockId = phrase_block_id;
          phrase_folder_and_name->folderId = folder_id;
          phrase_folder_and_name->name = copyString((char*)phrase_name_str, phrase_name_length);
          hashtable_set(phrases, phrase_block_id, phrase_folder_and_name);
          serialDebugPrintf("New phrase %s\r\n", phrase_folder_and_name->name);

          addToPhraseByFolderList(folder_id, phrase_block_id);
        } else if (!(blockInfo->isTombstoned)) {
          // Update `phrases` and `phrases_by_folder`
          if (phrase_folder_and_name->folderId == folder_id) {
            deleteFromPhraseByFolderList(phrase_folder_and_name->folderId, phrase_block_id);
            addToPhraseByFolderList(folder_id, phrase_block_id);
          }

          phrase_folder_and_name->phraseBlockId = phrase_block_id;
          phrase_folder_and_name->folderId = folder_id;
          free(phrase_folder_and_name->name);
          phrase_folder_and_name->name = copyString((char*)phrase_name_str, phrase_name_length);
          serialDebugPrintf("Update phrase %s\r\n", phrase_folder_and_name->name);
        }
      }

      return { phrase_block_id, new_phrase_block_version, entropy, (bool)is_tombstone };
    } else {
      return BLOCK_NOT_UPDATED;
    }
  } else {
    return BLOCK_NOT_UPDATED;
  }
}

void startBlockCacheInit() {
  serialDebugPrintf("startBlockCacheInit!\r\n");
  // will be initialized by first tree_insert
  occupiedBlockNumbers = NULL;
  blockIdByBlockNumber = hashtable_create();
  blockInfos = hashtable_create();
  tombstonedBlockIds = hashtable_create();

  phrases = hashtable_create();
  phrases_by_folder = hashtable_create();

  serialDebugPrintf("startBlockCacheInit DONE\r\n");
}

void registerBlockInBlockCache(uint8_t* block, uint16_t block_number) {
  // 1. Update caches
  BlockIdAndVersion blockAndVersion = BLOCK_NOT_UPDATED;
  switch (block[0]) {
    case phraser_BlockType_KeyBlock: 
      blockAndVersion = setKeyBlock(block); 
      break;
    case phraser_BlockType_SymbolSetsBlock: 
      blockAndVersion = setSymbolSetsBlock(block); 
      break;
    case phraser_BlockType_FoldersBlock: 
      blockAndVersion = setFoldersBlock(block); 
      break;
    case phraser_BlockType_PhraseTemplatesBlock: 
      blockAndVersion = setPhraseTemplatesBlock(block); 
      break;
    case phraser_BlockType_PhraseBlock: 
      blockAndVersion = registerPhraseBlock(block); 
      break;
  }

  // 2. Update DB structures
  serialDebugPrintf("2. Update DB structures! %d \r\n", block[0]);
  serialDebugPrintf("blockAndVersion.blockId %d\r\n", blockAndVersion.blockId);
  if (blockAndVersion.blockId > 0) {
    serialDebugPrintf("2.1. Update holders of last values / counters\r\n");
    // 2.1. Update holders of last values / counters
    serialDebugPrintf("lastBlockId %d\r\n", lastBlockId);
    if (blockAndVersion.blockId > lastBlockId) {
      lastBlockId = blockAndVersion.blockId;
    }
    serialDebugPrintf("blockAndVersion.blockVersion %d\r\n", blockAndVersion.blockVersion);
    serialDebugPrintf("lastBlockVersion %d\r\n", lastBlockVersion);
    serialDebugPrintf("lastBlockNumber %d\r\n", lastBlockNumber);
    serialDebugPrintf("block_number %d\r\n", block_number);
    serialDebugPrintf("lastEntropy %d\r\n", lastEntropy);
    serialDebugPrintf("blockAndVersion.entropy %d\r\n", blockAndVersion.entropy);
    if (blockAndVersion.blockVersion >= lastBlockVersion) {
      // we need == part in  "blockAndVersion.blockVersion >= lastBlockVersion" 
      // for runtime updates, when we increment version first and then update cache
      lastBlockVersion = blockAndVersion.blockVersion;
      lastBlockNumber = block_number;
      lastEntropy = blockAndVersion.entropy;
    }

    serialDebugPrintf("2.2. Add BlockNumber to BlockId mapping\r\n");
    // 2.2. Add BlockNumber to BlockId mapping
    hashtable_set(blockIdByBlockNumber, block_number, (void*)blockAndVersion.blockId);

    serialDebugPrintf("2.3. Update BlockId to BlockNumberAndVersionAndCount mapping\r\n");
    // 2.3. Update BlockId to BlockNumberAndVersionAndCount mapping
    BlockNumberAndVersionAndCount* blockInfo = 
            (BlockNumberAndVersionAndCount*)hashtable_get(blockInfos, blockAndVersion.blockId);
    if (blockInfo == NULL) {
      blockInfo = (BlockNumberAndVersionAndCount*)malloc(sizeof(BlockNumberAndVersionAndCount));
      blockInfo->blockNumber = block_number;
      blockInfo->blockVersion = blockAndVersion.blockVersion;
      blockInfo->copyCount = 1;
      blockInfo->isTombstoned = blockAndVersion.isTombstoned;
      hashtable_set(blockInfos, blockAndVersion.blockId, blockInfo);
    } else {
      if (blockInfo->blockVersion < blockAndVersion.blockVersion) {
        blockInfo->blockNumber = block_number;
        blockInfo->blockVersion = blockAndVersion.blockVersion;
        blockInfo->isTombstoned = blockAndVersion.isTombstoned;
      }
      blockInfo->copyCount++;
    }
  }
}

void processTombstonedBlocks(hashtable *t, uint32_t key, void* value) {
  BlockNumberAndVersionAndCount* blockInfo = (BlockNumberAndVersionAndCount*)value;
  if (blockInfo->isTombstoned) {
    if (blockInfo->copyCount == 1) {
      void* removed_value = hashtable_remove(t, key);
      free(removed_value);//should be the same as parameter value;
    } else {
      hashtable_set(tombstonedBlockIds, key, (void*)key);
    }
  }
}

void removeFromOccupiedBlockNumbers(uint32_t key) {
  while (tree_delete(&occupiedBlockNumbers, key));
}

void addToOccupiedBlockNumbers(uint32_t key) {
  tree_insert(&occupiedBlockNumbers, key);
}

void formOccupiedBlockNumbers(hashtable *t, uint32_t key, void* value) {
  BlockNumberAndVersionAndCount* block_info = (BlockNumberAndVersionAndCount*)value;
  tree_insert(&occupiedBlockNumbers, block_info->blockNumber);
}

void finalizeBlockCacheInit() {
  // 3. Invalidate all Tombstoned blocks with a single copy, add those with multiple copies to `tombstoneBlockIds`.
  hashtable_iterate_entries(blockInfos, processTombstonedBlocks);

  // 4. Use blockInfos to add all actual block numbers to `occupiedBlockNumbers`
  hashtable_iterate_entries(blockInfos, formOccupiedBlockNumbers);
}

//  arraylist<FolderOrPhrase*>
arraylist* getFolderContent(uint16_t parent_folder_id) {
  arraylist* ret_list = arraylist_create();

  //  hashtable* sub_folders_by_folder;//uint32_t, arraylist<uint32_t>
  //  hashtable* folders;//uint32_t, Folder
  arraylist* subfolders = (arraylist*)hashtable_get(sub_folders_by_folder, parent_folder_id);
  if (subfolders != NULL) {
    for (int i = 0; subfolders != NULL && i < arraylist_size(subfolders); i++) {
      uint32_t subfolder_id = (uint32_t)arraylist_get(subfolders, i);
      Folder* subfolder = (Folder*)hashtable_get(folders, subfolder_id);
      if (subfolder != NULL) {
        FolderOrPhrase* folder_or_phrase = (FolderOrPhrase*)malloc(sizeof(FolderOrPhrase));
        folder_or_phrase->folder = subfolder;
        folder_or_phrase->phrase = NULL;

        arraylist_add(ret_list, (void*)folder_or_phrase);
      }
    }
  }

  //  hashtable* phrases_by_folder;//uint32_t, arraylist<uint32_t>
  //  hashtable* phrases;//uint32_t, PhraseFolderAndName
  arraylist* folderPhrases = (arraylist*)hashtable_get(phrases_by_folder, parent_folder_id);
  if (folderPhrases != NULL) {
    for (int i = 0; folderPhrases != NULL && i < arraylist_size(folderPhrases); i++) {
      uint32_t phrase_id = (uint32_t)arraylist_get(folderPhrases, i);
      PhraseFolderAndName* phrase = (PhraseFolderAndName*)hashtable_get(phrases, phrase_id);
      if (phrase != NULL) {
        FolderOrPhrase* folder_or_phrase = (FolderOrPhrase*)malloc(sizeof(FolderOrPhrase));
        folder_or_phrase->folder = NULL;
        folder_or_phrase->phrase = phrase;

        arraylist_add(ret_list, (void*)folder_or_phrase);
      }
    }
  }

  return ret_list;
}

PhraseFolderAndName* getPhrase(uint16_t phrase_id) {
  return (PhraseFolderAndName*)hashtable_get(phrases, phrase_id);
}

Folder root_folder_obj = { 0, 0, "/" };
extern Folder* getFolder(uint16_t folder_id) {
  if (folder_id == 0) {
    return &root_folder_obj;
  } else {
    return (Folder*)hashtable_get(folders, folder_id);
  }
}

// returns max_block_count
uint16_t db_block_count() {
  return max_block_count;
}

uint16_t valid_block_count() {
  return blockInfos->size;
}

uint16_t free_block_count() {
  return db_block_count() - valid_block_count();
}

bool last_block_left() {
  return free_block_count() <= 1;
}

uint32_t get_last_entropy() {
  return lastEntropy;
}

uint32_t increment_and_get_next_block_version() {
  return ++lastBlockVersion;
}

node_t* occupied_block_numbers() {
  return occupiedBlockNumbers;
}

uint32_t last_block_number() {
  return lastBlockNumber;
}

uint32_t last_block_version() {
  return lastBlockVersion;
}

uint32_t key_block_number() {
  BlockNumberAndVersionAndCount* blockNumberAndVersionAndCount = 
    (BlockNumberAndVersionAndCount*)hashtable_get(blockInfos, key_block_id);
  return blockNumberAndVersionAndCount->blockNumber;
}

uint32_t folders_block_number() {
  BlockNumberAndVersionAndCount* blockNumberAndVersionAndCount = 
    (BlockNumberAndVersionAndCount*)hashtable_get(blockInfos, folders_block_id);
  return blockNumberAndVersionAndCount->blockNumber;
}
