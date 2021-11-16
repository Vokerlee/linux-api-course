#ifndef DAEMON_H_
#define DAEMON_H_

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

#define CPF_CLOEXEC 1

#define BD_NO_CHDIR           01    // Don't chdir("/")
#define BD_NO_CLOSE_FILES     02    // Don't close all open files
#define BD_NO_REOPEN_STD_FDS  04    // Don't reopen stdin, stdout, and stderr to /dev/null
#define BD_NO_UMASK0         010    // Don't do a umask(0) 

#define BD_MAX_CLOSE  8192          // Maximum file descriptors to close if sysconf(_SC_OPEN_MAX) is indeterminate

enum copy_type
{
    DEEP_COPY    = 1,
    SHALLOW_COPY = 2
};

int become_daemon(int flags);
int create_unique_pid_file(const char *prog_name, const char *pid_file, int flags);
int set_signals(sigset_t *waitset);

int set_lock(int fd, int type, int whence, int start, int len);

#endif // !DAEMON_H_
