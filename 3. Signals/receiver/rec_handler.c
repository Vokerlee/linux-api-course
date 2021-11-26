#include "rec_handler.h"

extern sig_atomic_t TRANSMITTER_PID;

pid_t MASTER_PID = 0; 
pid_t SLAVES_PID[N_PROCESSES];
pid_t TRANS_SLAVES_PID[N_PROCESSES];

struct process_data PROC_INFO[N_PROCESSES];

static const struct timespec MAX_DELAY = {2, 5 * 1e8}; // 2,5 seconds // for childs

size_t get_data_master(int fd, sigset_t waitset)
{
    printf("My pid = %d\n", getpid());

    pid_t transmitter_pid = 0;

    size_t data_size = get_data_size(waitset, &transmitter_pid);
    TRANSMITTER_PID  = transmitter_pid;
    printf("Transmitter pid = %d\n", transmitter_pid);
    printf("Request size = %zu\n", data_size);

    size_t process_file_length    = data_size / N_PROCESSES;
    size_t process_file_remainder = data_size % N_PROCESSES;

    off_t cur_pos = 0;

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
        if (signal == SIGRT_MSG1)
            my_number = siginfo.si_value.sival_int;
        else
            exit(EXIT_FAILURE);

        signal = sigtimedwait(&waitset, &siginfo, &MAX_DELAY);
        if (signal == SIGRT_MSG1)
            TRANS_SLAVES_PID[my_number] = siginfo.si_value.sival_int;
        else
            exit(EXIT_FAILURE);

        char *data = (char *) calloc(PROC_INFO[my_number].file_length, sizeof(char));
        if (data == NULL)
        {
            sigqueue(MASTER_PID, SIGRT_TERM, value);
            ERR_CHECK(data == NULL, BAD_ALLOC);
        }
        
        sigqueue(MASTER_PID, SIGRT_MSG1, value); // ready to receive
        size_t received_data_size = get_data(data, PROC_INFO[my_number].file_length, waitset, TRANS_SLAVES_PID[my_number]);
        if (received_data_size != PROC_INFO[my_number].file_length)
        {
            sigqueue(MASTER_PID, SIGRT_TERM, value);
            printf("I[%d]: size = %d\n", my_number, received_data_size);
            exit(EXIT_FAILURE);
        }

        sigqueue(MASTER_PID, SIGRT_MSG1, value); // ready to write
        signal = sigtimedwait(&waitset, &siginfo, &MAX_DELAY);
        if (signal == SIGUSR1)
        {
            off_t shift = lseek(fd, PROC_INFO[my_number].file_offset, SEEK_SET);
            if (shift != PROC_INFO[my_number].file_offset)
            {
                sigqueue(MASTER_PID, SIGRT_TERM, value);
                ERR_CHECK(shift != PROC_INFO[my_number].file_offset, errno);
            }

            int n_written = write(fd, data, PROC_INFO[my_number].file_length);
            if (n_written != PROC_INFO[my_number].file_length)
            {
                sigqueue(MASTER_PID, SIGRT_TERM, value);
                ERR_CHECK(n_written != PROC_INFO[my_number].file_length, errno);
            }
            
            sigqueue(MASTER_PID, SIGRT_MSG1, value);
        }
        else
        {
            sigqueue(MASTER_PID, SIGRT_TERM, value);
            exit(EXIT_FAILURE);
        }
            

        free(data);
        exit(EXIT_SUCCESS);
    }
    else // master
    {
        siginfo_t siginfo;
        sigval_t value;
        int signal = 0;

        transmit_data((char *) SLAVES_PID, sizeof(SLAVES_PID), transmitter_pid);
        get_data((char *) TRANS_SLAVES_PID, sizeof(TRANS_SLAVES_PID), waitset, transmitter_pid);

        for (int i = 0; i < N_PROCESSES; i++)
        {
            value.sival_int = i;
            sigqueue(SLAVES_PID[i], SIGRT_MSG1, value);
            value.sival_int = TRANS_SLAVES_PID[i];
            sigqueue(SLAVES_PID[i], SIGRT_MSG1, value);
        }

        size_t ready_children = 0;
        while(ready_children != N_PROCESSES) // ready to receive data
        {
            signal = sigwaitinfo(&waitset, &siginfo);
            if (signal == SIGRT_MSG1)
                ready_children++;
            else if (signal == SIGRT_TERM || signal == SIGINT || signal == SIGTERM)
                exit(EXIT_FAILURE);
            else
                sigqueue(siginfo.si_pid, SIGRT_BUSY, value);
        }

        kill(transmitter_pid, SIGUSR1);

        ready_children = 0;
        while(ready_children != N_PROCESSES) // ready to write data
        {
            signal = sigwaitinfo(&waitset, &siginfo);
            if (signal == SIGRT_MSG1)
                ready_children++;
            else if (signal == SIGRT_TERM || signal == SIGINT || signal == SIGTERM)
                exit(EXIT_FAILURE);
            else
                sigqueue(siginfo.si_pid, SIGRT_BUSY, value);
        }

        for (int i = 0; i < N_PROCESSES; i++)
        {
            kill(SLAVES_PID[i], SIGUSR1);
            signal = sigwaitinfo(&waitset, &siginfo);
            if (signal == SIGRT_MSG1)
                continue;
            else if (signal == SIGRT_TERM || signal == SIGINT || signal == SIGTERM)
                exit(EXIT_FAILURE);
            else
                sigqueue(siginfo.si_pid, SIGRT_BUSY, value);
        }

        printf("Everything is written\n");

        int status = 0;

        for (int i = 0; i < N_PROCESSES; ++i)
        {
            waitpid(SLAVES_PID[i], &status, 0);
            if (status != 0)
                exit(EXIT_FAILURE);
        }
    }

    return 0;
}
