/*
 * Copyright (c) 2024, <Matei Tudor-Andrei>
 */

#include "load_balancer.h"
#include "server.h"
#include "utils.h"

load_balancer *init_load_balancer(bool enable_vnodes) {
	/* TODO */
	if (enable_vnodes)
		return NULL;
	load_balancer *result = malloc(sizeof(*result));
	DIE(!result, "malloc failed");
	result->hash_function_servers = hash_uint;
	result->hash_function_docs = hash_string;
	result->size = 0;
	result->servers = malloc(sizeof(*(result->servers)) * MAX_SERVERS);
	DIE(!result->servers, "malloc failed");
	return result;
}

void sort(server **servers, int n, unsigned int (*hash)(void *))
{
	for (int i = 0 ; i < n - 1; i++) {
		for (int j = i + 1; j < n; j++) {
			if (hash(&servers[i]->id) > hash(&servers[j]->id)) {
				server *temp = servers[i];
				servers[i] = servers[j];
				servers[j] = temp;
			}
		}
	}
}

int reloc_docs(load_balancer* main, server *s1, server *s2, int c)
{
	int rem_count = 0;
	request *req = malloc(sizeof(*req));
	DIE(!req, "malloc failed()");
	req->type = GET_DOCUMENT;
	req->doc_name = NULL;
	req->doc_content = NULL;
	server_handle_request(s1, req);
	// we will send a "fake" get request so it empties the queue
	for (int i = 0; i < (s1)->db->size; i++) {
		if (main->hash_function_docs((s1)->db->name[i]) <
			main->hash_function_servers(&(s2)->id) ||
			main->hash_function_docs((s1)->db->name[i]) >
			main->hash_function_servers(&(s1)->id)) {
			if (c) {
				// if c is 1, then we added a server at the end, so we make
				// sure it doesn't get filled with any documents that have
				// bigger hashes or smaller hashes than the first one
				if (main->hash_function_docs(s1->db->name[i]) <
					main->hash_function_servers(&s1->id))
					continue;
				if (main->hash_function_docs(s1->db->name[i]) >
					main->hash_function_servers(&s2->id))
					continue;
			}
			if (rem_count) {
				s2->db->name = realloc((s2)->db->name, sizeof(char *) *
										 (rem_count + 1));
				DIE(!s2->db->name, "realloc failed");
				s2->db->content = realloc((s2)->db->content, sizeof(char *)
											* (rem_count + 1));
				DIE(!s2->db->content, "realloc failed");
			} else {
				(s2)->db->name = malloc(sizeof(char *));
				DIE(!s2->db->name, "malloc failed");
				(s2)->db->content = malloc(sizeof(char *));
				DIE(!s2->db->content, "malloc failed");
			}
			if (lru_cache_get((s1)->cache, (s1)->db->name[i]))
				// if the doc is in the cache, remove it
				lru_cache_remove((s1)->cache, (s1)->db->name[i]);
			// we add the docs in the new server
			(s2)->db->name[rem_count] = malloc(strlen((s1)->db->name[i]) + 1);
			DIE(!s2->db->name, "malloc failed");
			(s2)->db->content[rem_count] = malloc(strlen((s1)->db->content[i])
												  + 1);
			DIE(!s2->db->content, "malloc failed");
			strcpy((s2)->db->name[rem_count], (s1)->db->name[i]);
			strcpy((s2)->db->content[rem_count], (s1)->db->content[i]);
			free((s1)->db->name[i]);
			free((s1)->db->content[i]);
			for (int j = i; j < (s1)->db->size - 1; j++) {
				(s1)->db->name[j] = (s1)->db->name[j + 1];
				(s1)->db->content[j] = (s1)->db->content[j + 1];
			}

			// modify the sizes accordingly
			rem_count++;
			i--;
			s1->db->size--;
			s2->db->size++;
		}
	}
	free(req);
	return rem_count;
}

