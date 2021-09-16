#include "biz_handler.h"

int main(int argc, char *argv[])
{
    int input_error_state = error_input_vh(argc, argv);
    ERR_CHECK(input_error_state, input_error_state)
    
    biz_strings(argc, argv);

    return 0;
}