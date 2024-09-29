/*
 * Author : Pierre-Henri Symoneaux
 */

#include <stdlib.h>
#include <string.h>
#include "my_hashtable.h"

char err_ptr;
void* HT_ERROR = &err_ptr; // Data pointing to HT_ERROR are returned in case of error

/*  Internal funcion to calculate hash for keys.
  It's based on the DJB algorithm from Daniel J. Bernstein.
  The key must be ended by '\0' character.*/
static unsigned int ht_calc_hash(uint32_t key)
{
  return key;
}

/*  Create a hashtable with capacity 'capacity'
  and return a pointer to it*/
hashtable_t* ht_create(unsigned int capacity)
{
  hashtable_t* hasht = malloc(sizeof(hashtable_t));
  if(!hasht)
    return NULL;
  if((hasht->table = malloc(capacity*sizeof(hash_elem_t*))) == NULL)
  {
    free(hasht->table);
    return NULL;
  }
  hasht->capacity = capacity;
  hasht->e_num = 0;
  unsigned int i;
  for(i = 0; i < capacity; i++)
    hasht->table[i] = NULL;
  return hasht;
}

/*  Store data in the hashtable. If data with the same key are already stored,
  they are overwritten, and return by the function. Else it return NULL.
  Return HT_ERROR if there are memory alloc error*/
uint32_t ht_put(hashtable_t* hasht, uint32_t key, uint32_t data)
{
  if(data == 0)
    return 0;
  unsigned int h = ht_calc_hash(key) % hasht->capacity;
  hash_elem_t* e = hasht->table[h];

  while(e != NULL)
  {
    if(e->key == key)
    {
      uint32_t ret = e->data;
      e->data = data;
      return ret;
    }
    e = e->next;
  }

  // Getting here means the key doesn't already exist
  if((e = malloc(sizeof(hash_elem_t))) == NULL)
    return 0;
  e->key = key;
  e->data = data;

  // Add the element at the beginning of the linked list
  e->next = hasht->table[h];
  hasht->table[h] = e;
  hasht->e_num ++;

  return 0;
}

/* Retrieve data from the hashtable */
uint32_t ht_get(hashtable_t* hasht, uint32_t key)
{
  unsigned int h = key % hasht->capacity;
  hash_elem_t* e = hasht->table[h];
  while(e != NULL)
  {
    if(e->key == key)
      return e->data;
    e = e->next;
  }
  return 0;
}

/*  Remove data from the hashtable. Return the data removed from the table
  so that we can free memory if needed */
uint32_t ht_remove(hashtable_t* hasht, uint32_t key)
{
  unsigned int h = key % hasht->capacity;
  hash_elem_t* e = hasht->table[h];
  hash_elem_t* prev = NULL;
  while(e != NULL)
  {
    if(e->key == key)
    {
      uint32_t ret = e->data;
      if(prev != NULL)
        prev->next = e->next;
      else
        hasht->table[h] = e->next;
      free(e);
      e = NULL;
      hasht->e_num --;
      return ret;
    }
    prev = e;
    e = e->next;
  }
  return 0;
}

/* List keys. k should have length equals or greater than the number of keys */
void ht_list_keys(hashtable_t* hasht, uint32_t* k, size_t len)
{
  if(len < hasht->e_num)
    return;
  int ki = 0; //Index to the current string in **k
  int i = hasht->capacity;
  while(--i >= 0)
  {
    hash_elem_t* e = hasht->table[i];
    while(e)
    {
      k[ki++] = e->key;
      e = e->next;
    }
  }
}

/*  List values. v should have length equals or greater 
  than the number of stored elements */
void ht_list_values(hashtable_t* hasht, uint32_t* v, size_t len)
{
  if(len < hasht->e_num)
    return;
  int vi = 0; //Index to the current string in **v
  int i = hasht->capacity;
  while(--i >= 0)
  {
    hash_elem_t* e = hasht->table[i];
    while(e)
    {
      v[vi++] = e->data;
      e = e->next;
    }
  }
}

/* Iterate through table's elements. */
hash_elem_t* ht_iterate(hash_elem_it* iterator)
{
  while(iterator->elem == NULL)
  {
    if(iterator->index < iterator->ht->capacity - 1)
    {
      iterator->index++;
      iterator->elem = iterator->ht->table[iterator->index];
    }
    else
      return NULL;
  }
  hash_elem_t* e = iterator->elem;
  if(e)
    iterator->elem = e->next;
  return e;
}

/* Iterate through keys. */
uint32_t ht_iterate_keys(hash_elem_it* iterator)
{
  hash_elem_t* e = ht_iterate(iterator);
  return (e == NULL ? 0 : e->key);
}

/* Iterate through values. */
uint32_t ht_iterate_values(hash_elem_it* iterator)
{
  hash_elem_t* e = ht_iterate(iterator);
  return (e == NULL ? 0 : e->data);
}

/*  Removes all elements stored in the hashtable.
  if free_data, all stored datas are also freed.*/
void ht_clear(hashtable_t* hasht)
{
  hash_elem_it it = HT_ITERATOR(hasht);
  uint32_t k = ht_iterate_keys(&it);
  while(k != 0)
  {
    ht_remove(hasht, k);
    k = ht_iterate_keys(&it);
  }
}

/*  Destroy the hash table, and free memory.
  Data still stored are freed*/
void ht_destroy(hashtable_t* hasht)
{
  ht_clear(hasht); // Delete and free all.
  free(hasht->table);
  free(hasht);
}

unsigned int ht_element_count(hashtable_t* hasht) {
  return hasht->e_num;
}
