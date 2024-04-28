/* Hash table implementation. Based on TCL hash tables.
 * Tables may be indexed in three ways:
 *   1. by string.
 *   2. by integer.
 *   3. by array of integers. ((2) is the special case where length == 1)
 * Hash tables automatically grow to keep the value/bucket ratio below
 * the REBUILD_MULTIPLIER. However, they will not shrink.
 */
/*
 * TPM -- General purpose C data structure library.
 * Copyright (C) 1995 Thomas P. Minka <tpminka@media.mit.edu>
 * Thomas P. Minka, 20 Ames Street (E15-389), Cambridge, MA 02139.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "hash.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUCKETS 4
#define REBUILD_MULTIPLIER 3
#define DEBUG 0

/* Prototypes ****************************************************************/

HashTable HashTableCreate(int key_type, FreeFunc *ff);
void HashTableFree(HashTable ht);
void HashTableFlush(HashTable ht);

HashEntry *HashTableAddEntry(HashTable ht, HashKey key, int *found_return);
HashEntry *HashTableFindEntry(HashTable ht, HashKey key);
void HashTableRemoveEntry(HashTable ht, HashEntry *entry);

/* Private */
static void RebuildTable(HashTable ht);

/* Functions *****************************************************************/

/* used by HashTableCreate and HashTableFlush */
static void initialize(HashTable ht)
{
  int i;
  ht->num_buckets = INITIAL_BUCKETS;
  ht->mask = ht->num_buckets - 1; /* assumes num_buckets is power of two */
  ht->down_shift = 28;
  ht->num_entries = 0;
  ht->buckets = Allocate(ht->num_buckets, HashEntry *);
  for(i=0;i<ht->num_buckets;i++) ht->buckets[i] = NULL;
}

/* Create an empty hash table.
 * If key_type == 0, then it will use string keys.
 * Otherwise, it will use integer arrays of length key_type.
 * If ff != NULL, it will be used to free values in the table.
 */
HashTable HashTableCreate(int key_type, FreeFunc *ff)
{
  HashTable ht = Allocate(1, struct HashTableStruct);
  ht->key_type = key_type;
  ht->ff = ff;
  initialize(ht);
  return ht;
}

/* used by HashTableFlush and HashTableFree */
static void HashTableRemoveAll(HashTable ht)
{
  int b;
  HashEntry *e, *t;

  for(b=0;b<ht->num_buckets;b++) {
    for(e=ht->buckets[b];e;e=t) {
      if(ht->ff) ht->ff(e->value);
      t = e->next;
      free(e);
    }
  }
  ht->num_entries = 0;
}

/* Destroys the contents of the HashTable, but not the HashTable itself. */
void HashTableFlush(HashTable ht)
{
  HashTableRemoveAll(ht);
  free(ht->buckets);
  initialize(ht);
}

/* Destroys a HashTable, using the ff to free the stored values. */
void HashTableFree(HashTable ht)
{
  HashTableRemoveAll(ht);
  free(ht->buckets);
  free(ht);
}

/* taken from Tcl7.3. Produces a hash key for a string. */
static int HashString(char *string)
{
  int result = 0;
  char c;
  for(;;) {
    c = *string++;
    if(!c) break;
    result += (result<<3) + c;
  }
  return result;
}

/* Computes the hash bucket for the given key */
int HashBucket(HashTable ht, HashKey key)
{
  int i, b;
  if(ht->key_type == HASH_STRING) b = HashString(key);
  else {
    if(ht->key_type > 1) {
      b = *(int*)key;
      for(i=1;i<ht->key_type;i++) {
	b += ((int*)key)[i];
      }
    }
    else b = (int)key;
    b = (b * 1103515245) >> ht->down_shift;
  }
  return (b & ht->mask);
}

/* Used by HashTableFindEntry and HashTableLookupEntry. 
 * Returns the bucket index and HashEntry corresponding to key.
 */
static HashEntry *LookupEntry(HashTable ht, HashKey key, int *bucket)
{
  int b, i;
  HashEntry *e;

  b = HashBucket(ht, key);
#if DEBUG
  printf("hashes to %d\n", b);
#endif
  *bucket = b;
  if(ht->key_type == HASH_STRING) {
    for(e = ht->buckets[b]; e; e=e->next)
      if(!strcmp(key, e->key)) break;
  }
  else if(ht->key_type == HASH_WORD) {
    for(e = ht->buckets[b]; e; e=e->next)
      if(key == e->key) break;
  }
  else {
    for(e = ht->buckets[b]; e; e=e->next) {
      /* compare the arrays (key, e->key) */
      for(i=0;i<ht->key_type;i++) {
	if(((int*)key)[i] != ((int*)e->key)[i]) break;
      }
      if(i == ht->key_type) break;
    }
  }
  return e;
}

/* If ht has no entries matching the key, returns a new HashEntry 
 * corresponding to that key (which has been entered into the hash table) 
 * into which data may be placed, and sets found_return to 0.
 * Otherwise, returns the matching entry and sets found_return to 1.
 */
HashEntry *HashTableAddEntry(HashTable ht, HashKey key, int *found_return)
{
  int b;
  HashEntry *e;
  e = LookupEntry(ht, key, &b);
  if(!e) {
    *found_return = 0;
    if(ht->key_type == HASH_STRING) {
      e = (HashEntry*)malloc(sizeof(HashEntry) + 
			     strlen(key) + 1 - sizeof(e->keydata));
      e->key = e->keydata;
      strcpy(e->keydata, key);
    }
    else if(ht->key_type == HASH_WORD) {
      e = Allocate(1, HashEntry);
      e->key = key;
    }
    else {
      e = (HashEntry*)malloc(sizeof(HashEntry) + 
			     ht->key_type*sizeof(int) - sizeof(e->keydata));
      e->key = e->keydata;
      memcpy(e->keydata, key, ht->key_type*sizeof(int));
    }
    e->bucket = &ht->buckets[b];
    e->next = *e->bucket;
    ht->buckets[b] = e;

    ht->num_entries++;
    if(ht->num_entries >= REBUILD_MULTIPLIER * ht->num_buckets) 
      RebuildTable(ht);
  }
  else *found_return = 1;
  return e;
}

/* Returns the HashEntry matching the key, or NULL if there is none. */
HashEntry *HashTableFindEntry(HashTable ht, HashKey key)
{
  int b;
  return LookupEntry(ht, key, &b);
}

/* Removes the entry from the hash table, and uses the ff to free the 
 * entry data.
 */
void HashTableRemoveEntry(HashTable ht, HashEntry *entry)
{
  HashEntry **e;

  for(e = entry->bucket; *e != entry; e = &(*e)->next);
  *e = entry->next;
  if(ht->ff) ht->ff(entry->value);
  free(entry);
}

/* used to expand the table. */
static void RebuildTable(HashTable ht)
{
  HashEntry **old_buckets;
  HashEntry *e, *t;
  int i,b, old_size;

  old_size = ht->num_buckets;
  old_buckets = ht->buckets;

  ht->num_buckets *= 4;
  ht->down_shift -= 2;
  ht->mask = (ht->mask << 2) + 3;
  ht->buckets = Allocate(ht->num_buckets, HashEntry *);
  for(i=0;i<ht->num_buckets;i++) ht->buckets[i] = NULL;
  
  /* rehash all entries */
  for(i=0;i<old_size;i++) {
    for(e=old_buckets[i];e;e=t) {
      b = HashBucket(ht, e->key);
      e->bucket = &ht->buckets[b];
      t = e->next;
      e->next = *e->bucket;
      ht->buckets[b] = e;
    }
  }
  free(old_buckets);
}
