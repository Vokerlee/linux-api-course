#ifndef ARGS_HANDLING_H_
#define ARGS_HANDLING_H_

#include <sysexits.h>
#include <err.h>
#include <stdlib.h>
#include <stdio.h>

enum args_error
{
    ARGS_UNDERFLOW = -1,
    ARGS_OVERFLOW  = -2,

    UNKNOWN_ARG    = -3,    

};

const char *get_error_msg(int errnum);

int check_arguments(int argc, char** argv);

int check_dest_dir(char* src, char* dst);

int check_source_dir(char* src);

#endif // !ARGS_HANDLING_H_

