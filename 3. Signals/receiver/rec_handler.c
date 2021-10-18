#include "rec_handler.h"

const size_t BITS_PER_BYTE = 8;

size_t get_data_size(sigset_t waitset)
{
    size_t data_size = 0;
    size_t counter   = 0;

    siginfo_t siginfo;

    while (1)
    {
        int signal = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(signal == -1, errno);

        switch(signal)
        {
            case SIGUSR1:
            case SIGUSR2:
            {
                data_size = data_size | (siginfo.si_value.sival_int << (BITS_PER_BYTE * counter));
                counter++;
        
                break;
            }
                
            case SIGTERM:
            {
                fprintf(stdout, "End of transmission\n");
                exit(0);
            }
        }

        kill(siginfo.si_pid, SIGUSR1);

        if (counter >= sizeof(size_t))
            return data_size;
    }

    return data_size;
}

void get_data(char *data, size_t data_size, sigset_t waitset)
{
    void **void_data = (void **) data;
    const size_t void_data_size = data_size / sizeof(void *);

    size_t counter = 0;

    siginfo_t siginfo;

    for (size_t i = 0; i < void_data_size; ++i)
    {
        int signal = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(signal == -1, errno);

        switch(signal)
        {
            case SIGUSR1:
            case SIGUSR2:
            {
                void_data[i] = siginfo.si_value.sival_ptr;
                counter++;
                break;
            }
                
            case SIGTERM:
            {
                fprintf(stdout, "End of transmission\n");
                exit(0);
            }
        }

        kill(siginfo.si_pid, SIGUSR1);
    }

    size_t remainder_size = data_size % sizeof(void *);
    size_t passed_size    = void_data_size * sizeof(void *);

    for (size_t i = 0; i < remainder_size; ++i)
    {
        int signal = sigwaitinfo(&waitset, &siginfo);
        ERR_CHECK(signal == -1, errno);

        switch(signal)
        {
            case SIGUSR1:
            case SIGUSR2:
            {
                data[passed_size + i] = siginfo.si_value.sival_int;
                counter++;
                break;
            }
                
            case SIGTERM:
            {
                fprintf(stdout, "End of transmission.\n");
                exit(0);
            }
        }

        kill(siginfo.si_pid, SIGUSR1);
    }
}
