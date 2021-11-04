#include "stack.h"

int main(int argc, char *argv[])
{

    stack_t* stack = attach_stack(1211, 20);
    if (stack != NULL)
    {
        printf("SUCCESS!\n");
    }

    sleep(4);
    
    print_stack(stack, stdout);

    detach_stack(stack);
    

    return 0;
}
