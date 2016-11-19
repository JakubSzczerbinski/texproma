/*
 * Copyright (c) 2015 Nuxi, https://nuxi.nl/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <errno.h>
#include <limits.h>
#include <search.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ansi.h"
#include "word.h"
#include "config.h"
#include "dict.h"

struct dict {
  size_t offset_basis;  /* Initial value for FNV-1a hashing. */
  size_t index_mask;    /* Bitmask for indexing the table. */
  size_t entries_used;  /* Number of entries currently used. */
  entry_t *entries;     /* Hash table entries. */
};

dict_t *dict_new() {
  dict_t *hsearch = malloc(sizeof(dict_t));

  /*
   * Allocate a hash table object. Resizing the table dynamically if the use
   * increases a threshold does not affect the worst-case running time.
   */
  hsearch->entries = calloc(16, sizeof(entry_t));
  if (hsearch->entries == NULL) {
    free(hsearch);
    return NULL;
  }

  /*
   * Pick a random initialization for the FNV-1a hashing. This makes it
   * hard to come up with a fixed set of keys to force hash collisions.
   */
  hsearch->offset_basis = random();
  hsearch->index_mask = 0xf;
  hsearch->entries_used = 0;
  return hsearch;
}

void dict_reset(dict_t *dict) {
  entry_t *entry = NULL;

  while (dict_iter(dict, &entry)) {
    free(entry->key);
    if (entry->word)
      word_delete(entry->word);
    memset(entry, 0, sizeof(entry_t));
  }
}

void dict_delete(dict_t *dict) {
  dict_reset(dict);
  free(dict->entries);
  free(dict);
}

/*
 * Look up an unused entry in the hash table for a given hash. For this
 * implementation we use quadratic probing. Quadratic probing has the
 * advantage of preventing primary clustering.
 */
static entry_t *dict_lookup_free(dict_t *hsearch, size_t hash) {
  size_t index, i;

  for (index = hash, i = 0;; index += ++i) {
    entry_t *entry = &hsearch->entries[index & hsearch->index_mask];
    if (entry->key == NULL)
      return entry;
  }
}

/*
 * Computes an FNV-1a hash of the key. Depending on the pointer size, this
 * either uses the 32- or 64-bit FNV prime.
 */
static size_t hsearch_hash(size_t offset_basis, const char *str) {
  size_t hash;

  hash = offset_basis;
  while (*str != '\0') {
    hash ^= (uint8_t)*str++;
    if (sizeof(size_t) * CHAR_BIT <= 32)
      hash *= UINT32_C(16777619);
    else
      hash *= UINT64_C(1099511628211);
  }
  return (hash);
}

entry_t *dict_find(dict_t *hsearch, const char *key) {
  size_t hash = hsearch_hash(hsearch->offset_basis, key);

  /*
   * Search the hash table for an existing entry for this key.
   * Stop searching if we run into an unused hash table entry.
   */
  for (size_t index = hash, i = 0;; index += ++i) {
    entry_t *entry = &hsearch->entries[index & hsearch->index_mask];
    if (entry->key == NULL)
      break;
    if (strcmp(entry->key, key) == 0)
      return entry;
  }

  return NULL;
}

entry_t *dict_add(dict_t *hsearch, const char *key) {
  entry_t *entry;

  /*
   * Search the hash table for an existing entry for this key.
   * Stop searching if we run into an unused hash table entry.
   */
  size_t hash = hsearch_hash(hsearch->offset_basis, key);

  for (size_t index = hash, i = 0;; index += ++i) {
    entry = &hsearch->entries[index & hsearch->index_mask];
    if (entry->key == NULL)
      break;
    if (strcmp(entry->key, key) == 0)
      return entry;
  }

  if (hsearch->entries_used * 2 >= hsearch->index_mask) {
    /* Preserve the old hash table entries. */
    size_t old_count = hsearch->index_mask + 1;
    entry_t *old_entries = hsearch->entries;

    /*
     * Allocate and install a new table if insertion would
     * yield a hash table that is more than 50% used. By
     * using 50% as a threshold, a lookup will only take up
     * to two steps on average.
     */
    size_t new_count = (hsearch->index_mask + 1) * 2;
    entry_t *new_entries = calloc(new_count, sizeof(entry_t));
    if (new_entries == NULL)
      return NULL;
    hsearch->entries = new_entries;
    hsearch->index_mask = new_count - 1;

    /* Copy over the entries from the old table to the new table. */
    for (size_t i = 0; i < old_count; ++i) {
      entry = &old_entries[i];
      if (entry->key != NULL) {
        size_t old_hash = hsearch_hash(hsearch->offset_basis, entry->key);
        *dict_lookup_free(hsearch, old_hash) = *entry;
      }
    }

    /* Destroy the old hash table entries. */
    free(old_entries);

    /*
     * Perform a new lookup for a free table entry, so that
     * we insert the entry into the new hash table.
     */
    entry = dict_lookup_free(hsearch, hash);
  }

  /* Insert the new entry into the hash table. */
  entry->key = strdup(key);
  entry->word = NULL;
  ++hsearch->entries_used;
  return entry;
}

bool dict_iter(dict_t *dict, entry_t **entry_p) {
  entry_t *entry = *entry_p;
  entry_t *end = &dict->entries[dict->index_mask];

  if (entry == NULL)
    entry = dict->entries;
  else
    entry++;

  for (; entry <= end; entry++) {
    if (entry->key == NULL)
      continue;
    *entry_p = entry;
    return true;
  }

  return false;
}
