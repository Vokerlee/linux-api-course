#ifndef ERRORS_VH_H
#define ERRORS_VH_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

/*!-----------------------------------------------------------------------------
Checks console arguments for being valid

    @version 1.3

    @authors Vokerlee

    @param[in] argc Default argc from main function
    @param[in] argv Default argv from main function

    @brief Checks for (argc == 3) and (argv != nullptr)
*///----------------------------------------------------------------------------

int error_input_vh(int argc, char *argv[]);

/*!-----------------------------------------------------------------------------
Checks console arguments for being valid

    @version 1.2

    @authors Vokerlee

    @param[in] dump_file The stream to print all errors' dumps
    @param[in] error     Error code

    @brief Prints error dump
*///----------------------------------------------------------------------------

void error_dump_vh(FILE *dump_file, int error);

#ifndef ASSERT_OK
#define ASSERT_OK(error)                                                     \
    {                                                                        \
        fprintf(stderr, "ERROR AT LINE %d, FUNCTION %s\n"                    \
                        "DUMP IS CALLED\n", __LINE__, __FUNCTION__);         \
        error_dump_vh(stderr, error);                                        \
    }
#endif

/*!-----------------------------------------------------------------------------
Enum for error codes

    @version 1.4

    @authors Vokerlee

    @brief All errors that can be occured by Vokerlee program implementation
*///----------------------------------------------------------------------------

enum error_vh
{
    // ARGUMENTS HANDLER
    NO_INPUT_ARGS  = -1,
    ARGV_NULLPTR   = -2,

    // ARGS ERRORS
    ARGS_UNDERFLOW = -3,
    ARGS_OVERFLOW  = -4,

    // SYSTEM CALLS
    READ_SIZE      = -5,
    WRITE_SIZE     = -7,
    BAD_ALLOC      = -6,

    // OTHER FUNCTIONS ERRORs
    MEMSET         = -9
};

#endif // ERRORS_VH_H