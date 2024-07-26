/*
 * Copyright (c) 2024, <Matei Tudor-Andrei>
 */

#include <stdio.h>
#include <string.h>
#include "lru_cache.h"
#include "utils.h"

lru_cache *init_lru_cache(unsigned int cache_capacity) {
	/* TODO */
	lru_cache *cache = malloc(sizeof(lru_cache));
	DIE(!cache, "malloc failed");
	cache->max_size = cache_capacity;
	cache->dll = dll_create(DOC_NAME_LENGTH);
	cache->ht = ht_create(cache_capacity, hash_string, compare_function_strings
						  , key_val_free_function);
	cache->size = 0;
	return cache;
}

bool lru_cache_is_full(lru_cache *cache) {
	/* TODO */
	return cache->size == cache->max_size;
}

void free_lru_cache(lru_cache **cache) {
	/* TODO */
	ht_free((*cache)->ht);
	dll_free(&(*cache)->dll);
	free(*cache);
}

int find_node_pos(doubly_linked_list_t *dll, char *data)
{
	int i = 0;
	dll_node_t *temp = dll->head;
	while (temp != dll->head->prev) {
		if (!strcmp((char *)temp->data, data))
			return i;
		i++;
		temp = temp->next;
	}
	return i;
}

bool lru_cache_put(lru_cache *cache, void *key, void *value,
				   void **evicted_key) {
	/* TODO */
	if (!cache || !cache->ht || !cache->dll)
		return false;
	char *copy = malloc(DOC_NAME_LENGTH);
	DIE(!copy, "malloc failed");
	strcpy(copy, (char *)key);
	if (!lru_cache_get(cache, key)) {
		if (!lru_cache_is_full(cache)) {
			// if the cache is not full, then just add the new key
			*evicted_key = NULL;
			ht_put(cache->ht, key, strlen(key) + 1, value, strlen(value) + 1);
			dll_add_nth_node(cache->dll, cache->size, copy);
			cache->size++;
			free(copy);

			return true;
		}
		// if the cache is full, then evict the oldest key and add the new one
		dll_node_t *temp = dll_remove_nth_node(cache->dll, 0);
		*evicted_key = malloc(strlen(temp->data) + 1);
		memcpy(*evicted_key, temp->data, strlen(temp->data) + 1);
		ht_remove_entry(cache->ht, *evicted_key);
		ht_put(cache->ht, key, strlen(key) + 1, value, strlen(value) + 1);
		dll_add_nth_node(cache->dll, cache->dll->size, copy);
		free(temp->data);
		free(temp);
		free(copy);
		return true;
	}
	// if the key is already in cache, then simply move it at the end
	int pos = find_node_pos(cache->dll, copy);
	*evicted_key = NULL;
	ht_put(cache->ht, key, strlen(key) + 1, value, strlen((char *)value) + 1);
	dll_node_t *temp2 = dll_remove_nth_node(cache->dll, pos);
	dll_add_nth_node(cache->dll, cache->size, copy);
	free(temp2->data);
	free(temp2);
	free(copy);
	return true;
}

void *lru_cache_get(lru_cache *cache, void *key) {
	/* TODO */
	return ht_get(cache->ht, key);
}

void lru_cache_remove(lru_cache *cache, void *key) {
	/* TODO */
	ht_remove_entry(cache->ht, key);
	char *copy = malloc(DOC_NAME_LENGTH);
	DIE(!copy, "malloc failed");
	strcpy(copy, (char *)key);
	int pos = find_node_pos(cache->dll, copy);
	dll_node_t *rem = dll_remove_nth_node(cache->dll, pos);
	free(rem->data);
	free(copy);
	free(rem);
	cache->size--;
}
