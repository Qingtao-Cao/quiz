/*
 * A handy tool to analyse the occurence of each word in the given text file
 *	- Multi-threads version
 *
 * qingtao.cao.au@gmail.com
 */

#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include "node.h"

/* The more threads, the more contention on mutex */
#define THREADS_NUM_MIN		2
#define THREADS_NUM_DEF		4

struct analysis;

typedef struct thread {
	/* The current thread */
	pthread_t id;

	/* Address of the first and the last byte in the overall data buffer */
	char *start, *end;

	/* Point back to the parent data structure */
	struct analysis *parent;
} thread_t;

typedef struct analysis {
	/* The number of working threads */
	int threads_num;

	/* The fleet of working threads */
	thread_t *threads;

	/* The huge buffer containing all data to be analysed */
	char *data;

	/* The root of the overall tree built up by threads */
	node_t *root;
} analysis_t;

static void destroy_analysis(analysis_t *ana)
{
	if (!ana) {
		return;
	}

	if (ana->data) {
		free(ana->data);
	}

	if (ana->root) {
		destroy_tree(ana->root);
	}

	if (ana->threads) {
		free(ana->threads);
	}

	free(ana);
}

static analysis_t *setup_analysis(const int threads_num, const int size)
{
	analysis_t *ana;
	int i;

	if (!(ana = (analysis_t *)malloc(sizeof(analysis_t))) ||
		!(ana->data = (char *)malloc(size + 1)) ||
		!(ana->root = create_node(0)) ||
		!(ana->threads = (thread_t *)malloc(sizeof(thread_t) * threads_num))) {
		goto failed;
	}

	/*
	 * For sake of performance, build up the first level subtree
	 * to remove the bottle neck on the root node
	 */
	for (i = 0; i < AVAILABLE_CHARS; i++) {
		if (!(ana->root->children[i] = create_node('a'+i))) {
			goto failed;
		}
	}

	memset(ana->threads, 0, sizeof(thread_t) * threads_num);
	ana->threads_num = threads_num;

	for (i = 0; i < threads_num; i++) {
		ana->threads[i].parent = ana;
	}

	return ana;

failed:
	destroy_analysis(ana);
	return NULL;
}

static void *payload(void *arg)
{
	thread_t *current = (thread_t *)arg;
	char *token, *saveptr;
	int ret;

	/* Build up our tree from each token */
	if ((token = strtok_r(current->start, DELIMITER, &saveptr)) != NULL) {
		do {
			if ((ret = setup_tree(current->parent->root, token)) > 0) {
				break;
			}
		} while ((token = strtok_r(NULL, DELIMITER, &saveptr)) != NULL);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	analysis_t *ana;
	thread_t *current;
	struct stat statbuf;
	const char *file;
	char *start;
	int fd, ret, i, threads_num = 0, len;

	if (argc < 2 || argc > 3) {
		printf("Usage: %s <text file path> <num of threads>\n", argv[0]);
		return ERR_BAD_PARAM;
	}

	file = argv[1];

	if (argc == 3) {
		threads_num = atoi(argv[2]);
		if (threads_num <= 0) {
			threads_num = THREADS_NUM_MIN;
		}
	} else {
		threads_num = THREADS_NUM_DEF;
	}

	if (lstat(file, &statbuf) < 0 ||
		S_ISREG(statbuf.st_mode) == 0 ||
		statbuf.st_size == 0) {
		printf("Illegal text file : %s\n", file);
		return ERR_BAD_FILE;
	}

	/* Adjust the number of threads if needed */
	if (statbuf.st_size <= WORD_LEN_MAX) {
		threads_num = 1;
	} else if (statbuf.st_size < WORD_LEN_MAX * threads_num) {
		threads_num = statbuf.st_size / WORD_LEN_MAX;
	}

	if (!(ana = setup_analysis(threads_num, statbuf.st_size))) {
		printf("Failed to allocate analysis_t\n");
		ret = ERR_NO_MEM;
		goto failed;
	}

	if ((fd = open(file, O_RDONLY)) < 0) {
		printf("Failed to open file : %s\n", file);
		ret = ERR_IO;
		goto failed;
	}

	/* Read the whole content of the input file for sake of performance */
	if ((ret = read(fd, ana->data, statbuf.st_size)) < statbuf.st_size) {
		ret = ERR_IO;
		goto read_failed;
	}

	ana->data[ret] = '\0';

	start = ana->data;
	len = statbuf.st_size / threads_num;

	/*
	 * NOTE: the last thread/chunk will be treated differently
	 */
	for (i = 0; i < threads_num - 1; i++) {
		current = &ana->threads[i];
		current->start = start;
		current->end = start + len;

		/* Move along the end pointer to the closet delimiter */
		while (is_delimiter(*current->end) == 0) {
			current->end++;
		}

		/* Convert a delimiter to a NULL byte as boundary of chunks */
		*current->end = '\0';
		start = current->end + 1;
	}

	ana->threads[threads_num - 1].start = start;

	for (i = 0; i < threads_num; i++) {
		current = &ana->threads[i];
		if (pthread_create(&current->id, NULL, payload, (void *)current) != 0) {
			printf("Failed to start thread %d\n", i);
			ret = ERR_NO_MEM;
			break;
		}
	}

	for (i = 0; i < threads_num; i++) {
		if (pthread_join(ana->threads[i].id, NULL) != 0) {
			printf("Failed to join thread %d and it could be left zombie", i);
		}
	}

	dump_tree(ana->root, "");

	ret = ERR_SUCCESS;

	/* Fall through */

read_failed:
	close(fd);

failed:
	destroy_analysis(ana);

	return ret;
}
