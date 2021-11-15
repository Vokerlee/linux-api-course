#include "trans_handler.h"

const size_t BITS_PER_BYTE = 8;

const int SIGRT_TRANSMIT = 34;
const int SIGRT_BUSY     = 38;
const int SIGRT_TERM     = 40;

sig_atomic_t RECEIVER_PID = 0;

const useconds_t OVERFLOW_SIGNALS_DELAY = 200; // 0,2 seconds

void busy_receiver_handler(int signal, siginfo_t *siginfo, void *ptr)
{
    if (siginfo->si_pid == RECEIVER_PID)
    {
        printf("Receiver[PID = %d] is already busy by receiving data\n", RECEIVER_PID);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Signal %d was received by unknown process [PID = %d]\n", SIGRT_BUSY, siginfo->si_pid);
    }
}

void term_receiver_handler(int signal, siginfo_t *siginfo, void *ptr)
{
    if (siginfo->si_pid == RECEIVER_PID)
    {
        printf("Receiver[PID = %d] has been terminated\n", RECEIVER_PID);
        printf("Current process [PID = %d] is to be finished\n", getpid());
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Signal %d was received by unknown process [PID = %d]\n", SIGRT_TERM, siginfo->si_pid);
    }
}

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

// CALCULATING TIME & TRANSMITTING

    struct timeval  tv_start = {0};
    struct timezone tm_start = {0};
    int time_err = gettimeofday(&tv_start, &tm_start);
    ERR_CHECK(time_err == -1, errno);

    transmit_size(data_size, reciever_pid);
    transmit_data(data, data_size, reciever_pid);

    while(1) // in case of real-time signals overflow
    {
        errno = 0;
        sigval_t value;
        int signal = sigqueue(reciever_pid, SIGRT_TERM, value);
        ERR_CHECK(signal == -1 && errno != EAGAIN, errno);

        if (errno == EAGAIN)
        {
            usleep(OVERFLOW_SIGNALS_DELAY);
            continue;
        }

        break;
    }

    struct timeval  tv_end = {0};
    struct timezone tm_end = {0};
    time_err = gettimeofday(&tv_end, &tm_end);
    ERR_CHECK(time_err == -1, errno);

    time_t      d_sec  = tv_end.tv_sec  - tv_start.tv_sec;
    suseconds_t d_msec = tv_end.tv_usec - tv_start.tv_usec;

    double exec_time = (double) d_sec + (double) d_msec / 1e6;

    printf("[PID = %d] Transmission rate: %lg Kb/s\n",   getpid(), data_size / exec_time / 1e3);
    printf("[PID = %d] Transmission rate: %lg bits/s\n", getpid(), BITS_PER_BYTE * data_size / exec_time);

    free(data);
}

void transmit_size(const size_t data_size, const pid_t reciever_pid)
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

        errno = 0;
        error_state = sigqueue(reciever_pid, SIGRT_TRANSMIT, value);
        ERR_CHECK(error_state == -1 && errno != EAGAIN, errno);

        if (errno == EAGAIN) // real-time signals overflow => wait and repeat again();
        {
            usleep(OVERFLOW_SIGNALS_DELAY);
            continue;
        }
        
        counter++;
    }
}

void transmit_data(char *data, const size_t data_size, const pid_t reciever_pid)
{
    assert(data);

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

        errno = 0;
        error_state = sigqueue(reciever_pid, SIGRT_TRANSMIT, value);
        ERR_CHECK(error_state == -1 && errno != EAGAIN, errno);

        if (errno == EAGAIN) // real-time signals overflow => wait and repeat again();
        {
            usleep(OVERFLOW_SIGNALS_DELAY);
            continue;
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

        errno = 0;
        error_state = sigqueue(reciever_pid, SIGRT_TRANSMIT, value);
        ERR_CHECK(error_state == -1 && errno != EAGAIN, errno);

        if (errno == EAGAIN) // real-time signals overflow => wait and repeat again();
        {
            usleep(OVERFLOW_SIGNALS_DELAY);
            continue;
        }

        counter++;
    }
}
