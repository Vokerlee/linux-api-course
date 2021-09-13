#include "biz_error.h"

#include <string.h>

#define ERR_CASE(bizz_err, str_err)                                         \
    case bizz_err:                                                          \
        fprintf(dump_file, str_err);                                        \
        exit(NO_INPUT);                                                     \
        break;                                                              

int error_input(int argc, char *argv[])
{
    if (argc == 1)
        return NO_INPUT;
    else if (argc == 2)
        return ARGS_UNDERFLOW;
    else if (argc > 3)
        return ARGS_OVERFLOW;

    return 0;
}

void biz_error_dump(FILE *dump_file, int biz_error)
{
    if (dump_file)
    {
        switch(biz_error)
        {
            ERR_CASE(NO_INPUT,       "bizz-buzz error: 2 arguments required (instead of no arguments)\n")
            ERR_CASE(ARGS_UNDERFLOW, "bizz-buzz error: too few arguments (2 arguments required)\n")
            ERR_CASE(ARGS_OVERFLOW,  "bizz-buzz error: too many arguments (2 arguments required)\n")
            ERR_CASE(READ_SIZE,      "bizz-buzz error: function \"read (int __fd, void *__buf, size_t __nbytes)\" cannot read all symbols in input file\n")
            ERR_CASE(BAD_ALLOC,      "bizz-buzz error: allocation memory error\n")

            default:                                                     
                fprintf(dump_file, "bizz-buzz undefined error: %s\n", strerror(errno));                                       
                exit(NO_INPUT);                                                    
                break; 
        }
    }
    else
        perror("bizz-buzz error: error is occured, but dump file cannot be created\n");
}