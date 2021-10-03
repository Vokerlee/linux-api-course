#ifndef CONV_ERR_H_
#define CONV_ERR_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../include/sys_err.h"

#ifndef ERR_CHECK
#define ERR_CHECK(condition, errnum)                                                       \
    if (condition)                                                                         \
    {                                                                                      \
        fprintf(stderr, "ERROR AT LINE %d, FUNCTION %s:\n\t", __LINE__, __FUNCTION__);     \
        if (errnum < 0)                                                                    \
            error_msg_vh(errnum);                                                          \
        else                                                                               \
            error_msg("");                                                                 \
                                                                                           \
        exit(EXIT_FAILURE);                                                                \
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

/*!-----------------------------------------------------------------------------
Checks console arguments for being valid (Vokerlee handler)

    @version 1.3

    @authors Vokerlee

    @param[in] argc Default argc from main function
    @param[in] argv Default argv from main function

    @brief Checks for (argc == 2) and (argv != nullptr)
*///----------------------------------------------------------------------------

int error_input_vh(int argc, char *argv[]);

/*!-----------------------------------------------------------------------------
Prints error message of special case of the program (Vokerlee handler)

    @version 1.3

    @authors Vokerlee

    @param[in] errnum Error code from error_vh enumeration (always < 0)

    @brief Prints error message in stderr (erros for Vokerlee functions)
*///----------------------------------------------------------------------------

void error_msg_vh(int errnum);

#endif //  CONV_ERR_H_