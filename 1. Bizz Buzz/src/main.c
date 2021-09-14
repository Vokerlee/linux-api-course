#include "biz_handler.h"

int main(int argc, char *argv[])
{
    int input_error_state = error_input_vh(argc, argv);
    if (input_error_state)
        ASSERT_OK(input_error_state)
    else
        biz_strings(argc, argv);

    return 0;
}