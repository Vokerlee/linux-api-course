#include "daemon.h"
#include "backuper.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <utime.h>

extern enum copy_type COPY_TYPE;
extern const char *DAEMON_NAME;

unsigned int BACKUP_PERIOD = 40; // in seconds

void launch_backuper(char src_path[], char dst_path[], const sigset_t waitset)
{
    assert(src_path);
    assert(dst_path);

    siginfo_t siginfo;
    int signal = 0;

    syslog(LOG_INFO, "Beginning of backup session");
    unsigned int remain_seconds = alarm(BACKUP_PERIOD);
    syslog(LOG_INFO, "The following backup is in %u seconds", BACKUP_PERIOD);

    while(1)
    {
        errno = 0;
        int signal = sigwaitinfo(&waitset, &siginfo);
        
        switch(signal)
        {
            case SIGALRM:
                remain_seconds = alarm(BACKUP_PERIOD);
                backup(src_path, dst_path);
                syslog(LOG_INFO, "The following backup is in %u seconds", BACKUP_PERIOD);
                break;

            case SIGINT:
            case SIGTERM:
            case SIGQUIT:
                syslog(LOG_INFO, "Backuper terminates [User decision]");
                return;

            case SIGUSR1:
                BACKUP_PERIOD = siginfo.si_value.sival_int;
                syslog(LOG_INFO, "New period is to be set: %u seconds", BACKUP_PERIOD);
                break;
            case SIGUSR2:
                syslog(LOG_INFO, "Backuper gets SIGUSR2");
                break;

            default:
                syslog(LOG_ERR, "Unknown signal has been catched [SIGNAL %d]", signal);
        }
    }
}

int copy_file(const char src_path[], const char dst_path[], enum copy_type copy_type)
{
    assert(src_path);
    assert(dst_path);

    char dst[MAX_PATH_SIZE] = {0}; // buffer
    char src[MAX_PATH_SIZE] = {0}; // buffer

    strcpy(src, src_path);
    strcpy(dst, dst_path);

    struct stat src_info;

    int error_state = stat(src, &src_info);
    if (error_state == -1)
    {
        syslog(LOG_ERR, "Error while using stat() for \"%s\"", src);
        return -1;
    }

    int type = src_info.st_mode;
    char *argv[6] = {"cp"};

    if (S_ISDIR(type) || S_ISLNK(type) || S_ISREG(type))
    {
        argv[1] = "-r";

        if (copy_type == DEEP_COPY)
            argv[2] = "-L";
        else
            argv[2] = "-P";

        argv[3] = src;
        argv[4] = dst;
    }
    else
    {
        syslog(LOG_ERR, "File \"%s\" cannot be copied (see backuper description)", src);
        return -1;
    }

    int status = 0;

    if (fork() == 0)
    {
        errno = 0;
        int error_state = execvp(argv[0], argv);
        if (error_state == -1)
        {
            syslog(LOG_ERR, "Program \"%s\" cannot be launched", argv[0]);
            return -1;
        }

    }

    wait(&status); // wait for the child

    syslog(LOG_INFO, "File \"%s\" successfully copied to \"%s\"", src, dst);

    return 0;
}

