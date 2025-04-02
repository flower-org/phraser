#include "DebugTest.h"

#include "rbtree.h"
#include "hashtable.h"
#include "pbkdf2-sha256.h"
#include "PhraserUtils.h"
#include "SerialUtils.h"
#include "DefaultDbInitializer.h"
#include "BlockCache.h"
#include "BlockDAO.h"

static bool perNode(data_t val) { serialDebugPrintf("%u ", val); return true; }

void outputTree(node_t *root) {
  traverse_inorder(root, perNode);
  serialDebugPrintf("\r\n");
  traverse_inorder_backwards(root, perNode);
  serialDebugPrintf("\r\n");
  traverse_right_excl(root, 15, perNode);
  serialDebugPrintf("\r\n");
  traverse_left_excl(root, 15, perNode);
  serialDebugPrintf("\r\n");
}

void rbtreeTest() {
  node_t *root = NULL;

  serialDebugPrintf("START!\r\n");

  int testKeys[] = {10, 20, 30, 15, 15, 15, 25, 5, 35, 40};
  for (int i : testKeys) {
    tree_insert(&root, i);
  }

  outputTree(root);
  serialDebugPrintf("\r\nHOP!\r\n");

  tree_delete(&root, 5);

  outputTree(root);

  serialDebugPrintf("\r\nHOP2!\r\n");

  tree_delete(&root, 20);

  serialDebugPrintf("\r\nHOPHOP!\r\n");

  outputTree(root);

  node_t* low = tree_minimum(root);
  serialDebugPrintf("\rlow %u\r\n", low->_data);
  node_t* high = tree_maximum(root);
  serialDebugPrintf("high %u\r\n", high->_data);

  node_t* higher = tree_higherKey(root, 25);
  serialDebugPrintf("\rhigher %u\r\n", higher == NULL ? 0 : higher->_data);
  node_t* lower = tree_lowerKey(root, 25);
  serialDebugPrintf("\rlower %u\r\n", lower == NULL ? 0 : lower->_data);

  higher = tree_higherKey(root, 18);
  serialDebugPrintf("\rhigher %u\r\n", higher == NULL ? 0 : higher->_data);
  lower = tree_lowerKey(root, 18);
  serialDebugPrintf("\rlower %u\r\n", lower == NULL ? 0 : lower->_data);

  higher = tree_higherKey(root, 33);
  serialDebugPrintf("\rhigher %u\r\n", higher == NULL ? 0 : higher->_data);
  lower = tree_lowerKey(root, 33);
  serialDebugPrintf("\rlower %u\r\n", lower == NULL ? 0 : lower->_data);

  higher = tree_higherKey(root, 5);
  serialDebugPrintf("\rhigher %u\r\n", higher == NULL ? 0 : higher->_data);
  lower = tree_lowerKey(root, 5);
  serialDebugPrintf("\rlower %u\r\n", lower == NULL ? 0 : lower->_data);

  higher = tree_higherKey(root, 45);
  serialDebugPrintf("\rhigher %u\r\n", higher == NULL ? 0 : higher->_data);
  lower = tree_lowerKey(root, 45);
  serialDebugPrintf("\rlower %u\r\n", lower == NULL ? 0 : lower->_data);

  tree_destroy(&root);
}

void perKvp(hashtable *t, uint32_t key, void* value) {
  uint32_t val = (uint32_t)value;
  if (val == 16) {
    serialDebugPrintf("ITER: %d's value is: %d, REMOVING\n", key, val);
    hashtable_remove(t, val);
  } else {
    serialDebugPrintf("ITER: %d's value is: %d\n", key, val);
  }
}

