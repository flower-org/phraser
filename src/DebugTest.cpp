#include "DebugTest.h"

#include "rbtree.h"
#include "hashtable.h"

static void perNode(data_t val) { Serial.printf("%u ", val); }

void rbtreeTest() {
  node_t *root = NULL;

  Serial.printf("START!\r\n");

  int testKeys[] = {10, 20, 30, 15, 15, 15, 25, 5, 35, 40};
  for (int i : testKeys) {
    tree_insert(&root, i);
  }

  traverse_inorder(root, perNode);
  Serial.printf("\r\nHOP!\r\n");

  tree_delete(&root, 5);
  traverse_inorder(root, perNode);

  tree_delete(&root, 20);

  Serial.printf("\r\nHOPHOP!\r\n");

  traverse_inorder(root, perNode);
  Serial.println();

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

void hashtableTest() {
  int value1 = 456;
  hashtable* mytable = hashtable_create();
  hashtable_set(mytable, (char*)"foo", &value1);
  hashtable_remove(mytable, (char*)"boo");
  int* result = (int*)hashtable_get(mytable, (char*)"foo");
  Serial.printf("%s's value is: %d\n", "foo", *result);
}

void debugTest() {
  rbtreeTest();

  hashtableTest();
}