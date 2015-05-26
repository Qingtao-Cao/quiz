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

void destroy_node(node_t *node)
{
	int i;

	if (!node) {
		return;
	}

	for (i = 0; i < AVAILABLE_CHARS; i++) {
		destroy_node(node->children[i]);
	}

	free(node);
}

errcode_t setup_node(node_t *node, const char *word)
{
	node_t *p;
	int len, i, idx;
	char c;

	len = strlen(word);

	assert(node && word && len > 0);

	for (i = 0, p = node; i < len; i++) {
		/* Illegal word, skip it, resulting in leaf node's cnt == 0 */
		if ((c = to_lowercase(word[i])) < 0) {
			return 0;
		}

		idx = c - 'a';

		if (p->children[idx] != NULL) {
			p = p->children[idx];
			continue;
		}

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

void dump_node(const node_t *node, const char *path)
{
	node_t *child;
	char *path_new;
	int i;

	assert (node && path);	/* path could point to an empty string but never NULL */

	if (!(path_new = (char *)malloc(strlen(path) + 2))) {
		return;
	}

	sprintf(path_new, "%s%c", path, node->c);

	if (node->cnt) {
		printf("%s : %d\n", path_new, node->cnt);
	}

	for (i = 0; i < AVAILABLE_CHARS; i++) {
		if (!(child = node->children[i])) {
			continue;
		}

		dump_node(child, path_new);
	}

	free(path_new);
}

#ifdef MULTI_THREADS
root_t *create_tree(const char c)
{
	root_t *root;

	if (!(root = (root_t *)malloc(sizeof(root_t)))) {
		printf("Failed to allocate a root node\n");
		return NULL;
	}

	memset(root, 0, sizeof(root_t));

	if (!(root->n = create_node(c))) {
		printf("Failed to allocate a tree node for root\n");
		free(root);
		return NULL;
	}

	pthread_mutex_init(&root->mutex, NULL);
	return root;
}

void destroy_tree(root_t *root)
{
	if (root->n) {
		destroy_node(root->n);
	}

	pthread_mutex_destroy(&root->mutex);

	free(root);
}

errcode_t setup_tree(root_t **roots, const char *word)
{
	root_t *root;
	node_t *p;
	int len, i, idx;
	char c;

	len = strlen(word);

	assert(roots && word && len > 0);

	if ((c = to_lowercase(word[0])) < 0) {
		return 0;
	}

	idx = c - 'a';
	root = roots[idx];

	for (i = 1, p = root->n; i < len; i++) {
		/* Illegal word, skip it, resulting in leaf node's cnt == 0 */
		if ((c = to_lowercase(word[i])) < 0) {
			return 0;
		}

		idx = c - 'a';

		if (p->children[idx] != NULL) {
			p = p->children[idx];
			continue;
		}

		pthread_mutex_lock(&root->mutex);
		if (!p->children[idx]) {
			if (!(p->children[idx] = create_node(c))) {
				pthread_mutex_unlock(&root->mutex);
				return ERR_NO_MEM;
			}
		}
		pthread_mutex_unlock(&root->mutex);

		p = p->children[idx];
	}

	/* update counter on the leaf node */
	pthread_mutex_lock(&root->mutex);
	p->cnt++;
	pthread_mutex_unlock(&root->mutex);

	return 0;
}

void dump_tree(root_t *root)
{
	assert(root);

	dump_node(root->n, "");
}
#endif
