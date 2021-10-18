#ifndef TRANS_HANDLER_H_
#define TRANS_HANDLER_H_

#include "trans_err.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>


void transmit(const char *file_name, const pid_t reciever_pid);

void transmit_size(const size_t data_size, const pid_t reciever_pid, sigset_t waitset);

void transmit_data(const char *data, const size_t data_size, const pid_t reciever_pid, sigset_t waitset);


#endif // !TRANS_HANDLER_H_
