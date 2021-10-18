#include "trans_handler.h"

int main(int argc, char *argv[])
{
    // CHECKING FOR INPUT PARAMS' ERRORS
    int input_state = error_input_vh(argc, argv);
    ERR_CHECK(input_state != 0, input_state);

    char *endptr = NULL;
    pid_t receiver_pid = strtol(argv[2], &endptr, 0);
    ERR_CHECK(receiver_pid == -1, errno);

    transmit(argv[1], receiver_pid);

    return 0;
}