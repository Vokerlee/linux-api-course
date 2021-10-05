#include "rec_handler.h"

int main(int argc, char *argv[])
{
    int input_state = error_input_vh(argc, argv);
    ERR_CHECK(input_state != 0, input_state)

    return 0;
}