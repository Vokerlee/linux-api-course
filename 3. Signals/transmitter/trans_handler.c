#include "trans_handler.h"

const size_t BITS_PER_BYTE = 8;

void transmit(const char *file_name, const pid_t reciever_pid)
{
    assert(file_name);

// FILE OPENING

    errno = 0;
    int fd = open(file_name, O_RDONLY);
    ERR_CHECK(fd == -1, errno);

    errno = 0;
    struct stat file_stat = {0};
    int fstat_state = fstat(fd, &file_stat);
    ERR_CHECK(fstat_state == -1, errno);

    size_t data_size = file_stat.st_size;

// BUFFER CREATING & FILE READING

    char* data = (char*) calloc(data_size, sizeof(char));
    ERR_CHECK(data == NULL, BAD_ALLOC);

    errno = 0;
    int n_read = read(fd, data, data_size);
    ERR_CHECK(n_read == -1, errno);
    ERR_CHECK(n_read != data_size, READ_SIZE);

    errno = 0;
    int close_state = close(fd);
    ERR_CHECK(close_state == -1, errno);

// TRANSMIITING

    sigset_t waitset;
    sigemptyset(&waitset);

    sigaddset(&waitset, SIGUSR1);
    sigaddset(&waitset, SIGUSR2);
    
    errno = 0;
    int error_state = sigprocmask(SIG_BLOCK, &waitset, NULL);
    ERR_CHECK(error_state == -1, errno);

    transmit_size(data_size, reciever_pid, waitset);

    transmit_data(data, data_size, reciever_pid, waitset);

    kill(reciever_pid, SIGTERM);

    free(data);
}

void transmit_size(const size_t data_size, const pid_t reciever_pid, sigset_t waitset)
{
    int error_state = 0;
    
    siginfo_t siginfo;

    for (size_t i = 0; i < sizeof(size_t); ++i)
    {
        size_t bit_mask = 0xFF << (BITS_PER_BYTE * i);
        unsigned char byte = (bit_mask & data_size) >> (BITS_PER_BYTE * i);

        sigval_t value = {0};
        value.sival_int = byte;

        error_state = sigqueue(reciever_pid, SIGUSR1, value);
        ERR_CHECK(error_state == -1, errno);

        error_state = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(error_state == -1, errno);
    }
}

void transmit_data(const char *data, const size_t data_size, const pid_t reciever_pid, sigset_t waitset)
{
    int error_state = 0;

    const int *int_data = (const int *) data;
    const size_t int_data_size = data_size / sizeof(int);
    
    siginfo_t siginfo;
    sigval_t  value;

    // All full blocks (4 bytes)
    for (size_t i = 0; i < int_data_size; ++i)
    {
        value.sival_int = int_data[i];

        error_state = sigqueue(reciever_pid, SIGUSR1, value);
        ERR_CHECK(error_state == -1, errno);

        error_state = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(error_state == -1, errno);
    }

    // Remainder (less than 4 bytes)
    for (size_t i = 0; i < data_size % sizeof(int); ++i)
    {
        value.sival_int = data[int_data_size * sizeof(int) + i];

        error_state = sigqueue(reciever_pid, SIGUSR1, value);
        ERR_CHECK(error_state == -1, errno);

        error_state = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(error_state == -1, errno);
    }
}
