#include "errors_vh.h"
#include "biz_handler.h"

int main(int argc, char *argv[])
{
    int input_error_state = error_input_vh(argc, argv);

    if (!input_error_state)
        biz_strings(argc, argv);
    else
        error_dump_vh(stderr, input_error_state);

    return 0;
}