#include "daemon.h"
#include "backuper.h"

extern enum copy_type COPY_TYPE;
extern const char *DAEMON_NAME;

unsigned int BACKUP_PERIOD = 10; // in seconds

void launch_backuper(const char * const src_path, const char * const dst_path, const sigset_t waitset)
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
                syslog(LOG_INFO, "Backup begins right now");
                syslog(LOG_INFO, "The following backup is in %u seconds", BACKUP_PERIOD);
                break;

            case SIGINT:
            case SIGTERM:
            case SIGQUIT:
                syslog(LOG_INFO, "Backuper terminates [User decision]");
                return;

            case SIGUSR1:
            case SIGUSR2:
                syslog(LOG_INFO, "Backuper gets SIGUSR");
                break;


            default:
                syslog(LOG_ERR, "Unknown signal has been catched [SIGNAL %d]", signal);
        }
    }
    

}