int backup(const char src_path[], const char dst_path[])
{
    syslog(LOG_INFO, "\nBack up of \"%s\" to \"%s\" begins right now:", src_path, dst_path);

    char dst_name[MAX_PATH_SIZE]      = {0}; // buffer
    char src_name[MAX_PATH_SIZE]      = {0}; // buffer
    char src_real_path[MAX_PATH_SIZE] = {0}; // buffer

    int error_state = 0;

    struct stat src_info;
    struct stat dst_info;
    struct stat link_info;

    error_state = stat(src_path, &src_info);
    if (error_state == -1)
    {
        syslog(LOG_ERR, "Error while using stat() for \"%s\"", src_path);
        return -1;
    }

    if (S_ISDIR(src_info.st_mode) || (S_ISLNK(src_info.st_mode) && COPY_TYPE == DEEP_COPY))
    {
        if (S_ISLNK(src_info.st_mode))
        {
            error_state = readlink(src_path, src_real_path, MAX_PATH_SIZE);
            {
                syslog(LOG_ERR, "Error while using readlink() for \"%s\": %s", src_path, strerror(errno));
                return -1;
            }
        }
        else
            strcpy(src_real_path, src_path);

        DIR *src_dir = opendir(src_real_path);
        if (src_dir == NULL)
        {
            syslog(LOG_ERR, "Error while using opendir() for \"%s\" in backup()", src_real_path);
            return -1;
        }

        struct dirent *src_entry = NULL;
        while ((src_entry = readdir(src_dir)) != NULL)
        {
            if (strcmp(src_entry->d_name, ".") == 0)
                continue;
            if (strcmp(src_entry->d_name, "..") == 0)
                continue;

            snprintf(src_name, sizeof(src_name), "%s/%s", src_real_path, src_entry->d_name);
            snprintf(dst_name, sizeof(dst_name), "%s/%s", dst_path, src_entry->d_name);

            if (lookup_file(src_entry->d_name, dst_path, NAME))
            {
                syslog(LOG_INFO, "File \"%s\" already exists in \"%s\"", src_entry->d_name, dst_path);

                error_state = stat(dst_name, &dst_info);
                if (error_state == -1)
                {
                    syslog(LOG_ERR, "Error while using stat() for \"%s\"", dst_name);
                    return -1;
                }

                error_state = stat(src_name, &src_info);
                if (error_state == -1)
                {
                    syslog(LOG_ERR, "Error while using stat() for \"%s\"", src_name);
                    return -1;
                }

                if (dst_info.st_mtime < src_info.st_mtime)
                {
                    syslog(LOG_INFO, "But file \"%s\" has already been changed in \"%s\" => back it up", src_entry->d_name, src_real_path);

                    if (src_entry->d_type == DT_DIR)
                    {
                        if (backup(src_name, dst_name) == -1)
                            return -1;
                    }
                    else if (src_entry->d_type == DT_LNK && COPY_TYPE == DEEP_COPY)
                    {
                        if (backup(src_name, dst_name) == -1)
                            return -1;
                    }
                    else if (src_entry->d_type == DT_REG || src_entry->d_type == DT_LNK)
                    {
                        if (copy_file(src_name, dst_path, COPY_TYPE) == -1)
                            return -1;
                    }

                    // UPDATE TIME OF MODIFICATION
                      
                    struct utimbuf time_mod = {src_info.st_mtime, src_info.st_mtime};
                    error_state = utime(dst_path, &time_mod);
                    if (error_state == -1)
                    {
                        syslog(LOG_ERR, "Error while using utime() for \"%s\"", dst_path);
                        return -1;
                    }
                }
            }
            else
            {
                syslog(LOG_INFO, "File \"%s\" doesn't exist in \"%s\" => back it up", src_entry->d_name, dst_path);

                if (copy_file(src_name, dst_path, COPY_TYPE) == -1)
                    return -1;
            }

            memset(src_name, 0, sizeof(src_name));
        }

        closedir(src_dir);
    }

    return 0;
}

int lookup_file(char *file_path, const char dir_path[], const enum path_type path_type)
{ 
    assert(file_path);
    assert(dir_path);

    if (path_type == FULL_PATH) // file_path must be just a filename
        file_path = basename(file_path);
    else if (path_type != NAME)
        return -1;

    DIR *directory = opendir(dir_path);
    if (directory == NULL)
    {
        syslog(LOG_ERR, "Error while using opendir() for \"%s\" in lookup()", dir_path);
        return -1;
    }

    struct dirent *entry = NULL;
    while ((entry = readdir(directory)) != NULL)
    {
        assert(entry);

        if (strcmp(entry->d_name, file_path) == 0)
        {
            closedir(directory);
            return 1;
        }
    }
    
    closedir(directory);

    return 0;
}

