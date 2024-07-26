/*
 * Copyright (c) 2024, <Matei Tudor-Andrei>
 */

#include <stdio.h>
#include "server.h"
#include "lru_cache.h"
#include "constants.h"
#include "utils.h"




response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
	/* TODO */
	response *resp = malloc(sizeof(*resp));
	DIE(!resp, "malloc failed");
	resp->server_id = s->id;
	resp->server_response = malloc(MAX_RESPONSE_LENGTH);
	DIE(!resp->server_response, "malloc failed");
	resp->server_log = malloc(MAX_LOG_LENGTH);
	DIE(!resp->server_log, "malloc failed");
	void *temp = NULL;  // used to store the evicted key if there's one

	if (lru_cache_get(s->cache, doc_name) == NULL) {
		lru_cache_put(s->cache, doc_name, doc_content, &temp);
		// add the new file in cache
		if (temp) {
			// if there's an evicted key, free it and get the right log
			sprintf(resp->server_log, LOG_EVICT, doc_name, (char *)temp);
			free(temp);
		} else {
			sprintf(resp->server_log, LOG_MISS, doc_name);
		}
		for (int i = 0; i < s->db->size; i++) {
			if (!strcmp(doc_name, s->db->name[i])) {
				// update the content
				free(s->db->content[i]);
				s->db->content[i] = malloc(strlen(doc_content) + 1);
				DIE(!s->db->content[i], "malloc failed");
				strcpy(s->db->content[i], doc_content);
				sprintf(resp->server_response, MSG_B, doc_name);
				return resp;
			}
		}
		// if the file wasn't found, then create it
		s->db->size++;
		if (s->db->name) {
			s->db->name = realloc(s->db->name, s->db->size * sizeof(char *));
			s->db->content = realloc(s->db->content, s->db->size * sizeof(char *));
		} else {
			s->db->name = malloc(sizeof(char *));
			DIE(!s->db->name, "malloc failed");
			s->db->content = malloc(sizeof(char *));
			DIE(!s->db->content, "malloc failed");
		}
		int n = s->db->size;
		s->db->name[n - 1] = malloc(strlen(doc_name) + 1);
		DIE(!s->db->name[n - 1], "malloc failed");
		s->db->content[n - 1] = malloc(strlen(doc_content) + 1);
		DIE(!s->db->content[n - 1], "malloc failed");
		strcpy(s->db->name[n - 1], doc_name);
		strcpy(s->db->content[n - 1], doc_content);
		sprintf(resp->server_response, MSG_C, doc_name);
		return resp;
	}
	// if the file is in cache, search it there
	sprintf(resp->server_log, LOG_HIT, doc_name);
	lru_cache_put(s->cache, doc_name, doc_content, &temp);
	if (temp)
		free(temp);
	sprintf(resp->server_response, MSG_B, doc_name);
	for (int i = 0; i < s->db->size; i++) {  // update the database
		if (!strcmp(doc_name, s->db->name[i])) {
			free(s->db->content[i]);
			s->db->content[i] = malloc(strlen(doc_content) + 1);
			strcpy(s->db->content[i], doc_content);
			return resp;
		}
	}
	return NULL;
}

