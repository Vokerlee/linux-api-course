#include "stack.h"

int main(int argc, char *argv[])
{

    stack_t* stack = attach_stack(1211, 20);
    if (stack != NULL)
    {
        printf("SUCCESS!\n");
    }

    int a = 7;
    push(stack, &a);

    mark_destruct(stack);
    
    detach_stack(stack);

    return 0;
}
