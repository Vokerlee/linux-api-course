#include "rec_handler.h"

sig_atomic_t TRANSMITTER_PID = 0;

extern pid_t MASTER_PID;
extern pid_t SLAVES_PID[N_PROCESSES];
extern pid_t TRANS_SLAVES_PID[N_PROCESSES];

extern struct process_data PROC_INFO[N_PROCESSES];

void atexit_action()
{
    union sigval value;

    if (getpid() == MASTER_PID)
    {
        for (int i = 0; i < N_PROCESSES; i++)
            sigqueue(SLAVES_PID[i], SIGKILL, value);
    }
    else
    {
        for (int i = 0; i < N_PROCESSES; i++)
            sigqueue(TRANS_SLAVES_PID[i], SIGKILL, value);
    }
}

int main(int argc, char *argv[])
{
// CHECKING FOR INPUT PARAMS' ERRORS

    int error_state = 0;

    errno = 0;
    int input_state = error_input_vh(argc, argv);
    ERR_CHECK(input_state != 0, input_state);

    errno = 0;
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    ERR_CHECK(fd == -1, errno);

    MASTER_PID = getpid();

// SIGNALS SETTINGS

    sigset_t waitset;
    set_signals_mask(&waitset);

    errno = 0;
    error_state = atexit(atexit_action);
    ERR_CHECK(error_state == -1, errno);

// MAIN WORK

    get_data_master(fd, waitset);

    close(fd);

    exit(0);
}
