/* Definition of HashTable */
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

#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

#ifndef Allocate
#define Allocate(n,t) (t*)malloc((n)*sizeof(t))
#endif

typedef void FreeFunc(void *);

/* #include "tpm.h" */

/* Key types */
enum {
  HASH_STRING,
  HASH_WORD
  };

typedef char *HashKey;

typedef struct HashEntryStruct {
  void *value;
  struct HashEntryStruct **bucket;
  struct HashEntryStruct *next;
  char *key;
  char keydata[sizeof(int)]; /* must be last field */
} HashEntry;

typedef struct HashTableStruct {
  int key_type;
  int num_buckets, num_entries;
  int mask, down_shift;
  HashEntry **buckets;
  FreeFunc *ff;
} *HashTable;

#define HashEntryValue(h) (h)->value
#define HashEntryKey(h) (h)->key

#define HashIter(b, e, ht) \
  {int b; for(b=0; b<(ht)->num_buckets; b++) \
          for(e=(ht)->buckets[b]; e; e=e->next)

#endif

HashTable HashTableCreate(int key_type, FreeFunc *ff);
void HashTableFree(HashTable ht);
void HashTableFlush(HashTable ht);

HashEntry *HashTableAddEntry(HashTable ht, HashKey key, int *found_return);
HashEntry *HashTableFindEntry(HashTable ht, HashKey key);
void HashTableRemoveEntry(HashTable ht, HashEntry *entry);
