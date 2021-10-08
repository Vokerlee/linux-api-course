#include "conv_handler.h"

int main(int argc, char *argv[])
{
    int input_error_state = error_input_vh(argc, argv);
    ERR_CHECK(input_error_state != 0, input_error_state);

    conv_handler(argv[1]);

    return 0;
}