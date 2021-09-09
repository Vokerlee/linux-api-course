#ifndef BIZ_ERRORS_H
#define BIZ_ERRORS_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int error_input(int argc, char *argv[]);

void biz_error_dump(FILE *dump_file, int biz_error);

#ifndef ASSERT_OK
#define ASSERT_OK(error)                                                     \
    {                                                                        \
        fprintf(stderr, "ERROR AT LINE %d, FUNCTION %s\n"                    \
                        "BIZZ-DUMP IS CALLED\n", __LINE__, __FUNCTION__);    \
        biz_error_dump(stderr, error);                                       \
    }
#endif

enum biz_error
{
    NO_INPUT       = -10,
    ARGS_UNDERFLOW = -20,
    ARGS_OVERFLOW  = -30,
    READ_SIZE      = -40,
    BAD_ALLOC      = -50,
};

#endif // BIZ_ERRORS_H