#ifndef BACKUPER_H_
#define BACKUPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sysexits.h>
#include <err.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <syslog.h>
#include <assert.h>
#include <string.h>
#include <signal.h>


void launch_backuper(const char * const src_path, const char * const dst_path, const sigset_t waitset);


#endif // !BACKUPER_H_
