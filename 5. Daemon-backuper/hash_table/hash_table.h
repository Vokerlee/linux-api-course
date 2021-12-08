#define _XOPEN_SOURCE 500 /* Enable certain library functions (strdup) on linux.  See feature_test_macros(7) */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

struct list_entry_s;

struct entry_s
{
	int key;
	char *value;

	struct entry_s *next;
	struct entry_s *prev;

	struct list_entry_s *list_node;

	int n_repeats;
};

struct list_entry_s
{
	struct entry_s *hash_table_entry;

	struct list_entry_s *next;
	struct list_entry_s *prev;
};

typedef struct entry_s entry_t;
typedef struct list_entry_s list_entry_t;

struct hashtable_s
{
	size_t size;
	entry_t **table;	

	list_entry_t *start;
	list_entry_t *end;

	size_t n_entries;
};

typedef struct hashtable_s hashtable_t;

hashtable_t *ht_create(int size);

size_t ht_hash(hashtable_t *hashtable, int key);

entry_t *ht_newpair(int key, char *value);

int ht_set(hashtable_t *hashtable, int key, char *value);

char *ht_get(hashtable_t *hashtable, int key);

int ht_remove(hashtable_t *hashtable, int key);

int ht_clear(hashtable_t *hashtable);

int ht_delete(hashtable_t *hashtable);
