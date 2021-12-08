#include <sysexits.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>

const int INDENT = 40;
const char *PID_FILE_NAME = "/var/run/backuperd.pid";
const char *FIFO_NAME     = "/tmp/backuperd_fifo";

#define MAX_PID_STRING_LEN 100
#define MAX_PATH_SIZE 0x200

int check_dest_dir(char* src, char* dst);
int check_source_dir(char* src);


int main(int argc, char *argv[])
{
    fprintf(stderr, "\033[0;31m"); // red color

    char src_path[MAX_PATH_SIZE] = {0};
    char dst_path[MAX_PATH_SIZE] = {0};

    if (argc < 2)
         errx(EX_USAGE, "error: too few arguments");

    // HANDLE INPUT OPTIONS

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
    {
        int indent = 0;

        printf("\033[0;34m");
        printf("Usage: backuperd_ctl [OPTION]\n"
               "Options:\n");

        printf("\t-h, --help%n", &indent);
        printf("%*sPrint help information\n", INDENT - indent, " ");

        printf("\t-l, --log%n", &indent);
        printf("%*sPrint log information of backuperd work\n", INDENT - indent, " ");

        printf("\t-s, --stop%n", &indent);
        printf("%*sStop backuperd work\n", INDENT - indent, " ");

        printf("\t-c, --continues%n", &indent);
        printf("%*sContinues backuperd work\n", INDENT - indent, " ");

        printf("\t-t, --terminate%n", &indent);
        printf("%*sTerminate backuperd work\n", INDENT - indent, " ");

        printf("\t-p, --period [value in seconds]%n", &indent);
        printf("%*sSet new period of backuperd\n", INDENT - indent, " ");

        printf("\t-d, --dirs [src_name] [dst_name]%n", &indent);
        printf("%*sSet new source and destination directories for backup\n", INDENT - indent, " ");

        printf("\t-m, --mode [mode_name]%n", &indent);
        printf("%*sSet new backup mode. Mode name can be \"recursive\" or \"inotify\"\n", INDENT - indent, " ");
    }
    else if (strcmp(argv[1], "--log") == 0 || strcmp(argv[1], "-l") == 0)
    {
        fprintf(stderr, "\033[0;34m");
        system("journalctl -r --since=today --lines=300 --grep=backuper | grep \"backuperd\\[\"");
        fprintf(stderr, "\033[0;31m");
    }
    else
    {
        // READ PID OF DAEMON

        int fd = open(PID_FILE_NAME, O_RDONLY);
        if (errno == EACCES)
        {
            fprintf(stderr, "backuperd is not launched yet\n");
            exit(EXIT_FAILURE);
        }
        else if (fd == -1)
        {
            perror("PID file of backuperd cannot be opened");
            exit(EXIT_FAILURE);
        }

        char buffer[MAX_PID_STRING_LEN] = {0};

        int read_state = read(fd, buffer, MAX_PID_STRING_LEN - 1);
        if (read_state == -1)
        {
            perror("cannot read PID file of backuperd");
            exit(EXIT_FAILURE);
        }

        close(fd);

        pid_t backuperd_pid = atoi(buffer);

        int state = kill(backuperd_pid, 0); // exist daemon now or not
        if (state == -1)
        {
            fprintf(stderr, "backuperd is not launched yet\n");
            exit(EXIT_FAILURE);
        }

        if (strcmp(argv[1], "--stop") == 0 || strcmp(argv[1], "-s") == 0)
            kill(backuperd_pid, SIGSTOP);
        else if (strcmp(argv[1], "--continous") == 0 || strcmp(argv[1], "-c") == 0)
            kill(backuperd_pid, SIGCONT);
        else if (strcmp(argv[1], "--terminate") == 0 || strcmp(argv[1], "-t") == 0)
            kill(backuperd_pid, SIGTERM);
        else if (strcmp(argv[1], "--period") == 0 || strcmp(argv[1], "-p") == 0)
        {
            if (argc != 3)
                errx(EX_USAGE, "error: too few arguments for --period option, see --help");

            union sigval period_val;
            period_val.sival_int = atoi(argv[2]);

            if (period_val.sival_int > 0)
                sigqueue(backuperd_pid, SIGUSR1, period_val);
        }
        else if (strcmp(argv[1], "--mode") == 0 || strcmp(argv[1], "-m") == 0)
        {
            if (argc != 3)
                errx(EX_USAGE, "error: invalid amount of arguments for --dirs option, see --help");

            union sigval period_val;
            if (strcmp("inotify", argv[2]) == 0)
                period_val.sival_int = -1;
            else if (strcmp("recursive", argv[2]) == 0)
                period_val.sival_int = -2;
            else
                errx(EX_USAGE, "error: invalid mode, see --help");

            sigqueue(backuperd_pid, SIGUSR1, period_val);
        }
        else if (strcmp(argv[1], "--dirs") == 0 || strcmp(argv[1], "-d") == 0)
        {
            if (argc != 4)
                errx(EX_USAGE, "error: invalid amount of arguments for --dirs option, see --help");

            if (check_dest_dir(argv[2], argv[3]) == -1)
                errx(EX_USAGE, "error: source cannot coincide with destination");

            if (check_source_dir(argv[2]) == -1)
            {
                perror("check_src_dir()");
                exit(EXIT_FAILURE);
            }

            errno = 0;
            char *src_realpath_err = realpath(argv[2], src_path);
            if (src_realpath_err == NULL)
            {   
                int error = errno;
                fprintf(stderr, "source directory error in ");
                errno = error;
                perror("realpath()");

                exit(EXIT_FAILURE);
            }

            errno = 0;
            char *dst_realpath_err = realpath(argv[3], dst_path);
            if (dst_realpath_err == NULL)
            {   
                int error = errno;
                fprintf(stderr, "destination directory error in ");
                errno = error;
                perror("realpath()");
                
                exit(EXIT_FAILURE);
            }

            int fifo_fd = 0;
            unlink(FIFO_NAME);

            if((mkfifo(FIFO_NAME, O_RDWR)) == -1)
            {
                perror("mkfifo()");
                exit(EXIT_FAILURE);
            }

            kill(backuperd_pid, SIGUSR2);

            if ((fifo_fd = open(FIFO_NAME, O_WRONLY, 0666)) == -1)
            {
                perror("open()");
                exit(EXIT_FAILURE);
            }

            int wr1_err = write(fifo_fd, src_path, sizeof(src_path));
            int wr2_err = write(fifo_fd, dst_path, sizeof(dst_path));

            if (wr1_err == -1 || wr2_err == -1)
            {
                perror("write()");
                exit(EXIT_FAILURE);
            }

            close(fifo_fd);
        }
        else
        {
            fprintf(stderr, "unknown option; for more details see --help\n");
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
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