static response
*server_get_document(server *s, char *doc_name) {
	/* TODO */
	if (!doc_name)
		return NULL;
	response *resp = malloc(sizeof(*resp));
	DIE(!resp, "malloc failed");
	resp->server_log = malloc(MAX_LOG_LENGTH);
	DIE(!resp->server_log, "malloc failed");
	resp->server_id = s->id;
	void *temp;
	if (lru_cache_get(s->cache, doc_name) == NULL) {
		// if the file isn't in cache, then check in the database
		for (int i = 0; i < s->db->size; i++) {
			if (!strcmp(s->db->name[i], doc_name)) {
				resp->server_response = malloc(strlen(s->db->content[i]) + 1);
				strcpy(resp->server_response, s->db->content[i]);
				lru_cache_put(s->cache, doc_name, s->db->content[i], &temp);
				if (temp) {
					sprintf(resp->server_log, LOG_EVICT, doc_name,
							(char *)temp);
					free(temp);
				}
				else
					sprintf(resp->server_log, LOG_MISS, doc_name);
				return resp;
			}
		}
	// the file hasn't been found
	resp->server_response = malloc(10);
	sprintf(resp->server_response, "(null)");
	sprintf(resp->server_log, LOG_FAULT, doc_name);
	return resp;
	}

	// get the file dirrectly from cache
	lru_cache_put(s->cache, doc_name, lru_cache_get(s->cache, doc_name),
				  &temp);
	resp->server_response = malloc(strlen((char *)
								   lru_cache_get(s->cache, doc_name)) + 1);
	DIE(!resp->server_response, "malloc failed");
	strcpy(resp->server_response, (char *)lru_cache_get(s->cache, doc_name));
	sprintf(resp->server_log, LOG_HIT, doc_name);
	return resp;
}

server *init_server(unsigned int cache_size) {
	/* TODO */
	server *result = malloc(sizeof(server));
	DIE(!result, "malloc failed");
	result->q = q_create(sizeof(request), TASK_QUEUE_SIZE);
	result->db = malloc(sizeof(database_t));
	DIE(!result->db, "malloc failed");
	result->db->size = 0;
	result->db->name = NULL;
	result->db->content = NULL;
	result->cache = init_lru_cache(cache_size);
	return result;
}

response *server_handle_request(server *s, request *req) {
	/* TODO */
	if (req->type == EDIT_DOCUMENT) {
		response *resp = malloc(sizeof(*resp));
		DIE(!resp, "malloc failed");
		resp->server_response = malloc(MAX_RESPONSE_LENGTH);
		DIE(!resp->server_response, "malloc failed");
		resp->server_log = malloc(MAX_LOG_LENGTH);
		DIE(!resp->server_log, "malloc failed");
		resp->server_id = s->id;
		// add the request to the server's queue
		request temp;
		temp.type = req->type;
		temp.doc_name = malloc(strlen(req->doc_name) + 1);
		DIE(!temp.doc_name, "malloc failed");
		strcpy(temp.doc_name, req->doc_name);
		temp.doc_content = malloc(strlen(req->doc_content) + 1);
		DIE(!temp.doc_content, "malloc failed");
		strcpy(temp.doc_content, req->doc_content);
		q_enqueue(s->q, &temp);
		sprintf(resp->server_response, MSG_A, "EDIT", temp.doc_name);
		sprintf(resp->server_log, LOG_LAZY_EXEC, s->q->size);
		return resp;
	}

	// if a get request is received, then empty the queue
	while (!q_is_empty(s->q)) {
		request *front = (request *)q_front(s->q);
		response *temp1 = server_edit_document(s, front->doc_name,
											   front->doc_content);
		PRINT_RESPONSE(temp1);
		q_dequeue(s->q);
		free(front->doc_name);
		free(front->doc_content);
		if (front)
			free(front);
	}
	// get the wanted content
	response *ans = server_get_document(s, req->doc_name);
	return ans;
}


void empty_q(server *s)
{
	while (!q_is_empty(s->q)) {
		request *front = (request *)q_front(s->q);
		q_dequeue(s->q);
		free(front->doc_name);
		free(front->doc_content);
		if (front)
			free(front);
	}
}

void free_server(server **s) {
	/* TODO */
	free_lru_cache(&(*s)->cache);
	empty_q(*s);
	free((*s)->q->buff);
	free((*s)->q);

	for (int i = 0; i < (*s)->db->size; i++) {
		if ((*s)->db->name[i]) {
			free((*s)->db->name[i]);
			free((*s)->db->content[i]);
		}
	}
	free((*s)->db->name);
	free((*s)->db->content);
	free((*s)->db);
	free(*s);
}
