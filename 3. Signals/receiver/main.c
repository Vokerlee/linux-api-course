#include "rec_handler.h"

extern char *file_name;

int main(int argc, char *argv[])
{
// CHECKING FOR INPUT PARAMS' ERRORS
    errno = 0;
    int input_state = error_input_vh(argc, argv);
    ERR_CHECK(input_state != 0, input_state)

// PRINT PID FOR CONVENIENCE
    printf("My pid = %d\n", getpid());

// FILENAME STUFF
    char* file = argv[1];
    size_t fname_size = strlen(file);
    assert(fname_size > 0);

    file_name = (char*) calloc(FILENAME_SIZE, sizeof(char));
    ERR_CHECK(file_name == NULL, BAD_ALLOC)

    strncpy(file_name, file, fname_size);

// SIGNALS HANDLING SETTINGS
    struct sigaction act = {0};
    int    sig_err_state = 0;

    sigset_t set = {0}; 

    sig_err_state = sigemptyset(&set); 
    ERR_CHECK(sig_err_state == -1, errno)      

    sig_err_state = sigaddset(&set, SIGUSR1); 
    ERR_CHECK(sig_err_state == -1, errno)  
    sig_err_state = sigaddset(&set, SIGUSR2);
    ERR_CHECK(sig_err_state == -1, errno)  

    act.sa_mask    = set;
    act.sa_handler = usr1_handler;

    errno = 0;
    sig_err_state = sigaction(SIGUSR1, &act, 0);
    ERR_CHECK(sig_err_state == -1, errno)

    errno = 0;
    sig_err_state = sigaction(SIGUSR2, &act, 0);
    ERR_CHECK(sig_err_state == -1, errno)

// WAIT AND RELAX

    SLEEP // just exist and wait for some signals

    free(file_name); // even if sleep is infinite!

    return 0;
}