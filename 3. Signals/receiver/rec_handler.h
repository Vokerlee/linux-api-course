#ifndef REC_HANDLER_H_
#define REC_HANDLER_H_

#include "rec_err.h"
#include "settings.h"
#include "data_manip.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

size_t get_data_master(int fd, sigset_t waitset);

#endif // !REC_HANDLER_H_
