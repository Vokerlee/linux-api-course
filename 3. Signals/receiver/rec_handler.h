#ifndef REC_HANDLER_H_
#define REC_HANDLER_H_

#include "rec_err.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

extern const int SIGRT_TRANSMIT;
extern const int SIGRT_TERM;
extern const int SIGRT_BUSY;

size_t get_data_size(sigset_t waitset, pid_t *transmitter_pid);

size_t get_data(char *data, size_t data_size, sigset_t waitset, pid_t transmitter_pid);


#endif // !REC_HANDLER_H_
