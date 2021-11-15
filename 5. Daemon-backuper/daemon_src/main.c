#include "daemon.h"
#include "args_handling.h"

#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <err.h>
#include <errno.h>

extern enum copy_type COPY_TYPE;

#define MAX_PATH_SIZE 0x200

int main(int argc, char** argv)
{
    char src_path[MAX_PATH_SIZE] = {0};
    char dst_path[MAX_PATH_SIZE] = {0};

    int args_error = check_arguments(argc, argv);
    if (args_error < 0)
        errx(EX_USAGE, "error: %s", get_error_msg(args_error));

    printf("copy type = %d\n", COPY_TYPE);

    errno = 0;
    char *src_realpath_err = realpath(argv[1], src_path);
    if (src_realpath_err == NULL)
    {   
        int error = errno;
        fprintf(stderr, "source directory error in ");
        errno = error;
        perror("realpath()");

        exit(EXIT_FAILURE);
    }

    errno = 0;
    char *dst_realpath_err = realpath(argv[2], dst_path);
    if (dst_realpath_err == NULL)
    {   
        int error = errno;
        fprintf(stderr, "destination directory error in ");
        errno = error;
        perror("realpath()");
        
        exit(EXIT_FAILURE);
    }

    printf("Source      path: %s\n", src_path);
    printf("Destination path: %s\n", dst_path);

    errno = 0;
    int src_valid = check_source_dir(src_path);
    if (src_valid == -1)
    {
        perror("source directory error");
        exit(EXIT_FAILURE);
    }

    int dst_valid = check_dest_dir(src_path, dst_path);
    if (dst_valid == -1)
    {
        fprintf(stderr, "destination directory error: Coincides with the source directory\n");
        exit(EXIT_FAILURE);
    }
    
    // init_dest_dir(dst_path);

    // printf("Starting program...\n");

    // init_daemon(src_path, dst_path, lnk_type);

    // run_backup(src_path, dst_path);

    return 0;
}