void loader_add_server(load_balancer* main, int server_id, int cache_size)
{
	/* TODO: Remove test_server after checking the server implementation */
	main->servers[main->size] = init_server(cache_size);
	main->servers[main->size]->id = server_id;
	// we sort the servers by their hash so it's easier to add new ones
	sort(main->servers, main->size + 1, main->hash_function_servers);
	main->size++;
	if (main->size == 1)
		return;
	int pos;
	for (int i = 0; i < main->size; i++) {
		if (server_id == main->servers[i]->id) {
			pos = i;
			break;
		}
	}
	int d;
	if (pos == main->size - 1)
		d = 0;
	else
		d = pos + 1;
	int x = 0;
	if (d == 0)
		x = 1;

	int c = reloc_docs(main, main->servers[d], main->servers[pos], x);
	// if there are no differences, then we need to skip this part
	if (c && main->servers[d]->db->size) {
		main->servers[d]->db->name = realloc(main->servers[d]->db->name,
											 (main->servers[d]->db->size)*
											 sizeof(char *));

		main->servers[d]->db->content = realloc(main->servers[d]->db->content,
												(main->servers[d]->db->size) *
												sizeof(char *));
	}
}

void shift_left(server **servers, int n, int pos)
{
	for (int i = pos; i < n - 1; i++) {
		servers[i] = servers[i + 1];
	}
}

void move_load(server *s1, server *s2)
{
	if (s1->db->size + s2->db->size == 0)
		return;
	// allocate the necessary memory
	if (s2->db->name) {
		(s2)->db->name = realloc((s2)->db->name,
								 ((s1)->db->size + (s2)->db->size) *
								  sizeof(char *));
		s2->db->content = realloc(s2->db->content,
									((s1)->db->size + (s2)->db->size)
									* sizeof(char *));


	} else {
		s2->db->name = malloc(s1->db->size * sizeof(char *));
		DIE(!s2->db->name, "malloc failed");
		s2->db->content = malloc(s1->db->size * sizeof(char *));
		DIE(!s2->db->name, "malloc failed");
	}

	// move all files from server 1 to server 2
	for (int i = 0; i < (s1)->db->size; i++) {
		(s2)->db->name[(s2)->db->size + i] = malloc(strlen((s1)->db->name[i])
													+ 1);
		DIE(!(s2)->db->name[(s2)->db->size + i], "malloc failed");
		(s2)->db->content[(s2)->db->size + i] = malloc(strlen((s1)->db->
													   content[i]) + 1);
		DIE(!(s2)->db->content[(s2)->db->size + i], "malloc failed");
		strcpy((s2)->db->name[(s2)->db->size + i], (s1)->db->name[i]);
		strcpy((s2)->db->content[(s2)->db->size + i], (s1)->db->content[i]);
	}
	(s2)->db->size += (s1)->db->size;
}

void loader_remove_server(load_balancer* main, int server_id) {
	/* TODO */
	for (int i = 0 ; i < main->size - 1; i++) {
		if (main->servers[i]->id == server_id) {
			server *s = main->servers[i];
			request *req = malloc(sizeof(*req));
			req->type = GET_DOCUMENT;
			req->doc_name = NULL;
			req->doc_content = NULL;
			server_handle_request(s, req);
			move_load(main->servers[i], main->servers[i + 1]);
			free_server(&main->servers[i]);
			shift_left(main->servers, main->size, i);
			main->size--;
			free(req);
			return;
		}
	}
	// in case we want to remove  the last server
	server *s = main->servers[main->size - 1];
	request *req = malloc(sizeof(*req));
	req->type = GET_DOCUMENT;
	req->doc_name = NULL;
	req->doc_content = NULL;
	server_handle_request(s, req);
	move_load(main->servers[main->size - 1], main->servers[0]);
	free_server(&main->servers[main->size - 1]);
	main->size--;
	free(req);
}

response *loader_forward_request(load_balancer* main, request *req) {
	/* TODO */
	unsigned int hash = main->hash_function_docs(req->doc_name);
	for (int i = 0; i < main->size; i++) {
		// find the server that needs to handle the request
		if (hash <= main->hash_function_servers(&main->servers[i]->id)) {
			response *ans = server_handle_request(main->servers[i], req);
			return ans;
		}
	}
	response *ans = server_handle_request(main->servers[0], req);
	return ans;
}

void free_load_balancer(load_balancer** main) {
	/* TODO: get rid of test_server after testing the server implementation */
	for (int i = 0; i < (*main)->size; i++)
		free_server(&(*main)->servers[i]);
	free((*main)->servers);
	free(*main);
	*main = NULL;
}
