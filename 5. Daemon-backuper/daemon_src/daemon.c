#include "daemon.h"
#include "backuper.h"

#define BUF_SIZE 100

enum copy_type COPY_TYPE = SHALLOW_COPY;
const char *DAEMON_NAME = "backuperd";

int become_daemon(int flags)
{
    int maxfd = 0;
    int fd    = 0;

    switch (fork())
    {
        case -1:
            perror("fork error");
            return -1;
        case 0:
            break;
        default:
            exit(EXIT_SUCCESS);
    }

    if (setsid() == -1) // become leader of new session 
        return -1;
    
    switch (fork())
    { // ensure we are not session leader
        case -1:
            perror("fork error");
            return -1;
        case 0:
            break;
        default:
            exit(EXIT_SUCCESS);
    }

    if (!(flags & BD_NO_UMASK0))    
        umask(0); // clear file mode creation mask

    if (!(flags & BD_NO_CHDIR)) 
    {
        errno = 0;
        int error = chdir("/"); // change to root directory
        if (error == -1)
        {
            perror("chdir() error");
            return -1;
        }
    }

    if (!(flags & BD_NO_CLOSE_FILES))
    { // close all open files
        maxfd = sysconf(_SC_OPEN_MAX);

        if (maxfd == -1) // limit is indeterminate...
            maxfd = BD_MAX_CLOSE; // so take a guess
        for (fd = 0; fd < maxfd; fd++)
            close(fd);
    }

    if (!(flags & BD_NO_REOPEN_STD_FDS))
    {
        close(STDIN_FILENO); // reopen standard fd's to /dev/null

        errno = 0;
        fd = open("/dev/null", O_RDWR);
        if (fd != STDIN_FILENO) // 'fd' should be 0
        {
            perror("open /dev/null error");
            return -1;
        }
        if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
        {
            perror("dup2 error");
            return -1;
        }
        if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
        {
            perror("dup2 error");
            return -1;
        }
    }

    return 0;
}

int create_unique_pid_file(const char *prog_name, const char *pid_file, int flags)
{
    assert(prog_name);
    assert(pid_file);

    char buf[BUF_SIZE] = {0};

    int fd = open(pid_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        syslog(LOG_ERR, "Error while opening PID file %s", pid_file);
        return -1;
    }

    if (flags & CPF_CLOEXEC)
    {
        // Set the close-on-exec file descriptor flag
        flags = fcntl(fd, F_GETFD); // fetch flags
        if (flags == -1)
        {
            syslog(LOG_ERR, "Error while getting flags of PID file %s", pid_file);
            return -1;
        }

        flags |= FD_CLOEXEC; // turn on FD_CLOEXEC
        if (fcntl(fd, F_SETFD, flags) == -1) // update flags
        {
            syslog(LOG_ERR, "Error while updating flags of PID file %s", pid_file);
            return -1;
        }
    }

    if (set_lock(fd, F_WRLCK, SEEK_SET, 0, 0) == -1)
    {
        if (errno == EAGAIN || errno == EACCES)
            syslog(LOG_ERR, "Error: daemon %s is already launched", prog_name);
        else
            syslog(LOG_ERR, "Errorwhile creating PID file %s", pid_file);

        return -1;
    }

    if (ftruncate(fd, 0) == -1)
    {
        syslog(LOG_ERR, "Error while truncating PID file %s", pid_file);
        return -1;
    }
    
    snprintf(buf, BUF_SIZE, "%ld\n", (long) getpid());
    
    if (write(fd, buf, strlen(buf)) != strlen(buf))
    {
        syslog(LOG_ERR, "Error while recording %s PID to PID file %s", prog_name, pid_file);
        return -1;
    }

    return fd;
}

static int lock_ctl(int fd, int cmd, int type, int whence, int start, off_t len)
{
    struct flock fl;

    fl.l_type   = type;
    fl.l_whence = whence;
    fl.l_start  = start;
    fl.l_len    = len;

    return fcntl(fd, cmd, &fl);
}

int set_lock(int fd, int type, int whence, int start, int len)
{
    return lock_ctl(fd, F_SETLK, type, whence, start, len);
}

int set_signals(sigset_t *waitset)
{
    assert(waitset);

    sigemptyset(waitset);
    sigaddset(waitset, SIGALRM);
    sigaddset(waitset, SIGUSR1);
    sigaddset(waitset, SIGUSR2);
    sigaddset(waitset, SIGINT);
    sigaddset(waitset, SIGTERM);
    sigaddset(waitset, SIGQUIT);
    
    int error_state = sigprocmask(SIG_BLOCK, waitset, NULL);
    if (error_state == -1)
    {
        syslog(LOG_ERR, "Error while setting signals handlers");
        return -1;
    }

    return 0;
}


