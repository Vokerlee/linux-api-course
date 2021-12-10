#include "daemon.h"
#include "backuper.h"
#include "hash_table.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <utime.h>
#include <sys/inotify.h>

extern const char *DAEMON_NAME;
const char *FIFO_NAME = "/tmp/backuperd_fifo";

hashtable_t *WATCH_FD_HASH_TABLE = NULL;

extern enum copy_type COPY_TYPE;

unsigned int BACKUP_PERIOD   = 40; // in seconds
unsigned int RECONFIG_PERIOD = 16; // in BACKUP_PERIOD

size_t MAX_AMOUNT_OF_DIRECTORIES = 0x20000;

enum backup_mode BACKUP_MODE = RECURSIVE_MODE;

void launch_backuper(char src_path[], char dst_path[], const sigset_t waitset)
{
    assert(src_path);
    assert(dst_path);

    siginfo_t siginfo;
    int signal = 0;

    int reconfig_counter = 0;
    int inot_fd = 0;

    char rel_path[] = "";

    syslog(LOG_INFO, "Initial backup begins...");
    backup(src_path, dst_path);
    syslog(LOG_INFO, "Initial backup successfully finished");

    if (BACKUP_MODE == INOTIFY_MODE)
    {
        inot_fd = inotify_init1(IN_NONBLOCK);
        if (inot_fd == -1)
        {
            syslog(LOG_ERR, "inotify initialization error");
            exit(EXIT_FAILURE);
        }

        WATCH_FD_HASH_TABLE = ht_create(MAX_AMOUNT_OF_DIRECTORIES);
        if (WATCH_FD_HASH_TABLE == NULL)
        {
            syslog(LOG_ERR, "error while creating hashtable for watch descriptors (inotify)");
            exit(EXIT_FAILURE);
        }

        syslog(LOG_INFO, "Watch initialization starts");
        watch_initialization(inot_fd, src_path, rel_path);
        syslog(LOG_INFO, "Watch initialization successfully finished");
    }

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
                if (BACKUP_MODE == INOTIFY_MODE)
                {
                    reconfig_counter++;
                    if (reconfig_counter == RECONFIG_PERIOD)
                    {
                        syslog(LOG_INFO, "Reconfiguration for security starts...");
                        watch_off(inot_fd);

                        syslog(LOG_INFO, "Scanning all source directory starts...");
                        backup(src_path, dst_path);
                        syslog(LOG_INFO, "Scanning successfully finished");

                        watch_initialization(inot_fd, src_path, rel_path);
                        syslog(LOG_INFO, "Reconfiguration successfully finished");
                        
                        reconfig_counter = 0;
                    }
                    else
                    {
                        remain_seconds = alarm(BACKUP_PERIOD);
                        backup_update(inot_fd, src_path, dst_path);
                        syslog(LOG_INFO, "The following backup is in %u seconds", BACKUP_PERIOD);
                    }
                    
                    list_entry_t *list_entry = WATCH_FD_HASH_TABLE->start;

                    while (list_entry)
                    {
                        syslog(LOG_INFO, "dir = %s, fd = %d\n", list_entry->hash_table_entry->value, list_entry->hash_table_entry->key);
                        list_entry = list_entry->next;
                    }
                }
                else if (BACKUP_MODE == RECURSIVE_MODE)
                {
                    backup(src_path, dst_path);
                    syslog(LOG_INFO, "Backup successfully finished");
                }

                break;

            case SIGINT:
            case SIGTERM:
            case SIGQUIT:
                syslog(LOG_INFO, "Backuper terminates [User decision]");
                return;

            case SIGUSR1:
                syslog(LOG_ERR, "value = %d", siginfo.si_value.sival_int);
                if (siginfo.si_value.sival_int > 0)
                {
                    BACKUP_PERIOD = siginfo.si_value.sival_int;
                    syslog(LOG_INFO, "New period is to be set: %u seconds", BACKUP_PERIOD);
                }
                else if (siginfo.si_value.sival_int == -1 && BACKUP_MODE == RECURSIVE_MODE) // change mode to INOTIFY_MODE
                {
                    syslog(LOG_INFO, "Change mode to INOTIFY_MODE...");
                    inot_fd = inotify_init1(IN_NONBLOCK);
                    if (inot_fd == -1)
                    {
                        syslog(LOG_ERR, "inotify initialization error");
                        exit(EXIT_FAILURE);
                    }

                    WATCH_FD_HASH_TABLE = ht_create(MAX_AMOUNT_OF_DIRECTORIES);
                    if (WATCH_FD_HASH_TABLE == NULL)
                    {
                        syslog(LOG_ERR, "error while creating hashtable for watch descriptors (inotify)");
                        exit(EXIT_FAILURE);
                    }

                    syslog(LOG_INFO, "Watch initialization starts");
                    watch_initialization(inot_fd, src_path, rel_path);
                    syslog(LOG_INFO, "Watch initialization successfully finished");

                    BACKUP_MODE = INOTIFY_MODE;

                    syslog(LOG_INFO, "Backup for no data loss begins...");
                    backup(src_path, dst_path);
                    syslog(LOG_INFO, "Backup for no data loss successfully finished");

                    syslog(LOG_INFO, "Daemon mode was succcessfully changed to INOTIFY_MODE");
                }
                else if (siginfo.si_value.sival_int == -2 && BACKUP_MODE == INOTIFY_MODE) // change mode to RECURSIVE_MODE
                {
                    syslog(LOG_INFO, "Change mode to RECURSIVE_MODE...");
                    watch_off(inot_fd);
                    ht_delete(WATCH_FD_HASH_TABLE);
                    WATCH_FD_HASH_TABLE = NULL;

                    BACKUP_MODE = RECURSIVE_MODE;

                    syslog(LOG_INFO, "Daemon mode was succcessfully changed to RECURSIVE_MODE");
                }
                
                break;

            case SIGUSR2:
                syslog(LOG_INFO, "Ready to get new directories...");

                int fifo_fd = 0;
                if ((fifo_fd = open(FIFO_NAME, O_RDONLY, 0666)) == -1)
                {
                    perror("open()");
                    exit(EXIT_FAILURE);
                }

                memset(src_path, 0, MAX_PATH_SIZE);
                memset(dst_path, 0, MAX_PATH_SIZE);

                read(fifo_fd, src_path, MAX_PATH_SIZE);
                read(fifo_fd, dst_path, MAX_PATH_SIZE);

                syslog(LOG_INFO, "New source directory: %s", src_path);
                syslog(LOG_INFO, "New destination directory: %s", dst_path);

                if (BACKUP_MODE == INOTIFY_MODE)
                {
                    watch_off(inot_fd);
                    syslog(LOG_INFO, "Hash table is updated");

                    syslog(LOG_INFO, "Initial backup begins...");
                    backup(src_path, dst_path);
                    syslog(LOG_INFO, "Initial backup successfully finished");

                    watch_initialization(inot_fd, src_path, rel_path);
                    syslog(LOG_INFO, "Daemon is ready to do its main work");
                }
                else if (BACKUP_MODE == RECURSIVE_MODE)
                {
                    syslog(LOG_INFO, "Initial backup begins...");
                    backup(src_path, dst_path);
                    syslog(LOG_INFO, "Initial backup successfully finished");
                }

                reconfig_counter = 0;

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
    syslog(LOG_INFO, "Back up of \"%s\" to \"%s\" begins right now:", src_path, dst_path);

    char dst_name[MAX_PATH_SIZE]      = {0}; // buffer
    char src_name[MAX_PATH_SIZE]      = {0}; // buffer
    char src_real_path[MAX_PATH_SIZE] = {0}; // buffer

    int error_state = 0;

    struct stat src_info  = {0};
    struct stat dst_info  = {0};
    struct stat link_info = {0};

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

int watch_initialization(int inot_fd, char src_path[], char src_rel_path[])
{
    struct stat src_info  = {0};
    struct stat link_info = {0};

    char src_name[MAX_PATH_SIZE]     = {0}; // buffer
    char src_rel_name[MAX_PATH_SIZE] = {0}; // buffer

    int error_state = 0;

    error_state = lstat(src_path, &src_info);
    if (error_state == -1)
    {
        syslog(LOG_ERR, "Error while using stat() for \"%s\"", src_path);
        return -1;
    }

    if ((S_ISDIR(src_info.st_mode) && !S_ISLNK(src_info.st_mode)) || (S_ISLNK(src_info.st_mode) && (COPY_TYPE == DEEP_COPY)))
    {
        int watch_fd = inotify_add_watch(inot_fd, src_path, IN_CREATE | IN_DELETE_SELF | IN_MOVE_SELF | IN_MODIFY | IN_MOVE);
        if (watch_fd == -1)
        {
            syslog(LOG_ERR, "Error while using inotify_add_watch() for \"%s\"", src_path);
            return -1;
        }

        int ht_state = ht_set(WATCH_FD_HASH_TABLE, watch_fd, src_rel_path);
        if (ht_state == -1)
        {
            syslog(LOG_ERR, "Error while using ht_set() for \"%s\"", src_path);
            return -1;
        }
            
        DIR *src_dir = opendir(src_path);
        if (src_dir == NULL)
        {
            syslog(LOG_ERR, "Error while using opendir() for \"%s\" in backup()", src_path);
            return -1;
        }

        struct dirent *src_entry = NULL;
        while ((src_entry = readdir(src_dir)) != NULL)
        {
            if (strcmp(src_entry->d_name, ".") == 0)
                continue;
            if (strcmp(src_entry->d_name, "..") == 0)
                continue;

            if (src_entry->d_type == DT_DIR || ((src_entry->d_type == DT_LNK) && (COPY_TYPE == DEEP_COPY)))
            {
                snprintf(src_name,     sizeof(src_name), "%s/%s", src_path, src_entry->d_name);
                snprintf(src_rel_name, sizeof(src_name), "%s/%s", src_rel_path, src_entry->d_name);

                if (watch_initialization(inot_fd, src_name, src_rel_name) == -1)
                    return -1;
            }

            memset(src_name, 0, MAX_PATH_SIZE);
        }

        closedir(src_dir);
    }

    return 0;
}

int backup_update(int inot_fd, const char src_path[], const char dst_path[])
{
    char buf[NAME_MAX]
        __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event = NULL;
    ssize_t len = 0;

    char src_name[MAX_PATH_SIZE] = {0}; // buffer
    char dst_name[MAX_PATH_SIZE] = {0}; // buffer

    char rel_path[MAX_PATH_SIZE] = {0};

    while(1)
    {
        errno = 0;
        len = read(inot_fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN)
        {
            syslog(LOG_ERR, "read() from inorify fd error");
            break;
        }

        if (len <= 0 && errno == EAGAIN)
        {
            syslog(LOG_INFO, "Everything is backuped");
            break;
        }
            
        else if (len <= 0)
        {
            syslog(LOG_ERR, "Cannot read info about events");
            return -1;
        }

        for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) // read all structures in directory
        {
            event = (const struct inotify_event *) ptr;

            memset(src_name, 0, MAX_PATH_SIZE);
            memset(dst_name, 0, MAX_PATH_SIZE);
            memset(rel_path, 0, MAX_PATH_SIZE);

            if ((event->mask & IN_CREATE) || (event->mask & IN_MOVED_TO))
            {
                syslog(LOG_INFO, "File \"%s\" was created in \"%s/%s\" (or moved to) => back it up", event->name, src_path, ht_get(WATCH_FD_HASH_TABLE, event->wd));
                
                snprintf(src_name, sizeof(src_name), "%s%s/%s", src_path, ht_get(WATCH_FD_HASH_TABLE, event->wd), event->name);
                snprintf(dst_name, sizeof(dst_name), "%s%s",    dst_path, ht_get(WATCH_FD_HASH_TABLE, event->wd));
                snprintf(rel_path, sizeof(rel_path), "%s/%s",             ht_get(WATCH_FD_HASH_TABLE, event->wd), event->name);

                watch_initialization(inot_fd, src_name, rel_path);

                if ((event->mask & IN_ISDIR) || (event->mask & IN_MOVED_TO))
                {
                    if (copy_file(src_name, dst_name, COPY_TYPE) == -1)
                        continue;
                }
                else
                    syslog(LOG_INFO, "File \"%s\" in \"%s/%s\" is still empty => back up is delayed", event->name, src_path, ht_get(WATCH_FD_HASH_TABLE, event->wd));              
            }
            else if ((event->mask & IN_DELETE_SELF) || (event->mask & IN_MOVE_SELF))
            {
                syslog(LOG_INFO, "Directory \"%s%s\" was deleted (moved) => stop watching it", src_path, ht_get(WATCH_FD_HASH_TABLE, event->wd));

                int remove_state = ht_remove(WATCH_FD_HASH_TABLE, event->wd);
                if (remove_state == -1)
                {
                    syslog(LOG_ERR, "ht_remove() error");
                    continue;
                }
            }
            else if (event->mask & IN_MODIFY)
            {
                if (!(event->mask & IN_ISDIR))
                    syslog(LOG_INFO, "File \"%s\" was modified in \"%s%s\" => copy it", event->name, src_path, ht_get(WATCH_FD_HASH_TABLE, event->wd));
                else
                    continue;

                snprintf(src_name, sizeof(src_name), "%s%s/%s", src_path, ht_get(WATCH_FD_HASH_TABLE, event->wd), event->name);
                snprintf(dst_name, sizeof(dst_name), "%s%s",    dst_path, ht_get(WATCH_FD_HASH_TABLE, event->wd));

                if (copy_file(src_name, dst_name, COPY_TYPE) == -1)
                    continue;
            }
        }
    }

    return 0;
}

int watch_off(int inot_fd)
{
    list_entry_t *list_entry = WATCH_FD_HASH_TABLE->start;

    while (list_entry)
    {
        inotify_rm_watch(inot_fd, list_entry->hash_table_entry->key);
        list_entry = list_entry->next;
    }

    int return_status = ht_clear(WATCH_FD_HASH_TABLE);
    syslog(LOG_INFO, "Inotify watch was succcessfully offed");

    return return_status;
}