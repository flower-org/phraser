#pragma once

#include <Thumby.h>

typedef struct hashtable hashtable;
void hashtable_destroy(hashtable *t);
typedef struct hashtable_entry hashtable_entry;
hashtable_entry *hashtable_body_allocate(unsigned int capacity);
hashtable *hashtable_create();
void* hashtable_remove(hashtable *t,uint32_t key);
void hashtable_resize(hashtable *t,unsigned int capacity);
void* hashtable_set(hashtable *t,uint32_t key,void *value);
void *hashtable_get(hashtable *t,uint32_t key);
unsigned int hashtable_find_slot(hashtable *t,uint32_t key);
unsigned long hashtable_hash(uint32_t str);
void hashtable_iterate_entries(hashtable *t, void (*func)(hashtable *t, uint32_t key, void* value));

struct hashtable {
	unsigned int size;
	unsigned int capacity;
	hashtable_entry* body;
};
struct hashtable_entry {
	uint32_t key;
	void* value;
};
#define INTERFACE 0
