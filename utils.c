/*
 * Copyright (c) 2024, <Matei Tudor-Andrei>
 */

#include "utils.h"




unsigned int hash_uint(void *key)
{
	unsigned int uint_key = *((unsigned int *)key);

	uint_key = ((uint_key >> 16u) ^ uint_key) * 0x45d9f3b;
	uint_key = ((uint_key >> 16u) ^ uint_key) * 0x45d9f3b;
	uint_key = (uint_key >> 16u) ^ uint_key;

	return uint_key;
}

unsigned int hash_string(void *key)
{
	unsigned char *key_string = (unsigned char *) key;
	unsigned int hash = 5381;
	int c;

	while ((c = *key_string++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

char *get_request_type_str(request_type req_type) {
	switch (req_type) {
	case ADD_SERVER:
		return ADD_SERVER_REQUEST;
	case REMOVE_SERVER:
		return REMOVE_SERVER_REQUEST;
	case EDIT_DOCUMENT:
		return EDIT_REQUEST;
	case GET_DOCUMENT:
		return GET_REQUEST;
	}

	return NULL;
}

request_type get_request_type(char *request_type_str) {
	request_type type;

	if (!strncmp(request_type_str,
				 ADD_SERVER_REQUEST, strlen(ADD_SERVER_REQUEST)))
		type = ADD_SERVER;
	else if (!strncmp(request_type_str,
					  REMOVE_SERVER_REQUEST, strlen(REMOVE_SERVER_REQUEST)))
		type = REMOVE_SERVER;
	else if (!strncmp(request_type_str,
					  EDIT_REQUEST, strlen(EDIT_REQUEST)))
		type = EDIT_DOCUMENT;
	else if (!strncmp(request_type_str,
					  GET_REQUEST, strlen(GET_REQUEST)))
		type = GET_DOCUMENT;
	else
		DIE(1, "unknown request type");

	return type;
}

queue_t *
q_create(unsigned int data_size, unsigned int max_size)
{
	/* TODO */
	queue_t* ans = malloc(sizeof(queue_t));
	ans->buff = malloc(sizeof(void *) * max_size);
	for (unsigned int i = 0; i < max_size; i++)
		ans->buff[i] = NULL;
	ans->max_size = max_size;
	ans->data_size = data_size;
	ans->read_idx = 0;
	ans->write_idx = 0;
	ans->size = 0;
	return ans;
}

unsigned int
q_get_size(queue_t *q)
{
	/* TODO */
	if (q->write_idx >= q->read_idx)
		return q->write_idx - q->read_idx;
	return q->max_size - q->write_idx - 1;
}

unsigned int
q_is_empty(queue_t *q)
{
	/* TODO */
	if (q->size == 0)
		return 1;

	return 0;
}

void *
q_front(queue_t *q)
{
	/* TODO */
	void* temp = q->buff[q->read_idx];
	return temp;
}

int
q_dequeue(queue_t *q)
{
	/* TODO */
	if (q_is_empty(q))
		return 0;
	q->read_idx = (q->read_idx + 1) % q->max_size;
	q->size--;
	return 1;
}

int
q_enqueue(queue_t *q, void *new_data)
{
	/* TODO */
	if (q_get_size(q) == q->max_size)
		return 0;
	q->buff[q->write_idx] = malloc(q->data_size);
	DIE(!q->buff[q->write_idx], "ERROR");
	memcpy(q->buff[q->write_idx], new_data, q->data_size);
	q->write_idx = (q->write_idx + 1) % q->max_size;
	q->size++;
	return 1;
}

void
q_clear(queue_t *q)
{
	/* TODO */
	while (!q_is_empty(q))
		q_dequeue(q);
}

void
q_free(queue_t *q)
{
	/* TODO */
	for (unsigned int i = 0; i < q->max_size; i++) {
		if (q->buff[i])
			free(q->buff[i]);
	}
	free(q->buff);
}

linked_list_t *ll_create(unsigned int data_size)
{
	linked_list_t* ll;

	ll = malloc(sizeof(*ll));

	ll->head = NULL;
	ll->data_size = data_size;
	ll->size = 0;

	return ll;
}

void ll_add_nth_node(linked_list_t* list, int n, const void* new_data)
{
	ll_node_t *prev, *curr;
	ll_node_t* new_node;

	if (!list) {
		return;
	}

	if (n > list->size) {
		n = list->size;
	}

	curr = list->head;
	prev = NULL;
	while (n > 0) {
		prev = curr;
		curr = curr->next;
		--n;
	}

	new_node = malloc(sizeof(*new_node));
	new_node->data = malloc(list->data_size);
	memcpy(new_node->data, new_data, list->data_size);

	new_node->next = curr;
	if (prev == NULL) {
		list->head = new_node;
	} else {
		prev->next = new_node;
	}

	list->size++;
}

ll_node_t *ll_remove_nth_node(linked_list_t* list, int n)
{
	ll_node_t *prev, *curr;

	if (!list || !list->head) {
		return NULL;
	}

	if (n > list->size - 1) {
		n = list->size - 1;
	}

	curr = list->head;
	prev = NULL;
	while (n > 0) {
		prev = curr;
		curr = curr->next;
		--n;
	}

	if (prev == NULL) {
		list->head = curr->next;
	} else {
		prev->next = curr->next;
	}

	list->size--;

	return curr;
}


unsigned int ll_get_size(linked_list_t* list)
{
	 if (!list) {
		return -1;
	}

	return list->size;
}

void ll_free(linked_list_t** pp_list)
{
	ll_node_t* currNode;

	if (!pp_list || !*pp_list) {
		return;
	}

	while (ll_get_size(*pp_list) > 0) {
		currNode = ll_remove_nth_node(*pp_list, 0);
		free(currNode->data);
		currNode->data = NULL;
		free(currNode);
		currNode = NULL;
	}

	free(*pp_list);
	*pp_list = NULL;
}

void ll_print_int(linked_list_t* list)
{
	ll_node_t* curr;

	if (!list) {
		return;
	}

	curr = list->head;
	while (curr != NULL) {
		printf("%d ", *((int*)curr->data));
		curr = curr->next;
	}

	printf("\n");
}

void ll_print_string(linked_list_t* list)
{
	ll_node_t* curr;

	if (!list) {
		return;
	}

	curr = list->head;
	while (curr != NULL) {
		printf("%s ", (char*)curr->data);
		curr = curr->next;
	}

	printf("\n");
}




int compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

void key_val_free_function(void *data) {
	info* temp = (info *)data;
	if (temp->key)
		free(temp->key);
	if (temp->value)
		free(temp->value);
	free(temp);
}


hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
	hashtable_t* result = malloc(sizeof(*result));
	result->buckets = malloc(sizeof(linked_list_t*) * hmax);
	result->hmax = hmax;
	result->hash_function = hash_function;
	result->compare_function = compare_function;
	result->key_val_free_function = key_val_free_function;
	for (unsigned int i = 0; i < hmax; i++)
		result->buckets[i] = ll_create(sizeof(info));
	return result;
}


int ht_has_key(hashtable_t *ht, void *key)
{
	/* TODO */
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t* temp = ht->buckets[index]->head;
	while (temp) {
		info* data = temp->data;
		if (!ht->compare_function(key, data->key))
			return 1;
		temp = temp->next;
	}
	return 0;
}

void *ht_get(hashtable_t *ht, void *key)
{
	/* TODO */
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t* temp = ht->buckets[index]->head;
	while (temp) {
		info* dat = temp->data;
		if (!ht->compare_function(key, dat->key))
			return dat->value;
		temp = temp->next;
	}
	return NULL;
}


void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	/* TODO */
	unsigned int index = ht->hash_function(key) % ht->hmax;
	info* data = malloc(sizeof(info));
	data->key = malloc(key_size);
	data->value = malloc(value_size);
	memcpy(data->key, key, key_size);
	memcpy(data->value, value, value_size);
	ll_node_t* temp = ht->buckets[index]->head;
	while (temp) {
		info* dat = temp->data;
		if (!ht->compare_function(key, dat->key)) {
			key_val_free_function(dat);
			temp->data = data;
			return;
		}
		temp = temp->next;
	}
	ll_add_nth_node(ht->buckets[index], ht->buckets[index]->size, data);
	free(data);
	ht->size++;
}

void ht_remove_entry(hashtable_t *ht, void *key)
{
	if (!ht) {
		return;
	}
	int index = ht->hash_function(key) % ht->hmax;
	ll_node_t* first = ht->buckets[index]->head;

	int i = 0;
	while (first) {
		if (ht->compare_function(((info*)first->data)->key, key) == 0) {
			ht->key_val_free_function((info*)first->data);
			ll_node_t *tmp = ll_remove_nth_node(ht->buckets[index], i);
			free(tmp);
			ht->size--;
			return;
		}
		i++;
		first = first->next;
	}
}

void ht_free(hashtable_t *ht)
{
	/* TODO */
	for (unsigned int i = 0; i < ht->hmax; i++) {
		ll_node_t* temp = ht->buckets[i]->head;
		while (temp) {
			ll_remove_nth_node(ht->buckets[i], 0);
			key_val_free_function(temp->data);
			free(temp);
			temp = ht->buckets[i]->head;
		}
		free(ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}

unsigned int ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}

doubly_linked_list_t*
dll_create(unsigned int data_size)
{
	/* TODO */
	doubly_linked_list_t* ans = malloc(sizeof(*ans));
	ans->head = NULL;
	ans->data_size = data_size;
	ans->size = 0;
	return ans;
}

dll_node_t*
dll_get_nth_node(doubly_linked_list_t* list, unsigned int n)
{
	/* TODO */
	if (list->size == 0)
		return NULL;
	n = n % list->size;
	dll_node_t* node = list->head;
	while (n--) {
		node = node->next;
	}
	return node;
}

void
dll_add_nth_node(doubly_linked_list_t* list, unsigned int n,
				 const void* new_data)
{
	/* TODO */
	dll_node_t *node = malloc(sizeof(dll_node_t));
	DIE(!node, "ERROR");
	node->data = malloc(list->data_size);
	DIE(!node->data, "ERROR");
	memcpy(node->data, new_data, list->data_size);
	if (list->size == 0) {
		node->next = node;
		node->prev = node;
		list->head = node;
		list->size++;
		return;
	}
	if (n == 0) {
		dll_node_t *current = list->head->prev;
		dll_node_t *nxt = list->head;
		node->next = nxt;
		node->prev = current;
		current->next = node;
		nxt->prev = node;
		list->head = node;
		list->size++;

		return;
	}
	if (n >= list->size) {
		n = list->size - 1;
		dll_node_t *head = list->head;
		dll_node_t *tail = list->head->prev;
		node->next = head;
		node->prev = tail;
		head->prev = node;
		tail->next = node;
		list->size++;

		return;
	}
	n--;
	dll_node_t *current = dll_get_nth_node(list, n);
	dll_node_t *nxt = current->next;
	node->next = current->next;
	node->prev = nxt->prev;
	current->next = node;
	nxt->prev = node;
	list->size++;
}

dll_node_t*
dll_remove_nth_node(doubly_linked_list_t* list, unsigned int n)
{
	/* TODO */
	if (n == 0) {
		dll_node_t *ans = list->head;
		dll_node_t *tail = list->head->prev;
		dll_node_t *new_h = list->head->next;
		list->head = new_h;
		tail->next = new_h;
		new_h->prev = tail;
		list->size--;
		return ans;
	}
	if (n >= list->size) {
		dll_node_t *ans = list->head->prev;
		dll_node_t *new_t = ans->prev;
		list->head->prev = new_t;
		new_t->next = list->head;
		list->size--;
		return ans;
	}
	dll_node_t *ans = dll_get_nth_node(list, n);
	dll_node_t *next = ans->next;
	dll_node_t *prev = ans->prev;
	next->prev = prev;
	prev->next = next;
	list->size--;
	return ans;
}

unsigned int
dll_get_size(doubly_linked_list_t* list)
{
	/* TODO */
	unsigned int c = 0;
	dll_node_t *temp = list->head;
	if (temp)
		c++;
	temp = temp->next;
	while (temp != list->head) {
		c++;
		temp = temp->next;
	}
	return c;
}

void
dll_free(doubly_linked_list_t** pp_list)
{
	/* TODO */
	dll_node_t *temp = (*pp_list)->head;
	while ((*pp_list)->size--) {
		dll_node_t *next = temp->next;
		free(temp->data);
		free(temp);
		temp = next;
	}
	free(*pp_list);
}

void
dll_print_int_list(doubly_linked_list_t* list)
{
	/* TODO */

	dll_node_t *temp = list->head;
	if (!temp)
		return;
	printf("%d ", *(int *)temp->data);
	temp = temp->next;
	while (temp != list->head) {
		printf("%d ", *(int *)temp->data);
		temp = temp->next;
	}
	printf("\n");
}

void
dll_print_string_list(doubly_linked_list_t* list)
{
	/* TODO */
	dll_node_t *temp = list->head->prev;
	if (!temp)
		return;
	printf("%s ", (char *)temp->data);
	temp = temp->prev;
	while (temp != list->head->prev) {
		printf("%s ", (char *)temp->data);
		temp = temp->prev;
	}
	printf("\n");
}
