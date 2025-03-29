#include "DebugTest.h"

#include "rbtree.h"
#include "hashtable.h"

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

void perKvp(uint32_t key, void* value) {
  Serial.printf("ITER: %d's value is: %d\n", key, *(int*)value);
}

void hashtableTest() {
  hashtable* mytable = hashtable_create();

  int value1 = 456;
  hashtable_set(mytable, 15, &value1);

  hashtable_remove(mytable, 27);// returns NULL

  int value2 = 789;
  hashtable_set(mytable, 0, &value2);

  int* result = (int*)hashtable_get(mytable, 15);
  Serial.printf("%d's value is: %d\n", 15, *result);

  int* result2 = (int*)hashtable_get(mytable, 0);
  Serial.printf("%d's value is: %d\n", 0, *result2);

  iterate_entries(mytable, perKvp);
}

void debugTest() {
  rbtreeTest();

  hashtableTest();
}