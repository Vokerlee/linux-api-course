#ifndef TRANS_HANDLER_H_
#define TRANS_HANDLER_H_

#include "trans_err.h"
#include "settings.h"
#include "data_manip.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

void busy_receiver_handler(int signal, siginfo_t *siginfo, void *ptr);
void term_receiver_handler(int signal, siginfo_t *siginfo, void *ptr);

void transmit(const char *file_name, sigset_t waitset, const pid_t reciever_pid);

#endif // !TRANS_HANDLER_H_
