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

// SIGNALS SETTINGS

    sigset_t waitset;
    sigemptyset(&waitset);

    sigaddset(&waitset, SIGUSR1);
    sigaddset(&waitset, SIGUSR2);
    sigaddset(&waitset, SIGTERM);
    sigaddset(&waitset, SIGINT);
    
    errno = 0;
    int error_state = sigprocmask(SIG_BLOCK, &waitset, NULL);
    ERR_CHECK(error_state == -1, errno);

// CALCULATING TIME & TRANSMITTING

    struct timeval  tv_start = {0};
    struct timezone tm_start = {0};
    int time_err = gettimeofday(&tv_start, &tm_start);
    ERR_CHECK(time_err == -1, errno);

    transmit_size(data_size, reciever_pid, waitset);

    transmit_data(data, data_size, reciever_pid, waitset);

    kill(reciever_pid, SIGTERM);

    struct timeval  tv_end = {0};
    struct timezone tm_end = {0};
    time_err = gettimeofday(&tv_end, &tm_end);
    ERR_CHECK(time_err == -1, errno);

    time_t      d_sec  = tv_end.tv_sec  - tv_start.tv_sec;
    suseconds_t d_msec = tv_end.tv_usec - tv_start.tv_usec;

    double exec_time = (double) d_sec + (double) d_msec / 1e6;

    printf("Transmission rate: %lg Kb/s\n",   data_size / exec_time / 1e3);
    printf("Transmission rate: %lg bits/s\n", BITS_PER_BYTE * data_size / exec_time);

    free(data);
}

void transmit_size(const size_t data_size, const pid_t reciever_pid, sigset_t waitset)
{
    int error_state = 0;
    int signal = 0;
    
    siginfo_t siginfo;
    sigval_t  value;

    size_t counter = 0;

    while (counter < sizeof(size_t))
    {
        size_t bit_mask = 0xFF << (BITS_PER_BYTE * counter);
        value.sival_int = (bit_mask & data_size) >> (BITS_PER_BYTE * counter);

        error_state = sigqueue(reciever_pid, SIGUSR1, value);
        sleep(1);
        ERR_CHECK(error_state == -1, errno);

        while(1)
        {
            signal = sigwaitinfo(&waitset, &siginfo);
            ERR_CHECK(error_state == -1, errno);

            if (signal == SIGINT || signal == SIGTERM)
            {
                kill(reciever_pid, SIGTERM);
                exit(0);
            }
            else 
            {
                if (siginfo.si_pid == reciever_pid)
                    break;
                else
                    continue;
            }
        }
        
        counter++;
    }
}

void transmit_data(char *data, const size_t data_size, const pid_t reciever_pid, sigset_t waitset)
{
    int error_state = 0;
    int signal = 0;

    void **void_data = (void **) data;
    const size_t void_data_size = data_size / sizeof(void *);
    
    siginfo_t siginfo;
    sigval_t  value;

    size_t counter = 0;

    // All full blocks (8 bytes)
    while (counter < void_data_size)
    {
        value.sival_ptr = void_data[counter];

        error_state = sigqueue(reciever_pid, SIGUSR1, value);
        ERR_CHECK(error_state == -1, errno);

        while(1)
        {
            signal = sigwaitinfo(&waitset, &siginfo);
            ERR_CHECK(error_state == -1, errno);

            if (signal == SIGINT || signal == SIGTERM)
            {
                kill(reciever_pid, SIGTERM);
                exit(0);
            }
            else
            {
                if (siginfo.si_pid == reciever_pid)
                    break;
                else
                    continue;
            }
        }

        counter++;
    }

    size_t remainder_size = data_size % sizeof(void *);
    size_t passed_size    = void_data_size * sizeof(void *);
    value.sival_ptr       = NULL;
    counter = 0;

    // Remainder (less than 8 bytes)
    while (counter < remainder_size)
    {
        value.sival_int = data[passed_size + counter];

        error_state = sigqueue(reciever_pid, SIGUSR1, value);
        ERR_CHECK(error_state == -1, errno);

        while(1)
        {
            signal = sigwaitinfo(&waitset, &siginfo);
            ERR_CHECK(error_state == -1, errno);

            if (signal == SIGINT || signal == SIGTERM)
            {
                kill(reciever_pid, SIGTERM);
                exit(0);
            }
            else
            {
                if (siginfo.si_pid == reciever_pid)
                    break;
                else
                    continue;
            }
        }

        counter++;
    }
}
