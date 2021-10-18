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
    sigval_t  value;

    for (size_t i = 0; i < sizeof(size_t); ++i)
    {
        size_t bit_mask = 0xFF << (BITS_PER_BYTE * i);
        value.sival_int = (bit_mask & data_size) >> (BITS_PER_BYTE * i);

        error_state = sigqueue(reciever_pid, SIGUSR1, value);
        ERR_CHECK(error_state == -1, errno);

        error_state = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(error_state == -1, errno);
    }
}

void transmit_data(char *data, const size_t data_size, const pid_t reciever_pid, sigset_t waitset)
{
    int error_state = 0;

    void **void_data = (void **) data;
    const size_t void_data_size = data_size / sizeof(void *);
    
    siginfo_t siginfo;
    sigval_t  value;

    // All full blocks (8 bytes)
    for (size_t i = 0; i < void_data_size; ++i)
    {
        value.sival_ptr = void_data[i];

        error_state = sigqueue(reciever_pid, SIGUSR1, value);
        ERR_CHECK(error_state == -1, errno);

        error_state = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(error_state == -1, errno);
    }

    size_t remainder_size = data_size % sizeof(void *);
    size_t passed_size    = void_data_size * sizeof(void *);
    value.sival_ptr       = NULL;

    // Remainder (less than 8 bytes)
    for (size_t i = 0; i < remainder_size; ++i)
    {
        value.sival_int = data[passed_size + i];

        error_state = sigqueue(reciever_pid, SIGUSR1, value);
        ERR_CHECK(error_state == -1, errno);

        error_state = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(error_state == -1, errno);
    }
}
