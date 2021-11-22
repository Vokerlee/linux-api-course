#include "data_manip.h"
#include "sys_err.h"

const useconds_t OVERFLOW_SIGNALS_DELAY = 200; // 0,2 seconds // for transmitter
const size_t OVERFLOW_MAX_AMOUNT = 10; // for transmitter

static const struct timespec MAX_DELAY = {0, 5 * 1e8}; // 0,5 seconds // for receiver
static const useconds_t SLEEP_DELAY = 200; // 0,2 seconds, in case of too much delay from transmitter // // for receiver

#ifndef ERR_CHECK
#define ERR_CHECK(condition, errnum)                                                                                \
    do {                                                                                                            \
        if (condition)                                                                                              \
        {                                                                                                           \
            fprintf(stderr, "[PID %d] ERROR AT LINE %d, FUNCTION %s:\n\t", getpid(), __LINE__, __FUNCTION__);       \
            error_msg("");                                                                                          \
                                                                                                                    \
            exit(EXIT_FAILURE);                                                                                     \
        }                                                                                                           \
    } while(0)                                                                                 
#endif

// TRANSMITTER OPERATIONS

void transmit_size(const size_t data_size, const pid_t reciever_pid)
{
    int error_state = 0;
    int signal = 0;
    
    siginfo_t siginfo;
    sigval_t  value;

    size_t counter = 0;

    size_t overflow_times = 0;

    while (counter < sizeof(size_t))
    {
        size_t bit_mask = 0xFF << (BITS_PER_BYTE * counter);
        value.sival_int = (bit_mask & data_size) >> (BITS_PER_BYTE * counter);

        errno = 0;
        error_state = sigqueue(reciever_pid, SIGRT_TRANSMIT, value);
        ERR_CHECK(error_state == -1 && errno != EAGAIN, errno);

        if (errno == EAGAIN) // real-time signals overflow => wait and repeat again();
        {
            overflow_times++;

            if (overflow_times == OVERFLOW_MAX_AMOUNT)
                exit(EXIT_FAILURE);

            usleep(OVERFLOW_SIGNALS_DELAY);
            continue;
        }

        overflow_times = 0;
        
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

    size_t overflow_times = 0;

    // All full blocks (8 bytes)
    while (counter < void_data_size)
    {
        value.sival_ptr = void_data[counter];

        errno = 0;
        error_state = sigqueue(reciever_pid, SIGRT_TRANSMIT, value);
        if (error_state == -1 && errno != EAGAIN)
            exit(EXIT_FAILURE);
        else if (errno == EAGAIN) // real-time signals overflow => wait and repeat again();
        {
            overflow_times++;

            if (overflow_times == OVERFLOW_MAX_AMOUNT)
                exit(EXIT_FAILURE);

            usleep(OVERFLOW_SIGNALS_DELAY);
            continue;
        }

        overflow_times = 0;

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
        if (error_state == -1 && errno != EAGAIN)
            exit(EXIT_FAILURE);
        if (errno == EAGAIN) // real-time signals overflow => wait and repeat again();
        {
            overflow_times++;

            if (overflow_times == OVERFLOW_MAX_AMOUNT)
                exit(EXIT_FAILURE);

            usleep(OVERFLOW_SIGNALS_DELAY);
            continue;
        }

        overflow_times = 0;

        counter++;
    }
}

// RECEIVER OPERATIONS

size_t get_data_size(sigset_t waitset, pid_t *transmitter_pid)
{
    assert(transmitter_pid);

    size_t data_size = 0;
    size_t counter   = 0;

    siginfo_t siginfo;
    int signal = 0;

    signal = sigwaitinfo(&waitset, &siginfo);
    ERR_CHECK(signal == -1, errno);

    *transmitter_pid = siginfo.si_pid;

    switch(signal)
    {
        case SIGRT_TRANSMIT:
        {
            data_size = data_size | (siginfo.si_value.sival_int << (BITS_PER_BYTE * counter));
            counter++;
    
            break;
        }

        case SIGRT_TERM:
        case SIGINT:
        case SIGTERM:
        {
            fprintf(stdout, "\nEnd of transmission\n");
            exit(0);
        }
    }

    while (1)
    {
        errno = 0;
        signal = sigtimedwait(&waitset, &siginfo, &MAX_DELAY);
        ERR_CHECK(signal == -1 && errno != EAGAIN, errno);

        if (signal == SIGINT || signal == SIGTERM || signal == SIGRT_TERM)
        {
            fprintf(stdout, "\nEnd of transmission\n");
            exit(0);
        }
        else if (errno == EAGAIN)
        {
            int error = kill(*transmitter_pid, 0);
            if (error == -1)
                exit(EXIT_FAILURE);
            else
                usleep(SLEEP_DELAY);
        }
        else if (signal == SIGRT_TRANSMIT)
        {
            if (siginfo.si_pid == *transmitter_pid)
            {
                data_size = data_size | (siginfo.si_value.sival_int << (BITS_PER_BYTE * counter));
                counter++;
            }
            else
            {
                union sigval value;
                sigqueue(siginfo.si_pid, SIGRT_BUSY, value);

                continue;
            }
                
        }

        if (counter >= sizeof(size_t))
            return data_size;
    }

    return data_size;
}

size_t get_data(char *data, size_t data_size, sigset_t waitset, pid_t transmitter_pid)
{
    assert(data);

    int error_state = 0;
    void **void_data = (void **) data;
    const size_t void_data_size = data_size / sizeof(void *);

    size_t bytes_counter = 0;
    size_t counter = 0;
    int signal = 0;

    siginfo_t siginfo;

    while(counter < void_data_size)
    {
        errno = 0;
        signal = sigtimedwait(&waitset, &siginfo, &MAX_DELAY);
        ERR_CHECK(signal == -1 && errno != EAGAIN, errno);

        if (signal == SIGRT_TRANSMIT)
        {
            void_data[counter] = siginfo.si_value.sival_ptr;
            bytes_counter += sizeof(void *);
            counter++;
        }
        else if (errno == EAGAIN)
        {
            int error = kill(transmitter_pid, 0);
            if (error == -1)
                return bytes_counter;
            else
                usleep(SLEEP_DELAY);
        }
        else 
            return bytes_counter;

    }

    size_t remainder_size = data_size % sizeof(void *);
    size_t passed_size    = void_data_size * sizeof(void *);
    counter = 0;

    while (counter < remainder_size)
    {
        errno = 0;
        signal = sigtimedwait(&waitset, &siginfo, &MAX_DELAY);
        ERR_CHECK(signal == -1 && errno != EAGAIN, errno);

        if (signal == SIGRT_TRANSMIT)
        {
            data[passed_size + counter] = siginfo.si_value.sival_int;
            bytes_counter += sizeof(char);
            counter++;
        }
        else if (errno == EAGAIN)
        {
            int error = kill(transmitter_pid, 0);
            if (error == -1)
                return bytes_counter;
            else
                usleep(SLEEP_DELAY);
        }
        else
            return bytes_counter;
    }

    return bytes_counter;
}

// GENEREAL FUNCTIONS

void set_signals_mask(sigset_t *waitset)
{
    assert(waitset);

    sigemptyset(waitset);
    sigaddset(waitset, SIGRT_TRANSMIT);
    sigaddset(waitset, SIGRT_TERM);
    sigaddset(waitset, SIGRT_MSG1);
    sigaddset(waitset, SIGRT_MSG2);
    sigaddset(waitset, SIGTERM);
    sigaddset(waitset, SIGINT);
    sigaddset(waitset, SIGUSR1);
    sigaddset(waitset, SIGUSR2);

    int error_state = sigprocmask(SIG_BLOCK, waitset, NULL);
    ERR_CHECK(error_state == -1, errno);
}
