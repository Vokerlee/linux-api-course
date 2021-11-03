#include "stack.h"

int main(int argc, char *argv[])
{

    stack_t* stack = attach_stack(1234, 20);
    if (stack != NULL)
    {
        printf("SUCCESS!\n");
    }

    print_stack(stack, stdout);

    return 0;
}