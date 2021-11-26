#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <sys/types.h>
#include <unistd.h>

#define BITS_PER_BYTE 8

#define SIGRT_TRANSMIT 34
#define SIGRT_BUSY     38
#define SIGRT_TERM     40

#define SIGRT_MSG1     44
#define SIGRT_MSG2     45

#define N_PROCESSES 48

struct process_data
{
    off_t file_offset;
    off_t file_length;
};


#endif // !SETTINGS_H_
