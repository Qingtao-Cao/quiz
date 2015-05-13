
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "node.h"

node_t *create_node(const char c)
{
	node_t *node;

	if (!(node = (node_t *)malloc(sizeof(node_t)))) {
		printf("Failed to allocate a tree node\n");
		return NULL;
	}

	memset(node, 0, sizeof(node_t));

	node->c = c;

	return node;
}

void destroy_tree(node_t *root)
{
	int i;

	if (!root) {
		return;
	}

	for (i = 0; i < AVAILABLE_CHARS; i++) {
		destroy_tree(root->children[i]);
	}

	free(root);
}

errcode_t setup_tree(node_t *root, const char *word)
{
	node_t *p;
	int len, i, idx;
	char c;

	assert(root && word);

	p = root;
	len = strlen(word);

	for (i = 0; i < len; i++) {
		c = word[i];

		if (c >= 'A' && c <= 'Z') {
			c += 'a' - 'A';
		}

		/* Illegal word, skip it, resulting in leaf node's cnt == 0 */
		if (c < 'a' || c > 'z') {
			return 0;
		}

		idx = c - 'a';
		if (!p->children[idx]) {
			if (!(p->children[idx] = create_node(c))) {
				return ERR_NO_MEM;
			}
		}

		p = p->children[idx];
	}

	/* update counter on the leaf node */
	p->cnt++;

	return 0;
}

void dump_tree(const node_t *node, char *path)
{
	node_t *child;
	char *path_new;
	int i;

	assert (node && path);	/* path could point to an empty string but never NULL */

	if (node->c == 0) {	/* root node */
		if (!(path_new = strdup(path))) {
			return;
		}
	} else {
		if (!(path_new = (char *)malloc(strlen(path) + 2))) {
			return;
		}

		sprintf(path_new, "%s%c", path, node->c);
	}

	if (node->cnt) {
		printf("%s : %d\n", path_new, node->cnt);
	}

	for (i = 0; i < AVAILABLE_CHARS; i++) {
		if (!(child = node->children[i])) {
			continue;
		}

		dump_tree(child, path_new);
	}

	free(path_new);
}
