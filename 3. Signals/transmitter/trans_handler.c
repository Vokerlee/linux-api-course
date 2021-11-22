#include "trans_handler.h"

sig_atomic_t RECEIVER_PID = 0;

pid_t MASTER_PID = 0; 
pid_t SLAVES_PID[N_PROCESSES];
pid_t REC_SLAVES_PID[N_PROCESSES];

struct process_data PROC_INFO[N_PROCESSES];

static const struct timespec MAX_DELAY = {2, 5 * 1e8}; // 2,5 seconds // for childs

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
        printf("Receiver [PID = %d] has been terminated\n", RECEIVER_PID);
        printf("Current process [PID = %d] is to be finished\n", getpid());
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Signal %d was received by unknown process [PID = %d]\n", SIGRT_TERM, siginfo->si_pid);
    }
}

void transmit(const char *file_name, sigset_t waitset, const pid_t reciever_pid)
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

// MAIN WORK

    transmit_size(data_size, reciever_pid);
    get_data((char *) REC_SLAVES_PID, sizeof(REC_SLAVES_PID), waitset, reciever_pid);

    size_t process_file_length    = data_size / N_PROCESSES;
    size_t process_file_remainder = data_size % N_PROCESSES;

    off_t cur_pos = 0;

    MASTER_PID = getpid();

    for (int i = 0; i < N_PROCESSES; i++)
    {
        if (i == 0)
        {
            PROC_INFO[i].file_length = process_file_length + process_file_remainder;
            PROC_INFO[i].file_offset = cur_pos;
            cur_pos += PROC_INFO[i].file_length;
        }
        else
        {
            PROC_INFO[i].file_length = process_file_length;
            PROC_INFO[i].file_offset = cur_pos;
            cur_pos += PROC_INFO[i].file_length;
        }

        SLAVES_PID[i] = fork();
        if (SLAVES_PID[i] == -1)
        {
            int saved_errno = errno;
            for (int j = 0; j < i; j++)
                kill(SLAVES_PID[j], SIGKILL);

            ERR_CHECK(SLAVES_PID[i] == -1, saved_errno);
        }

        if (SLAVES_PID[i] == 0)
            break;
    }

    if (getpid() != MASTER_PID) // slave
    {
        siginfo_t siginfo;
        sigval_t  value;
        int signal = 0;

        int my_number = 0;

        signal = sigtimedwait(&waitset, &siginfo, &MAX_DELAY);
        if (signal == SIGUSR1)
            my_number = siginfo.si_value.sival_int;
        else
            exit(EXIT_FAILURE);

        signal = sigtimedwait(&waitset, &siginfo, &MAX_DELAY); // when all receiver slaves are already ready
        if (signal != SIGUSR1)
            exit(EXIT_FAILURE);

        transmit_data(data + PROC_INFO[my_number].file_offset, PROC_INFO[my_number].file_length, REC_SLAVES_PID[my_number]);

        sigqueue(MASTER_PID, SIGRT_MSG1, value);

        free(data);
        exit(EXIT_SUCCESS);
    }
    else // master
    {
        // CALCULATING TIME & TRANSMITTING

        struct timeval  tv_start = {0};
        struct timezone tm_start = {0};
        int time_err = gettimeofday(&tv_start, &tm_start);
        ERR_CHECK(time_err == -1, errno);

        // OTHER STUFF

        siginfo_t siginfo;
        sigval_t value;
        int signal = 0;

        transmit_data((char *) SLAVES_PID, sizeof(SLAVES_PID), reciever_pid);

        for (int i = 0; i < N_PROCESSES; i++)
        {
            value.sival_int = i; 
            sigqueue(SLAVES_PID[i], SIGUSR1, value);
        }

        signal = sigtimedwait(&waitset, &siginfo, &MAX_DELAY); // when all receiver slaves are already ready
        if (signal != SIGUSR1)
            exit(EXIT_FAILURE);

        for (int i = 0; i < N_PROCESSES; ++i) // say to children that everyone is ready
            kill(SLAVES_PID[i], SIGUSR1);

        size_t ready_children = 0;
        while(ready_children != N_PROCESSES) // when all children have done their work
        {
            int signal = sigwaitinfo(&waitset, &siginfo);
            if (signal == SIGRT_MSG1)
                ready_children++;
            else
                exit(EXIT_FAILURE);
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

        int status = 0;

        for (int i = 0; i < N_PROCESSES; ++i)
        {
            waitpid(SLAVES_PID[i], &status, 0);
            if (status != 0)
                exit(EXIT_FAILURE);
        }
            
        printf("Everything is written\n");
        free(data);
    }
}
