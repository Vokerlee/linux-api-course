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

#define MAX_PATH_SIZE 0x200

enum path_type
{
    FULL_PATH,
    NAME
};

void launch_backuper(char src_path[], char dst_path[], const sigset_t waitset);

int lookup_file(char *file_path, const char dir_path[], const enum path_type path_type);

int copy_file(const char src_path[], const char dst_path[], enum copy_type copy_type);

int backup(const char src_path[], const char dst_path[]);


#endif // !BACKUPER_H_
