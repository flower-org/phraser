/**
 * Hashtable implementation
 * (c) 2011-2019 @marekweb
 *
 * Uses dynamic addressing with linear probing.
 */

 #include <stdlib.h>
 #include <assert.h>
 #include <string.h>
 #include "hashtable.h"
 
 #define HASHTABLE_INITIAL_CAPACITY 4
 uint32_t HASHTABLE_NULL_KEY = -1;

 /**
	* Compute the hash value for the given string.
	* Implements the djb k=33 hash function.
	*/
 unsigned long hashtable_hash(uint32_t key)
 {
	 return key;
 }
 
int key_cmp(uint32_t key1, uint32_t key2) {
	return key1-key2;
}

 /**
	* Find an available slot for the given key, using linear probing.
	*/
 unsigned int hashtable_find_slot(hashtable* t, uint32_t key)
 {
	int index = hashtable_hash(key) % t->capacity;
	//This won't infinite loop, because we pre-emptively resize on "set" on 80% capacity reached.
	while (t->body[index].key != HASHTABLE_NULL_KEY && key_cmp(t->body[index].key, key) != 0) {
		index = (index + 1) % t->capacity;
	}
	return index;
 }
 
 void hashtable_iterate_entries(hashtable *t, void (*func)(hashtable *t, uint32_t key, void* value)) {
		for (int i = 0; i < t->capacity; i++) {
			if (t->body[i].key != HASHTABLE_NULL_KEY) {
				func(t, t->body[i].key, t->body[i].value);
			}
		}	
 }

 /**
	* Return the item associated with the given key, or NULL if not found.
	*/
 void* hashtable_get(hashtable* t, uint32_t key)
 {
	 int index = hashtable_find_slot(t, key);
	 if (t->body[index].key != HASHTABLE_NULL_KEY) {
		 return t->body[index].value;
	 } else {
		 return NULL;
	 }
 }
 
 /**
	* Assign a value to the given key in the table.
	*/
 void* hashtable_set(hashtable* t, uint32_t key, void* value)
 {
	 int index = hashtable_find_slot(t, key);
	 if (t->body[index].key != HASHTABLE_NULL_KEY) {
		 /* Entry exists; update it. */
		 void* old_val = t->body[index].value;
		 t->body[index].value = value;
		 return old_val;
	 } else {
		 t->size++;
		 if ((float)t->size / t->capacity > 0.8) {
			 /* Resize the hash table if we reached 80% capacity */
			 hashtable_resize(t, t->capacity * 2);
			 index = hashtable_find_slot(t, key);
		 }
		 /* Create a new  entry */
		 t->body[index].key = key;
		 t->body[index].value = value;
		 return NULL;
	 }
 }
 
 /**
	* Remove a key from the table
	*/
 void* hashtable_remove(hashtable* t, uint32_t key)
 {
	 int index = hashtable_find_slot(t, key);
	 if (t->body[index].key != HASHTABLE_NULL_KEY) {
		 t->body[index].key = HASHTABLE_NULL_KEY;
		 void* old_val = t->body[index].value;
		 t->body[index].value = NULL;
		 t->size--;
		 return old_val;
		} else {
			return NULL;
		}
 }
 
 /**
	* Create a new, empty hashtable
	*/
 hashtable* hashtable_create()
 {
	 hashtable* new_ht = (hashtable*)malloc(sizeof(hashtable));
	 new_ht->size = 0;
	 new_ht->capacity = HASHTABLE_INITIAL_CAPACITY;
	 new_ht->body = hashtable_body_allocate(new_ht->capacity);
	 return new_ht;
 }
 
 /**
	* Allocate a new memory block with the given capacity.
	*/
 hashtable_entry* hashtable_body_allocate(unsigned int capacity)
 {
	 // calloc fills the allocated memory with zeroes
	 hashtable_entry* entries = (hashtable_entry*)calloc(capacity, sizeof(hashtable_entry));
	 for (int i = 0; i < capacity; i++) {
		entries[i].key = HASHTABLE_NULL_KEY;
	 }
	 return entries;
 }
 
 /**
	* Resize the allocated memory.
	*/
 void hashtable_resize(hashtable* t, unsigned int capacity)
 {
	 assert(capacity >= t->size);
	 unsigned int old_capacity = t->capacity;
	 hashtable_entry* old_body = t->body;
	 t->body = hashtable_body_allocate(capacity);
	 t->capacity = capacity;
 
	 // Copy all the old values into the newly allocated body
	 for (int i = 0; i < old_capacity; i++) {
		 if (old_body[i].key != HASHTABLE_NULL_KEY) {
			 hashtable_set(t, old_body[i].key, old_body[i].value);
		 }
	 }
 }
 
 /**
	* Destroy the table and deallocate it from memory. This does not deallocate the contained items.
	*/
 void hashtable_destroy(hashtable* t)
 {
	 free(t->body);
	 free(t);
 }
 