#include "stack.h"

int main(int argc, char *argv[])
{
    int error = 0;
    key_t key = 20000;
    size_t stack_size = 10;

    pid_t pid = getpid();

    for (int i = 0; i < 0; i++)
    {
        if (fork() == -1)
        {
            perror("fork() error");
            exit(EXIT_FAILURE);
        }
    }

    stack_t* stack = attach_stack(key, stack_size);
    if (stack == NULL)
        exit(EXIT_FAILURE);

    size_t value = 21;

    push(stack, (void *) value);
    push(stack, (void *) value);
    push(stack, (void *) value);
    pop(stack, (void **) &value);
    pop(stack, (void **) &value);
    pop(stack, (void **) &value);
    
    // if (pid == getpid()) {
    //     sleep(2);
    //     printf("count %d\n", get_count(stack));

    //     mark_destruct(stack);
    // }

    detach_stack(stack);

    return 0;
}
