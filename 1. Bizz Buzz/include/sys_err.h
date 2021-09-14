#ifndef SYS_ERR_H_
#define SYS_ERR_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>

typedef enum 
{ 
    FALSE = 0, 
    TRUE  = 1
} Boolean;

/*!-----------------------------------------------------------------------------
Checks console arguments for being valid

    @version 1.1

    @authors Vokerlee

    @param[in] Just like in printf(...) function: format message with optional arguments

    @brief Prints error message in stderr (system errors)
*///----------------------------------------------------------------------------

void error_msg(const char *format, ...);


#endif // SYS_ERR_H_