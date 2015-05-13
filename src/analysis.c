/*
 * A handy tool to analyse the occurence of each word in the given text file
 *		- when RAM is limited
 *
 * qingtao.cao.au@gmail.com
 */

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "node.h"

#define CHUNK_SIZE_MIN		64		/* MUST be longer than the longest word */
#define CHUNK_SIZE_MAX		4096
#define CHUNK_SIZE_DEF		(CHUNK_SIZE_MAX)

int main(int argc, char *argv[])
{
	node_t *root = NULL;
	struct stat statbuf;
	const char *file;
	int fd, ret;
	int frag_len, size, already_read;
	int chunk_size = CHUNK_SIZE_DEF;
	char fragment[WORD_LEN_MAX + 1], *buf = NULL, *buf_start, *p;
	char *token, *saveptr;

	if (argc < 2 || argc > 3) {
		printf("Usage: %s <text file path> <chunk size>\n", argv[0]);
		return ERR_BAD_PARAM;
	}

	file = argv[1];

	if (argc == 3) {
		chunk_size = atoi(argv[2]);
		if (chunk_size < CHUNK_SIZE_MIN) {
			chunk_size = CHUNK_SIZE_MIN;
		} else if (chunk_size > CHUNK_SIZE_MAX) {
			chunk_size = CHUNK_SIZE_MAX;
		}
	}

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

	frag_len = already_read = 0;
	while (already_read <= statbuf.st_size) {
		buf_start = buf;
		size = chunk_size;

		if (frag_len > 0) {
			memcpy(buf, fragment, frag_len);
			buf[frag_len] = '\0';
			buf_start += frag_len;
			size -= frag_len;
			frag_len = 0;
		}

		if ((ret = read(fd, buf_start, size)) < 0) {
			ret = ERR_IO;
			break;
		} else if (ret == 0) {
			if (frag_len == 0) {	/* no leftover from previous chunk */
				break;
			}
		} else {
			already_read += ret;
			buf_start[ret] = '\0';

			/*
			 * If more data available, the last word in current chunk
			 * may be cut-acrossed, save and remove it.
			 *
			 * However, we can't tell if the first character in the next
			 * chunk is a delimiter or not, if yes, then the current chunk
			 * contains a complete word and should be handled properly.
			 *
			 * Fortunately, this won't happen if the chunk size is larger
			 * than the length of the longest possible word.
			 */
			if (ret == size && already_read < statbuf.st_size) {
				for (p = buf_start + ret - 1, frag_len = 0;
					 p >= buf && is_delimiter(*p) == 0;
					 p--, frag_len++);

				if (p < buf) {
					ret = ERR_BAD_PARAM;
					printf("The specified chunk size is too small to "
						   "accommodate a long word (partial): %s\n", buf);
					goto failed;
				} else if (frag_len > 0) {

					memcpy(fragment, p + 1, frag_len);

					/* Truncate the fragment of the last word */
					*(p + 1) = '\0';
				}
			}
		}

		/* Build up our tree from each token */
		if ((token = strtok_r(buf, DELIMITER, &saveptr)) != NULL) {
			do {
				if ((ret = setup_tree(root, token)) > 0) {
					goto mem_failed;
				}
			} while ((token = strtok_r(NULL, DELIMITER, &saveptr)) != NULL);
		}

	}

	dump_tree(root, "");

	ret = ERR_SUCCESS;

	/* Fall through */

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
