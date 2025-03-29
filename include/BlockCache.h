#pragma once

#include <Thumby.h>
#include <Arduino.h>

#include "PhraserUtils.h"
#include "Schema_reader.h"

extern uint8_t* key_block_buffer;

extern uint8_t HARDCODED_SALT[];
extern const int HARDCODED_SALT_LEN;

extern uint8_t HARDCODED_IV_MASK[];
extern const int IV_MASK_LEN;

extern const int PBKDF_INTERATIONS_COUNT;


// - Login data cache
extern uint8_t* key_block_key;
extern uint32_t key_block_key_length;

// - KeyBlock cache
extern uint16_t key_block_id;
extern uint8_t* key_block_buffer;

extern uint16_t block_count;
extern uint8_t* main_key;
extern uint32_t main_key_length;
extern uint8_t* main_iv_mask;
extern uint32_t main_iv_mask_length;

// - SymbolSetsBlock cache
extern uint16_t symbol_sets_block_id;
extern uint8_t* symbol_sets_block_buffer;

// - FoldersBlock cache
extern uint16_t folders_block_id;
extern uint8_t* folders_block_buffer;
/*final Map<Integer, Folder> folders;
final Map<Integer, Set<Integer>> subFoldersByFolder;*/

// - PhraseTemplatesBlock cache
extern uint16_t phrase_templates_block_id;
extern uint8_t* phrase_templates_block_buffer;
/*final Map<Integer, PhraseTemplate> phraseTemplates;
final Map<Integer, WordTemplate> wordTemplates;*/

// - Phrase Blocks (minimal info) cache
/*final Map<Integer, PhraseFolderAndName> phrases;
final Map<Integer, Set<Integer>> phrasesByFolder;*/

void setLoginData(uint8_t* key, uint32_t key_length);

void startBlockCacheInit();
void registerBlockInBlockCache(uint8_t* block, uint16_t block_number);
void finalizeBlockCacheInit();
