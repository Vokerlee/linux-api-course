#ifndef DATA_MANIP_H_
#define DATA_MANIP_H_

#include "settings.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


void set_signals_mask(sigset_t *waitset);

void transmit_size(const size_t data_size, const pid_t reciever_pid);
void transmit_data(char *data, const size_t data_size, const pid_t reciever_pid);

size_t get_data_size(sigset_t waitset, pid_t *transmitter_pid);
size_t get_data(char *data, size_t data_size, sigset_t waitset, pid_t transmitter_pid);


#endif // !DATA_MANIP_H_
