#include "rec_err.h"

int error_input_vh(int argc, char *argv[])
{
    assert(argv);

    if (argc < 2)
        return ARGS_UNDERFLOW;
    else if (argc > 2)
        return ARGS_OVERFLOW;

    return 0;
}

void error_msg_vh(int errnum)
{
    switch(errnum)
    {
        case ARGS_OVERFLOW:                                     
            fprintf(stderr, "ERROR: Too many arguments (2 argument required)\n");                     
            exit(ARGS_OVERFLOW);
        case ARGS_UNDERFLOW:                                     
            fprintf(stderr, "ERROR: Too few arguments (2 argument required)\n");                     
            exit(ARGS_UNDERFLOW);
        case READ_SIZE:                                     
            fprintf(stderr, "ERROR: Function \"read (int __fd, void *__buf, size_t __nbytes)\" cannot read all symbols in input file\n");                     
            exit(READ_SIZE);
        case BAD_ALLOC:                                     
            fprintf(stderr, "ERROR: Allocation memory error\n");                     
            exit(BAD_ALLOC);
        default:                                                     
            fprintf(stderr, "ERROR: UNKNOWN\n");                                       
            exit(EXIT_FAILURE);                                                    
    }
}
