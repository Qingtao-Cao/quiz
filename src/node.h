#ifndef _NODE_H
#define _NODE_H

#include "lib.h"

#ifdef MULTI_THREADS
#include <pthread.h>
#endif

/*
 * Descriptor of a node in the analysis tree
 */
typedef struct node {
	/* Occurence of the word represented by this node */
	int cnt;

	/* The alphabet of this node */
	char c;

	/* Pointers to the next alphabets of potential words */
	struct node *children[AVAILABLE_CHARS];
} node_t;

node_t *create_node(const char c);
void destroy_node(node_t *node);
errcode_t setup_node(node_t *node, const char *word);
void dump_node(const node_t *node, const char *path);

#ifdef MULTI_THREADS
typedef struct root {
	node_t *n;
	pthread_mutex_t mutex;
} root_t;

root_t *create_tree(const char c);
void destroy_tree(root_t *root);
errcode_t setup_tree(root_t **root, const char *word);
void dump_tree(root_t *root);
#endif

#endif	/* _NODE_H */
