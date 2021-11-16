#include "args_handling.h"
#include "daemon.h"
#include "backuper.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sysexits.h>
#include <err.h>
#include <errno.h>
#include <syslog.h>

extern enum copy_type COPY_TYPE;
extern const char *DAEMON_NAME;

const char *PID_FILE_NAME = "/var/run/backuperd.pid";

#define MAX_PATH_SIZE 0x200

int main(int argc, char** argv)
{
    // GETTING FILES' PATHS

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

    // CHECKING PATHS FOR ERRORS

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

    // DAEMON STUFF

    printf("Launching daemon-backuper...\n");

    int is_daemon = become_daemon(0); // become daemon
    if (is_daemon == -1)
    {
        fprintf(stderr, "becoming daemon error\n");
        exit(EXIT_FAILURE);
    }

    openlog(DAEMON_NAME, LOG_PID, LOG_USER | LOG_LOCAL0); // open logs

    int pid_file_fd = create_unique_pid_file(argv[0], PID_FILE_NAME, 0); // check if there is already existing daemon
    if (pid_file_fd == -1)
        exit(EXIT_FAILURE);

    syslog(LOG_INFO, "Unique PID file \"%s\" created", PID_FILE_NAME);

    sigset_t waitset;
    int signals_error = set_signals(&waitset); // set signals handlers for communication with user
    if (signals_error == -1)
        exit(EXIT_FAILURE);

    launch_backuper(src_path, dst_path, waitset);

    syslog(LOG_INFO, "Successful termination");

    close(pid_file_fd);
    closelog();

    return 0;
}
