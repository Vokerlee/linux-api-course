#include "hash_table.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <utime.h>
#include <sys/inotify.h>
#include <syslog.h>

/* Create a new hashtable. */
hashtable_t *ht_create(int size)
{
	hashtable_t *hashtable = NULL;
	int i = 0;

	if (size < 1)
		return NULL;

	if ((hashtable = malloc(sizeof(hashtable_t))) == NULL)
		return NULL;

	/* Allocate pointers to the head nodes. */
	if ((hashtable->table = malloc(sizeof(entry_t *) * size)) == NULL)
		return NULL;

	for (i = 0; i < size; i++)
		hashtable->table[i] = NULL;

	hashtable->size = size;
	hashtable->n_entries = 0;

	hashtable->start = NULL;
	hashtable->end = NULL;

	return hashtable;	
}

/* Hash a string for a particular hash table. */
size_t ht_hash(hashtable_t *hashtable, int key)
{
	if (hashtable == NULL)
		return -1;

	size_t hashval = key;

	hashval = ((hashval >> 16) ^ hashval) * 0x45d9f3b;
    hashval = ((hashval >> 16) ^ hashval) * 0x45d9f3b;
    hashval =  (hashval >> 16) ^ hashval;

	return hashval % hashtable->size;
}

/* Create a key-value pair. */
entry_t *ht_newpair(int key, char *value)
{
	entry_t *newpair = NULL;

	if ((newpair = malloc(sizeof(entry_t))) == NULL)
		return NULL;

	if ((newpair->value = strdup(value)) == NULL)
		return NULL;

	newpair->next = NULL;
	newpair->prev = NULL;
	newpair->key  = key;
	newpair->list_node = NULL;

	newpair->n_repeats = 0;

	return newpair;
}

/* Insert a key-value pair into a hash table. */
int ht_set(hashtable_t *hashtable, int key, char *value)
{
	if (hashtable == NULL)
		return -1;
	if (hashtable->size == hashtable->n_entries)
		return -1;

	int bin = 0;

	entry_t *newpair = NULL;
	entry_t *next    = NULL;
	entry_t *last    = NULL;

	bin = ht_hash(hashtable, key);
	next = hashtable->table[bin];

	while (next != NULL && next->key != key)
	{
		last = next;
		next = next->next;
	}

	/* There's already a pair.  Let's replace that string. */
	if (next != NULL && next->key == key)
	{
		free(next->value);
		next->value = strdup(value);
		if (next->value == NULL)
			return -1;

		next->n_repeats++;

	/* Nope, could't find it.  Time to grow a pair. */
	}
	else
	{
		newpair = ht_newpair(key, value);
		if (newpair == NULL)
			return -1;

		list_entry_t *list_node = calloc(1, sizeof(list_entry_t));
		if (list_node == NULL)
		{
			free(newpair);
			return -1;
		}

		if (hashtable->n_entries == 0)
		{
			hashtable->start = list_node;
			hashtable->end   = list_node;
		}

		/* We're at the start of the linked list in this bin. */
		if (next == hashtable->table[bin])
		{
			newpair->next = next;
			newpair->prev = next;
			hashtable->table[bin] = newpair;
	
		/* We're at the end of the linked list in this bin. */
		}
		else if (next == NULL)
		{
			last->next = newpair;
			newpair->prev = last;
	
		/* We're in the middle of the list. */
		}
		else 
		{
			newpair->next = next;
			newpair->prev = last;
			last->next = newpair;
		}

		list_node->hash_table_entry = newpair;

		if (hashtable->n_entries != 0)
		{
			hashtable->end->next = list_node;
			list_node->prev = hashtable->end;
		}
		
		hashtable->end = list_node;
		newpair->list_node = list_node;
	}

	hashtable->n_entries++;

	return 0;
}

/* Retrieve a key-value pair from a hash table. */
char *ht_get(hashtable_t *hashtable, int key)
{
	if (hashtable == NULL)
		return NULL;

	int bin = 0;
	entry_t *pair = NULL;

	bin = ht_hash(hashtable, key);
	pair = hashtable->table[bin];

	while (pair != NULL && pair->key != key)
		pair = pair->next;

	/* Did we actually find anything? */
	if (pair == NULL || pair->key != key)
		return NULL;
	else
		return pair->value;
}

int ht_remove(hashtable_t *hashtable, int key)
{
	if (hashtable == NULL)
		return -1;

	int bin = 0;
	entry_t *pair = NULL;

	bin = ht_hash(hashtable, key);
	pair = hashtable->table[bin];

	if (pair != NULL && pair->n_repeats == 0)
	{
		hashtable->n_entries--;
		if (pair->prev != NULL)
			pair->prev->next = NULL;

		struct list_entry_s *node = pair->list_node;
		struct list_entry_s *prev_node = node->prev;
		struct list_entry_s *next_node = node->next;

		if (hashtable->start == node)
			hashtable->start = node->next;
		if (hashtable->end == node)
			hashtable->end = node->prev;

		free(pair->value);
		free(pair);
		free(node);

		hashtable->table[bin] = NULL;

		if (prev_node != NULL)
			prev_node->next = next_node;

		if (next_node != NULL)
			next_node->prev = prev_node;

		return 0;
	}
	else if (pair != NULL)
	{
		pair->n_repeats--;
		return 0;
	}

	return -1;
}

int ht_delete(hashtable_t *hashtable)
{
	int error = ht_clear(hashtable);
	if (error == -1)
		return -1;

	free(hashtable->table);
	free(hashtable);

	return 0;
}

int ht_clear(hashtable_t *hashtable)
{
	if (hashtable == NULL)
		return -1;

	list_entry_t *list_entry = hashtable->start;

    while (list_entry)
    {
		list_entry_t *list_entry_next = list_entry->next;

		free(list_entry->hash_table_entry->value);
		free(list_entry->hash_table_entry);
		free(list_entry);

        list_entry = list_entry_next;
    }

	hashtable->start = NULL;
	hashtable->end   = NULL;

	hashtable->n_entries = 0;

	return 0;
}


