#include "DebugTest.h"

#include "rbtree.h"
#include "hashtable.h"
#include "pbkdf2-sha256.h"
#include "PhraserUtils.h"

static void perNode(data_t val) { Serial.printf("%u ", val); }

void outputTree(node_t *root) {
  traverse_inorder(root, perNode);
  Serial.printf("\r\n");
  traverse_inorder_backwards(root, perNode);
  Serial.printf("\r\n");
  traverse_right_excl(root, 15, perNode);
  Serial.printf("\r\n");
  traverse_left_excl(root, 15, perNode);
  Serial.printf("\r\n");
}

void rbtreeTest() {
  node_t *root = NULL;

  Serial.printf("START!\r\n");

  int testKeys[] = {10, 20, 30, 15, 15, 15, 25, 5, 35, 40};
  for (int i : testKeys) {
    tree_insert(&root, i);
  }

  outputTree(root);
  Serial.printf("\r\nHOP!\r\n");

  tree_delete(&root, 5);

  outputTree(root);

  Serial.printf("\r\nHOP2!\r\n");

  tree_delete(&root, 20);

  Serial.printf("\r\nHOPHOP!\r\n");

  outputTree(root);

  node_t* low = tree_minimum(root);
  Serial.printf("\rlow %u\r\n", low->_data);
  node_t* high = tree_maximum(root);
  Serial.printf("high %u\r\n", high->_data);

  node_t* higher = tree_higherKey(root, 25);
  Serial.printf("\rhigher %u\r\n", higher == NULL ? 0 : higher->_data);
  node_t* lower = tree_lowerKey(root, 25);
  Serial.printf("\rlower %u\r\n", lower == NULL ? 0 : lower->_data);

  higher = tree_higherKey(root, 18);
  Serial.printf("\rhigher %u\r\n", higher == NULL ? 0 : higher->_data);
  lower = tree_lowerKey(root, 18);
  Serial.printf("\rlower %u\r\n", lower == NULL ? 0 : lower->_data);

  higher = tree_higherKey(root, 33);
  Serial.printf("\rhigher %u\r\n", higher == NULL ? 0 : higher->_data);
  lower = tree_lowerKey(root, 33);
  Serial.printf("\rlower %u\r\n", lower == NULL ? 0 : lower->_data);

  higher = tree_higherKey(root, 5);
  Serial.printf("\rhigher %u\r\n", higher == NULL ? 0 : higher->_data);
  lower = tree_lowerKey(root, 5);
  Serial.printf("\rlower %u\r\n", lower == NULL ? 0 : lower->_data);

  higher = tree_higherKey(root, 45);
  Serial.printf("\rhigher %u\r\n", higher == NULL ? 0 : higher->_data);
  lower = tree_lowerKey(root, 45);
  Serial.printf("\rlower %u\r\n", lower == NULL ? 0 : lower->_data);

  tree_destroy(&root);
}

void perKvp(hashtable *t, uint32_t key, void* value) {
  uint32_t val = (uint32_t)value;
  if (val == 16) {
    Serial.printf("ITER: %d's value is: %d, REMOVING\n", key, val);
    hashtable_remove(t, val);
  } else {
    Serial.printf("ITER: %d's value is: %d\n", key, val);
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
  Serial.printf("%d's value is: %d\n", 15, result);

  int result2 = (int)hashtable_get(mytable, 0);
  Serial.printf("%d's value is: %d\n", 0, result2);

  hashtable_iterate_entries(mytable, perKvp);

  Serial.printf("======================\n");

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
  Serial.printf("HEX: %s\r\n", hex);
}

void debugTest() {
  sha256Test();

//  rbtreeTest();

//  hashtableTest();
}