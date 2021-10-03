#include "conv_handler.h"

int main(int argc, char *argv[])
{
    int input_error_state = error_input_vh(argc, argv);
    ERR_CHECK(input_error_state != 0, input_error_state);
    
    int fd = open(argv[1], O_RDONLY);
    ERR_CHECK(fd == -1, errno)

    conv_handler(fd);

    int close_err = close(fd);
    ERR_CHECK(close_err == -1, errno)

    return 0;
}