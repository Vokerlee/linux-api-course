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

extern const size_t BUFFER_SIZE;
extern const size_t FILENAME_SIZE;

void usr1_handler(int signal);

#endif // !REC_HANDLER_H_
