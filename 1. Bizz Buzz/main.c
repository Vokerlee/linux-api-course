#include "biz_error.h"
#include "biz_handler.h"

int main(int argc, char *argv[])
{
    int input_error_state = error_input(argc, argv);

    if (!input_error_state)
        biz_strings(argc, argv);
    else
        biz_error_dump(stderr, input_error_state);

    return 0;
}