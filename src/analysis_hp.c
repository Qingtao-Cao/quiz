/*
 * A handy tool to analyse the occurence of each word in the given text file
 *		- when performance matters
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

#define THREADS_NUM_DEF		10

typedef struct thread {
	/* The current thread */
	pthread_t id;

	/* The root of the tree built up by this thread */
	node_t *root;

	/* Address of the first and the last byte in the overall data buffer */
	char *start, *end;

	/* Buffer size, for debug purpose */
	int chunk_size;
} thread_t;

typedef struct analysis {
	/* The number of working threads */
	int threads_num;

	/* The fleet of working threads */
	thread_t *threads;

	/* The huge buffer containing all data to be analysed */
	char *data;
} analysis_t;

static void destroy_analysis(analysis_t *ana)
{
	int i;

	if (!ana) {
		return;
	}

	if (ana->data) {
		free(ana->data);
	}

	if (ana->threads_num > 0) {
		for (i = 0; i < ana->threads_num; i++) {
			if (ana->threads[i].root) {
				destroy_tree(ana->threads[i].root);
			}
		}

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
		!(ana->threads = (thread_t *)malloc(sizeof(thread_t) * threads_num))) {
		goto failed;
	}

	memset(ana->threads, 0, sizeof(thread_t) * threads_num);
	ana->threads_num = threads_num;

	for (i = 0; i < threads_num; i++) {
		if (!(ana->threads[i].root = create_node(0))) {
			goto failed;
		}
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
			if ((ret = setup_tree(current->root, token)) > 0) {
				break;
			}
		} while ((token = strtok_r(NULL, DELIMITER, &saveptr)) != NULL);
	}

	dump_tree(current->root, "");

	return NULL;
}

int main(int argc, char *argv[])
{
	analysis_t *ana;
	thread_t *current;
	struct stat statbuf;
	const char *file;
	char *start;
	int fd, ret, i, threads_num, len;

	if (argc < 2 || argc > 3) {
		printf("Usage: %s <text file path> <num of threads>\n", argv[0]);
		return ERR_BAD_PARAM;
	}

	file = argv[1];

	if (argc == 3) {
		threads_num = atoi(argv[2]);
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

	printf("num of threads : %d\n", threads_num);

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
	if ((ret = read(fd, ana->data, statbuf.st_size)) < 0) {
		ret = ERR_IO;
		goto read_failed;
	}

	/* Append terminator to what has been read */
	ana->data[ret] = '\0';

	start = ana->data;
	len = statbuf.st_size / threads_num;

	for (i = 0; i < threads_num; i++) {
		current = &ana->threads[i];
		current->start = start;
		current->end = start + len - 1;

		if (current->end >= ana->data + statbuf.st_size) {
			current->end = ana->data + statbuf.st_size - 1;
		}

		/* Move forward the end pointer if it cuts-across a word */
		while (is_delimiter(*current->end) == 0) {
			current->end--;
		}

		/* Convert a delimier to a NULL byte as boundary of adjanct chunks */
		*current->end = '\0';

		current->chunk_size = current->end - current->start;
		printf("chunk %d size : %d\n", i, current->chunk_size);

		start = current->end + 1;
	}

	for (i = 0; i < threads_num; i++) {
		if (pthread_create(&ana->threads[i].id, NULL,
						   payload, (void *)&ana->threads[i]) != 0) {
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

	ret = ERR_SUCCESS;

	/* Fall through */

read_failed:
	close(fd);

failed:
	destroy_analysis(ana);

	return ret;
}
