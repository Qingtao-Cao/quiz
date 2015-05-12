/*
 * A handy tool to analyse the occurence of each word in the given text file
 *
 * qingtao.cao.au@gmail.com
 *
 * To compile:
 *	gcc -O2 analysis.c -o analysis
 *
 *	In particular, specify "-DDEBUG" to enforce the chunk size to be a small
 *	value, specify "-g" for debug purpose.
 *
 * To check memory usage of this program:
 *	valgrind --leak-check=full --log-file=analysis.val ./analysis <input file>
 */

#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

typedef enum {
	ERR_SUCCESS = 0,
	ERR_NO_MEM,
	ERR_BAD_FILE,
	ERR_IO,
	ERR_BAD_PARAM
} errcode_t;

#define AVAILABLE_CHARS		26
#define CHUNK_SIZE_MIN		64		/* MUST be longer than the longest word */
#define CHUNK_SIZE_MAX		4096
#define CHUNK_SIZE_DEF		(CHUNK_SIZE_MAX)

/*
 * Descriptor of a node in the analysis tree
 */
typedef struct node {
	/* occurance of the word represented by this node */
	int cnt;

	/* The alphabet of this node */
	char c;

	/* Pointers to the next alphabets of potential words */
	struct node *children[AVAILABLE_CHARS];

} node_t;

static const char *DELIMITER = " \t\n\"\',.:!?";
static int DELIMITER_NUM = 10;

/*
 * Return 1 if the givn charater is one of delimiter chars
 * 0 otherwise
 */
static int is_delimiter(const char c)
{
	int i;

	for (i = 0; i < DELIMITER_NUM; i++) {
		if (c == DELIMITER[i]) {
			return 1;
		}
	}

	return 0;
}

static node_t *create_node(const char c)
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

static void destroy_tree(node_t *root)
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

static errcode_t setup_tree(node_t *root, const char *word)
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


static void dump_tree(const node_t *node, char *path)
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

int main(int argc, char *argv[])
{
	node_t *root = NULL;
	struct stat statbuf;
	const char *file;
	int fd, ret;
	int seg_len, size, i, already_read;
	int chunk_size = CHUNK_SIZE_DEF;
	char *segment = NULL, *buf = NULL, *buf_start;
	char *token, *saveptr;

	if (argc < 2 || argc > 3) {
		printf("Usage: %s <text file path> <chunk size>\n", argv[0]);
		return ERR_BAD_PARAM;
	}

	file = argv[1];

#ifdef DEBUG
	chunk_size = 10;
	printf("In debug mode, chunk size is enforced to %d\n", chunk_size);
#else
	if (argc == 3) {
		chunk_size = atoi(argv[2]);
		if (chunk_size < CHUNK_SIZE_MIN) {
			chunk_size = CHUNK_SIZE_MIN;
		} else if (chunk_size > CHUNK_SIZE_MAX) {
			chunk_size = CHUNK_SIZE_MAX;
		}
	}
#endif

	if (lstat(file, &statbuf) < 0 ||
		S_ISREG(statbuf.st_mode) == 0 ||
		statbuf.st_size == 0) {
		printf("Illegal text file : %s\n", file);
		return ERR_BAD_FILE;
	}

	if (!(root = create_node(0)) ||
		!(buf = (char *)malloc(chunk_size + 1))) {
		ret = ERR_NO_MEM;
		goto failed;
	}

	if ((fd = open(file, O_RDONLY)) < 0) {
		printf("Failed to open file : %s\n", file);
		ret = ERR_IO;
		goto failed;
	}

	seg_len = already_read = 0;
	while (already_read <= statbuf.st_size) {
		buf_start = buf;
		size = chunk_size;

		if (seg_len > 0) {
			memcpy(buf, segment, seg_len);
			buf[seg_len] = '\0';
			buf_start += seg_len;
			size -= seg_len;
			seg_len = 0;
		}

		if ((ret = read(fd, buf_start, size)) < 0) {
			ret = ERR_IO;
			break;
		} else if (ret == 0) {
			if (seg_len == 0) {	/* no leftover from previous chunk */
				break;
			}
		} else {
			already_read += ret;
			buf_start[ret] = '\0';

			/*
			 * If more data available, the last word in current chunk
			 * may be cut-acrossed, save it and remove it
			 */
			if (ret == size && already_read < statbuf.st_size) {
				for (i = size - 1, seg_len = 0;
					 i >= 0 && is_delimiter(buf_start[i]) == 0;
					 i--, seg_len++);

				if (seg_len == size) {
					ret = ERR_BAD_PARAM;
					printf("The specified chunk size is too small to "
						   "accommodate a long word (partial): %s\n", buf_start);
					goto failed;
				} else if (seg_len > 0) {
					if (!(segment = (char *)realloc(segment, seg_len))) {
						printf("Failed to allocate mem for the segment of the last word\n");
							goto mem_failed;
					}

					memcpy(segment, buf_start + i + 1, seg_len);

					/* Truncate the segment of the last word */
					buf_start[i + 1] = '\0';
				}
			}
		}

		/* Build up our tree from each token */
		if ((token = strtok_r(buf, DELIMITER, &saveptr)) != NULL) {
			do {
				if ((ret = setup_tree(root, token)) > 0) {
					goto tree_failed;
				}
			} while ((token = strtok_r(NULL, DELIMITER, &saveptr)) != NULL);
		}

	}

	dump_tree(root, "");

	/* Fall through */

tree_failed:
	if (segment) {
		free(segment);
	}

mem_failed:
	close(fd);

failed:
	if (root) {
		destroy_tree(root);
	}

	if (buf) {
		free(buf);
	}

	return ret;
}
