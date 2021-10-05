#include "rec_handler.h"

extern char *file_name;

int main(int argc, char *argv[])
{
    int input_state = error_input_vh(argc, argv);
    ERR_CHECK(input_state != 0, input_state)

    printf("My pid = %d\n", getpid());

    char* file = argv[1];
    size_t fname_size = strlen(file);
    assert(fname_size > 0);

    file_name = (char*) calloc(FILENAME_SIZE, sizeof(char));
    ERR_CHECK(file_name == NULL, BAD_ALLOC)

    strncpy(file_name, file, fname_size); // copy to global variable

    signal(SIGUSR1, &usr1_handler);

    while (1) // just to exist and wait for the signal
        sleep(100);

    return 0;
}