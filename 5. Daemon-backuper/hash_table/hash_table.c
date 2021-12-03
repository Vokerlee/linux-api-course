#include "hash_table.h"

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
	entry_t *next    = NULL;newpair->next = NULL;
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

	/* Nope, could't find it.  Time to grow a pair. */
	}
	else
	{
		newpair = ht_newpair(key, value);
		if (newpair == NULL)
			return -1;

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

	if (pair != NULL)
	{
		hashtable->n_entries--;
		if (pair->prev != NULL)
			pair->prev->next = NULL;

		free(pair->value);
		free(pair);

		hashtable->table[bin] = NULL;

		return 0;
	}

	return -1;
}
