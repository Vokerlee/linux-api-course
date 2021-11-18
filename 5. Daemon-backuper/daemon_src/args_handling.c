#include "args_handling.h"
#include "daemon.h"

#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>

extern enum copy_type COPY_TYPE;

const char *get_error_msg(int errnum)
{
    switch(errnum)
    {
        case ARGS_OVERFLOW:                                     
            return "too many arguments (maximum 3 possible)";
        case ARGS_UNDERFLOW:                                     
            return "too few arguments (at least 2 required)";  
        case UNKNOWN_ARG:
            return "unknown option of the program";                                           
    }

    return NULL;
}

int check_arguments(int argc, char** argv)
{
    assert(argv);

    if (argc < 3)
        return ARGS_UNDERFLOW;
    else if (argc > 4)
        return ARGS_OVERFLOW;

    const char shallow_copy_name[] = "--shallow";
    const char deep_copy_name[]    = "--deep";

    int shallow_copy = 0;
    int deep_copy    = 0;

    if (argc == 3)
        COPY_TYPE = SHALLOW_COPY; // default type of copying
    else if (argc == 4)
    {
        char *copy_type_name = argv[3];
        shallow_copy = strcmp(copy_type_name, shallow_copy_name);
        deep_copy    = strcmp(copy_type_name, deep_copy_name);

        if (shallow_copy != 0 && deep_copy != 0)
            return UNKNOWN_ARG;

        if (shallow_copy == 0)
            COPY_TYPE = SHALLOW_COPY;
        else
            COPY_TYPE = DEEP_COPY; 
    }

    return 0;
}

int check_dest_dir(char* src, char* dst) // destination cannot be source
{
    assert(src);
    assert(dst);

    int src_len = strlen(src);
    if (strncmp(src, dst, src_len) == 0)
        return -1;

    return 0;
}

int check_source_dir(char* src)
{
    assert(src);
    
    DIR *dir = opendir(src);
    if (dir == NULL)
        return -1;
    else
        closedir(dir);

    return 0;
}
