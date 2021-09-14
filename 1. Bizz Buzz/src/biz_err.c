#include "biz_err.h"

int error_input_vh(int argc, char *argv[])
{
    if (argv)
    {
        if (argc == 1)
            return NO_INPUT_ARGS;
        else if (argc == 2)
            return ARGS_UNDERFLOW;
        else if (argc > 3)
            return ARGS_OVERFLOW;
    }
    else
        return ARGV_NULLPTR;

    return 0;
}

void error_msg_vh(int errnum)
{
    switch(errnum)
    {
        case NO_INPUT_ARGS:                                     
            fprintf(stderr, "ERROR: 2 arguments required (instead of no arguments)\n");                     
            exit(NO_INPUT_ARGS);
        case ARGS_UNDERFLOW:                                     
            fprintf(stderr, "ERROR: Too few arguments (2 arguments required)\n");                     
            exit(ARGS_UNDERFLOW);
        case ARGS_OVERFLOW:                                     
            fprintf(stderr, "ERROR: Too many arguments (2 arguments required)\n");                     
            exit(ARGS_OVERFLOW);
        case READ_SIZE:                                     
            fprintf(stderr, "ERROR: Function \"read (int __fd, void *__buf, size_t __nbytes)\" cannot read all symbols in input file\n");                     
            exit(READ_SIZE);
        case BAD_ALLOC:                                     
            fprintf(stderr, "ERROR: Allocation memory error\n");                     
            exit(BAD_ALLOC);
        case MEMSET:                                     
            fprintf(stderr, "ERROR: memset() function error\n");                     
            exit(MEMSET);
        default:                                                     
            fprintf(stderr, "ERROR: UNKNOWN\n");                                       
            exit(0);                                                    
    }
}
