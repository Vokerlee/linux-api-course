#include "trans_handler.h"

extern pid_t RECEIVER_PID;

int main(int argc, char *argv[])
{
    // CHECKING FOR INPUT PARAMS' ERRORS
    int input_state = error_input_vh(argc, argv);
    ERR_CHECK(input_state != 0, input_state);

    char *endptr = NULL;
    pid_t receiver_pid = strtol(argv[2], &endptr, 0);
    ERR_CHECK(receiver_pid == -1, errno);

    RECEIVER_PID = receiver_pid;

    struct sigaction old_handler;
    struct sigaction busy_handler;
    struct sigaction term_handler;

    busy_handler.sa_sigaction = busy_receiver_handler;
    busy_handler.sa_flags = SA_SIGINFO;
    term_handler.sa_sigaction = term_receiver_handler;
    term_handler.sa_flags = SA_SIGINFO;

    errno = 0;
    int error_state = sigaction(SIGRT_BUSY, &busy_handler, &old_handler);
    ERR_CHECK(error_state == -1, errno);

    errno = 0;
    error_state = sigaction(SIGRT_TERM, &term_handler, &old_handler);
    ERR_CHECK(error_state == -1, errno);

    transmit(argv[1], receiver_pid);

    return 0;
}
