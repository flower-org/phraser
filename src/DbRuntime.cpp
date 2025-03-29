#include <assert.h>

#include "DbRuntime.h"

#include "rbtree.h"
#include "hashtable.h"

uint16_t lastBlockId;
uint32_t lastBlockVersion;
uint32_t lastBlockNumber;

node_t* occupiedBlockNumbers;
hashtable* blockNumberAndVersionByBlockId;
hashtable* blockIdByBlockNumber;

void init() {
  // will be initialized by first tree_insert
  occupiedBlockNumbers = NULL;
  blockNumberAndVersionByBlockId = hashtable_create();
  blockIdByBlockNumber = hashtable_create();


}
