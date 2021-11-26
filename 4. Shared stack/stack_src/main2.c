#include "stack.h"

int main(int argc, char *argv[])
{
    int error = 0;
    key_t key = 20000;
    size_t stack_size = 10;

    stack_t* stack = attach_stack(key, stack_size);
    if (stack == NULL)
        exit(EXIT_FAILURE);

    set_wait(stack, -1, NULL);

    print_stack(stack, stdout);

    size_t value = 0;

    mark_destruct(stack); 
    detach_stack(stack);

  
    return 0;
}