void hashtableTest() {
  hashtable* mytable = hashtable_create();

  hashtable_set(mytable, 15, (void*)456);
  hashtable_set(mytable, 16, (void*)16);
  hashtable_set(mytable, 17, (void*)17);
  hashtable_set(mytable, 18, (void*)18);
  hashtable_set(mytable, 19, (void*)19);

  hashtable_remove(mytable, 27);// returns NULL

  hashtable_set(mytable, 0, (void*)789);

  int result = (int)hashtable_get(mytable, 15);
  serialDebugPrintf("%d's value is: %d\n", 15, result);

  int result2 = (int)hashtable_get(mytable, 0);
  serialDebugPrintf("%d's value is: %d\n", 0, result2);

  hashtable_iterate_entries(mytable, perKvp);

  serialDebugPrintf("======================\n");

  hashtable_iterate_entries(mytable, perKvp);
}

void sha256Test() {
  const unsigned char* test_string = (const unsigned char*)"TEST SRING HERE";
  int length = strlen((const char*)test_string);
  const unsigned char* test_string2 = (const unsigned char*)"ALsDSDSSGD";
  int length2 = strlen((const char*)test_string);
  const unsigned char* test_string3 = (const unsigned char*)"iuwiojlkmewdslk";
  int length3 = strlen((const char*)test_string);

	unsigned char sha2sum[32];
	sha2_context ctx;
  sha2_starts(&ctx, 0);
  sha2_update(&ctx, test_string, length);
  sha2_update(&ctx, test_string2, length2);
  sha2_update(&ctx, test_string3, length2);
  sha2_finish(&ctx, sha2sum);

  char* hex = bytesToHexString(sha2sum, 32);
  serialDebugPrintf("HEX: %s\r\n", hex);
}

void objectMutationTest() {
  uint8_t block[FLASH_SECTOR_SIZE];

  uint8_t out_new_aes_key[32];
  uint8_t out_new_aes_iv_mask[16];

  serialDebugPrintf("Key\r\n");
  initDefaultKeyBlock(block, HARDCODED_SALT, HARDCODED_IV_MASK, out_new_aes_key, out_new_aes_iv_mask, 128);
  if (updateVersionAndEntropyBlock(block, FLASH_SECTOR_SIZE, HARDCODED_SALT, HARDCODED_IV_MASK, increment_and_get_next_block_version(), true) == ERROR) {
    serialDebugPrintf("Error\r\n");
  }

  serialDebugPrintf("SymbolSets\r\n");
  initDefaultSymbolSetsBlock(block, HARDCODED_SALT, HARDCODED_IV_MASK);
  if (updateVersionAndEntropyBlock(block, FLASH_SECTOR_SIZE, HARDCODED_SALT, HARDCODED_IV_MASK, increment_and_get_next_block_version(), true) == ERROR) {
    serialDebugPrintf("Error\r\n");
  }

  serialDebugPrintf("Folder\r\n");
  initDefaultFoldersBlock(block, HARDCODED_SALT, HARDCODED_IV_MASK);
  if (updateVersionAndEntropyBlock(block, FLASH_SECTOR_SIZE, HARDCODED_SALT, HARDCODED_IV_MASK, increment_and_get_next_block_version(), true) == ERROR) {
    serialDebugPrintf("Error\r\n");
  }

  serialDebugPrintf("PhraseTemplate\r\n");
  initDefaultPhraseTemplatesBlock(block, HARDCODED_SALT, HARDCODED_IV_MASK);
  if (updateVersionAndEntropyBlock(block, FLASH_SECTOR_SIZE, HARDCODED_SALT, HARDCODED_IV_MASK, increment_and_get_next_block_version(), true) == ERROR) {
    serialDebugPrintf("Error\r\n");
  }

  serialDebugPrintf("Phrase\r\n");
  initDefaultPhraseBlock(block, HARDCODED_SALT, HARDCODED_IV_MASK);
  if (updateVersionAndEntropyBlock(block, FLASH_SECTOR_SIZE, HARDCODED_SALT, HARDCODED_IV_MASK, increment_and_get_next_block_version(), true) == ERROR) {
    serialDebugPrintf("Error\r\n");
  }
}

void debugTest() {
  objectMutationTest();

  //  sha256Test();

  //  rbtreeTest();

  //  hashtableTest();
}