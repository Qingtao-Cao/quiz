#include <sys/syscall.h>
#include <unistd.h>

const int WORD_LEN_MAX = 64;

const char *DELIMITER = " \t\n\"\',.:!?=";
const int DELIMITER_NUM = 11;

pid_t get_tid(void)
{
	return syscall(SYS_gettid);
}

/*
 * Return 1 if the givn charater is one of delimiter chars
 * 0 otherwise
 */
int is_delimiter(const char c)
{
	int i;

	for (i = 0; i < DELIMITER_NUM; i++) {
		if (c == DELIMITER[i]) {
			return 1;
		}
	}

	return 0;
}

/*
 * Convert a character into lower case
 * return -1 if it is not an alphabet
 */
int to_lowercase(const char c)
{
	int lc = c;

	if (lc >= 'A' && lc <= 'Z') {
		lc += 'a' - 'A';
	}

	if (lc < 'a' || c > 'z') {
		return -1;
	}

	return lc;
}
