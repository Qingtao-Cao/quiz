
#ifndef _LIB_H
#define _LIB_H

#include <sys/syscall.h>
#include <unistd.h>

#define AVAILABLE_CHARS	26

extern const int WORD_LEN_MAX;
extern const char *DELIMITER;
extern const int DELIMITER_NUM;

typedef enum {
	ERR_SUCCESS = 0,
	ERR_NO_MEM,
	ERR_BAD_FILE,
	ERR_IO,
	ERR_BAD_PARAM
} errcode_t;

int is_delimiter(const char c);
pid_t get_tid(void);

#endif	/* _LIB_H */
