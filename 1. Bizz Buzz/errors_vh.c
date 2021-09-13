#include "errors_vh.h"

#include <string.h>

#define ERR_CASE(err_num, str_err)                                         \
    case err_num:                                                          \
        fprintf(dump_file, str_err);                                       \
        exit(err_num);                                                    

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

void error_dump_vh(FILE *dump_file, int error)
{
    if (dump_file)
    {
        switch(error)
        {
            ERR_CASE(NO_INPUT_ARGS,  "Error: 2 arguments required (instead of no arguments)\n")
            ERR_CASE(ARGS_UNDERFLOW, "Error: Too few arguments (2 arguments required)\n")
            ERR_CASE(ARGS_OVERFLOW,  "Error: Too many arguments (2 arguments required)\n")
            ERR_CASE(READ_SIZE,      "Error: Function \"read (int __fd, void *__buf, size_t __nbytes)\" cannot read all symbols in input file\n")
            ERR_CASE(BAD_ALLOC,      "Error: Allocation memory error\n")
            ERR_CASE(WRITE_SIZE,     "Error: Function \"ssize_t write (int __fd, const void *__buf, size_t __n)\" cannot write all necessary symbols\n")
            ERR_CASE(MEMSET,         "Error: memset function error\n")

            default:                                                     
                fprintf(dump_file, "Error: %s\n", strerror(errno));                                       
                exit(error);                                                    
        }
    }
    else
        perror("Error: some error during execution was occured, but dump file cannot be created or out of access\n");           
        
}