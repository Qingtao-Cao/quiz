
#ifndef _NODE_H
#define _NODE_H

#include "lib.h"

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
void destroy_tree(node_t *root);
errcode_t setup_tree(node_t *root, const char *word);
void dump_tree(const node_t *node, char *path);

#endif	/* _NODE_H */
