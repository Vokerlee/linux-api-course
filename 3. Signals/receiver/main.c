#include "rec_handler.h"

int main(int argc, char *argv[])
{
// CHECKING FOR INPUT PARAMS' ERRORS

    int error_state = 0;

    errno = 0;
    int input_state = error_input_vh(argc, argv);
    ERR_CHECK(input_state != 0, input_state);

// OPEN FILE

    errno = 0;
    int fd = open(argv[1], O_WRONLY | O_CREAT, 0666);
    ERR_CHECK(fd == -1, errno);

// PRINT PID FOR CONVENIENCE

    printf("My pid = %d\n", getpid());

// SIGNALS SETTINGS

    sigset_t waitset;

    sigemptyset(&waitset);
    sigaddset(&waitset, SIGUSR1);
    sigaddset(&waitset, SIGUSR2);
    sigaddset(&waitset, SIGTERM);
    sigaddset(&waitset, SIGINT);

    error_state = sigprocmask(SIG_BLOCK, &waitset, NULL);
    ERR_CHECK(error_state == -1, errno);

// SIGNALS HANDLING

    pid_t transmitter_pid = 0;

    size_t data_size = get_data_size(waitset, &transmitter_pid);
    printf("Transmitter pid = %d\n", transmitter_pid);
    printf("Request size = %zu\n", data_size);
    
    char *data = (char *) calloc(data_size, sizeof(char));
    ERR_CHECK(data == NULL, BAD_ALLOC);

    size_t get_data_size = get_data(data, data_size, waitset, transmitter_pid);
    printf("Read size = %zu\n", get_data_size);

    errno = 0;
    error_state = write(fd, data, get_data_size);
    ERR_CHECK(error_state == -1, errno);

    free(data);
    close(fd);

    return 0;
}