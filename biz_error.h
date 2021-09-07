#ifndef BIZ_ERRORS_H
#define BIZ_ERRORS_H

#include <stdio.h>

int error_input(int argc, char *argv[]);

void biz_error_dump(FILE *dump_file, int biz_error);

enum biz_error
{
    NO_INPUT       = -1,
    ARGS_UNDERFLOW = -2,
    ARGS_OVERFLOW  = -3,
    READ_SIZE      = -4,
    BAD_ALLOC      = -5,
}

#endif // BIZ_ERRORS_H