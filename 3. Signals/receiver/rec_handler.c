#include "rec_handler.h"

const size_t BITS_PER_BYTE = 8;

size_t get_data_size(sigset_t waitset, pid_t *transmitter_pid)
{
    size_t data_size = 0;
    size_t counter   = 0;

    siginfo_t siginfo;
    int signal  = 0;

    signal = sigwaitinfo(&waitset, &siginfo);
    ERR_CHECK(signal == -1, errno);

    *transmitter_pid = siginfo.si_pid;

    switch(signal)
    {
        case SIGUSR1:
        case SIGUSR2:
        {
            data_size = data_size | (siginfo.si_value.sival_int << (BITS_PER_BYTE * counter));
            counter++;
    
            break;
        }
        case SIGINT:
        case SIGTERM:
        {
            fprintf(stdout, "\nEnd of transmission\n");
            kill(*transmitter_pid, SIGTERM);

            exit(0);
        }
    }

    kill(siginfo.si_pid, SIGUSR1);

    while (1)
    {
        signal = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(signal == -1, errno);

        if (signal == SIGINT || signal == SIGTERM)
        {
            fprintf(stdout, "\nEnd of transmission\n");
            kill(*transmitter_pid, SIGTERM);

            exit(0);
        }
        else if (signal == SIGUSR1 || signal == SIGUSR2)
        {
            if (siginfo.si_pid == *transmitter_pid)
            {
                data_size = data_size | (siginfo.si_value.sival_int << (BITS_PER_BYTE * counter));
                counter++;
            }
            else
                continue;
        }

        kill(siginfo.si_pid, SIGUSR1);

        if (counter >= sizeof(size_t))
            return data_size;
    }

    return data_size;
}

size_t get_data(char *data, size_t data_size, sigset_t waitset, pid_t transmitter_pid)
{
    int error_state = 0;
    void **void_data = (void **) data;
    const size_t void_data_size = data_size / sizeof(void *);

    size_t bytes_counter = 0;
    size_t counter = 0;

    siginfo_t siginfo;

    while(counter < void_data_size)
    {
        int signal = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(signal == -1, errno);

        if (signal == SIGINT || signal == SIGTERM)
        {
            fprintf(stdout, "\nEnd of transmission\n");
            kill(transmitter_pid, SIGTERM);

            return bytes_counter;
        }
        else if (signal == SIGUSR1 || signal == SIGUSR2)
        {
            if (siginfo.si_pid == transmitter_pid)
            {
                void_data[counter] = siginfo.si_value.sival_ptr;
                bytes_counter += sizeof (void *);
                counter++;
            }
            else
                continue;
        }

        kill(siginfo.si_pid, SIGUSR1);
    }

    size_t remainder_size = data_size % sizeof(void *);
    size_t passed_size    = void_data_size * sizeof(void *);
    counter = 0;

    while (counter < remainder_size)
    {
        int signal = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(signal == -1, errno);

        if (signal == SIGINT || signal == SIGTERM)
        {
            fprintf(stdout, "\nEnd of transmission\n");
            kill(transmitter_pid, SIGTERM);

            return bytes_counter;
        }
        else if (signal == SIGUSR1 || signal == SIGUSR2)
        {
            if (siginfo.si_pid == transmitter_pid)
            {
                data[passed_size + counter] = siginfo.si_value.sival_int;
                bytes_counter += sizeof(char);
                counter++;
            }
            else
                continue;
        }

        kill(siginfo.si_pid, SIGUSR1);
    }

    return bytes_counter;
}
