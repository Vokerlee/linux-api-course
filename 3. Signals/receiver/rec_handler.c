#include "rec_handler.h"

extern pid_t TRANSMITTER_PID;

const size_t BITS_PER_BYTE = 8;

const int SIGRT_TRANSMIT = 34;
const int SIGRT_BUSY     = 38;
const int SIGRT_TERM     = 40;

const size_t MAX_PROCESSES_AMOUNT = 1000;

size_t get_data_size(sigset_t waitset, pid_t *transmitter_pid)
{
    assert(transmitter_pid);

    size_t data_size = 0;
    size_t counter   = 0;

    siginfo_t siginfo;
    int signal = 0;

    struct timespec delay = {1, 0};

    signal = sigwaitinfo(&waitset, &siginfo);
    ERR_CHECK(signal == -1, errno);

    *transmitter_pid = siginfo.si_pid;
    TRANSMITTER_PID  = siginfo.si_pid;

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
        signal = sigtimedwait(&waitset, &siginfo, &delay);
        ERR_CHECK(signal == -1 && errno != EAGAIN, errno);

        if (signal == SIGINT || signal == SIGTERM || signal == SIGRT_TERM)
        {
            fprintf(stdout, "\nEnd of transmission\n");
            exit(0);
        }
        else if (errno == EAGAIN)
        {
            fprintf(stdout, "\nEnd of transmission (too large delay)\n");
            exit(0);
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

    siginfo_t siginfo;

    struct timespec delay = {1, 0};

    while(counter < void_data_size)
    {
        errno = 0;
        int signal = sigtimedwait(&waitset, &siginfo, &delay);
        ERR_CHECK(signal == -1 && errno != EAGAIN, errno);

        if (signal == SIGINT || signal == SIGTERM || signal == SIGRT_TERM)
        {
            fprintf(stdout, "\nEnd of transmission\n");
            return bytes_counter;
        }
        else if (errno == EAGAIN)
        {
            fprintf(stdout, "\nEnd of transmission (too large delay)\n");
            return bytes_counter;
        }
        else if (signal == SIGRT_TRANSMIT)
        {
            if (siginfo.si_pid == transmitter_pid)
            {
                void_data[counter] = siginfo.si_value.sival_ptr;
                bytes_counter += sizeof(void *);
                counter++;
            }
            else
            {
                union sigval value;
                sigqueue(siginfo.si_pid, SIGRT_BUSY, value);
                
                continue;
            }
                
        }
    }

    size_t remainder_size = data_size % sizeof(void *);
    size_t passed_size    = void_data_size * sizeof(void *);
    counter = 0;

    while (counter < remainder_size)
    {
        errno = 0;
        int signal = sigtimedwait(&waitset, &siginfo, &delay);
        ERR_CHECK(signal == -1 && errno != EAGAIN, errno);

        if (signal == SIGINT || signal == SIGTERM || signal == SIGRT_TERM)
        {
            fprintf(stdout, "\nEnd of transmission\n");
            return bytes_counter;
        }
        else if (errno == EAGAIN)
        {
            fprintf(stdout, "\nEnd of transmission (too large delay)\n");
            return bytes_counter;
        }
        else if (signal == SIGRT_TRANSMIT)
        {
            if (siginfo.si_pid == transmitter_pid)
            {
                void_data[counter] = siginfo.si_value.sival_ptr;
                bytes_counter += sizeof(void *);
                counter++;
            }
            else
            {
                union sigval value;
                sigqueue(siginfo.si_pid, SIGRT_BUSY, value);
                
                continue;
            }
        }
    }

    return bytes_counter;
}
