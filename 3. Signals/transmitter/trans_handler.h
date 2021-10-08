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


#endif // !TRANS_HANDLER_H_
