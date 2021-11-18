#include <sysexits.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>


const int INDENT = 40;
const char *PID_FILE_NAME = "/var/run/backuperd.pid";

#define MAX_PID_STRING_LEN 100


int main(int argc, char *argv[])
{
    fprintf(stderr, "\033[0;31m"); // red color

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
                errx(EX_USAGE, "error: too few arguments for --period option");

            union sigval period_val;
            period_val.sival_int = atoi(argv[2]);

            if (period_val.sival_int > 0)
                sigqueue(backuperd_pid, SIGUSR1, period_val);
        }
        else
        {
            fprintf(stderr, "unknown option; for more details see --help\n");
        }
    }

    exit(EXIT_SUCCESS);
}
