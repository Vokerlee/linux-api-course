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
#include <sys/time.h>

extern const int SIGRT_TRANSMIT;
extern const int SIGRT_TERM;
extern const int SIGRT_BUSY;

void busy_receiver_handler(int signal, siginfo_t *siginfo, void *ptr);
void term_receiver_handler(int signal, siginfo_t *siginfo, void *ptr);

void transmit(const char *file_name, const pid_t reciever_pid);

void transmit_size(const size_t data_size, const pid_t reciever_pid);

void transmit_data(char *data, const size_t data_size, const pid_t reciever_pid);


#endif // !TRANS_HANDLER_H_
