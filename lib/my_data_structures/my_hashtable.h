#pragma once

/*
 * Author : Pierre-Henri Symoneaux
 */

#include <stdlib.h>
#include <string.h>
#include "Arduino.h"

// Inititalize hashtable iterator on hashtable 'ht'
#define HT_ITERATOR(ht) {ht, 0, ht->table[0]}

//Hashtable element structure
typedef struct hash_elem_t {
  struct hash_elem_t* next; // Next element in case of a collision
  uint32_t data; // Pointer to the stored element
  uint32_t key;   // Key of the stored element
} hash_elem_t;

//Hashtabe structure
typedef struct {
  unsigned int capacity;  // Hashtable capacity (in terms of hashed keys)
  unsigned int e_num; // Number of element currently stored in the hashtable
  hash_elem_t** table;  // The table containaing elements
} hashtable_t;

//Structure used for iterations
typedef struct {
  hashtable_t* ht;  // The hashtable on which we iterate
  unsigned int index; // Current index in the table
  hash_elem_t* elem;  // Curent element in the list
} hash_elem_it;

/*  Create a hashtable with capacity 'capacity'
  and return a pointer to it*/
hashtable_t* ht_create(unsigned int capacity);

/*  Store data in the hashtable. If data with the same key are already stored,
  they are overwritten, and return by the function. Else it return NULL.
  Return HT_ERROR if there are memory alloc error*/
uint32_t ht_put(hashtable_t* hasht, uint32_t key, uint32_t data);

/* Retrieve data from the hashtable */
uint32_t ht_get(hashtable_t* hasht, uint32_t key);

/*  Remove data from the hashtable. Return the data removed from the table
  so that we can free memory if needed */
uint32_t ht_remove(hashtable_t* hasht, uint32_t key);

/* List keys. k should have length equals or greater than the number of keys */
void ht_list_keys(hashtable_t* hasht, uint32_t* k, size_t len);

/*  List values. v should have length equals or greater 
  than the number of stored elements */
void ht_list_values(hashtable_t* hasht, uint32_t* v, size_t len);

/* Iterate through table's elements. */
hash_elem_t* ht_iterate(hash_elem_it* iterator);

/* Iterate through keys. */
uint32_t ht_iterate_keys(hash_elem_it* iterator);

/* Iterate through values. */
uint32_t ht_iterate_values(hash_elem_it* iterator);

/*  Removes all elements stored in the hashtable.
  if free_data, all stored datas are also freed.*/
void ht_clear(hashtable_t* hasht);

/*  Destroy the hash table, and free memory.
  Data still stored are freed*/
void ht_destroy(hashtable_t* hasht);

/* Get number of table's elements. */
unsigned int ht_element_count(hashtable_t* hasht);
