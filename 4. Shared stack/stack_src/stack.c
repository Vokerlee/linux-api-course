#include "stack.h"
#include "sys_err.h"

const size_t N_STACK_SEMAPHORES = 3;

int attach_sem_set(int key)
{
    errno = 0;
    int sem_id = semget(key, N_STACK_SEMAPHORES, IPC_CREAT | IPC_EXCL | 0666); // check for semaphore set existense
    if (sem_id != -1) // successfully created sem set => set didn't exist
    {
        union semun arg;
        struct sembuf sop = {0};

        arg.val = 0;
        if (semctl(sem_id, 0, SETVAL, arg) == -1) // initialization
        {
            error_msg("semctl error in semaphores set initialization");
            semctl(key, 0, IPC_RMID);

            return -1;
        }

        if (semop(sem_id, &sop, 1) == -1) // just to change sem_otime to say other processes that it has been initialized
        {
            error_msg("semop error in semaphores set initialization");
            semctl(key, 0, IPC_RMID);

            return -1;
        }
    }
    else
    {
        const int n_tries_to_get_set = 10;
        union semun arg;
        struct semid_ds ds;

        sem_id = semget(key, 1, 0666); // retrieve ID of existing set
        if (sem_id == -1)
        {
            error_msg("semget error in semaphores set attachment");
            return -1;
        }

        // Wait until another process has called semop() for initialization

        arg.buf = &ds;
        for (size_t j = 0; j < n_tries_to_get_set; j++)
        {
            if (semctl(sem_id, 0, IPC_STAT, arg) == -1)
            {
                error_msg("semctl error");
                return -1;
            }

            if (ds.sem_otime != 0) // has semop() performed?
                break;
            
            sleep(1); // if not, wait and retry
        }

        if (ds.sem_otime == 0) // initialization has not been completed => some error has happened;
        {
            error_msg("initialazing error");
            return -1;
        }
    }

    return sem_id;
}

int delete_sem_set(int key)
{
    errno = 0;
    int sem_id = semget(key, N_STACK_SEMAPHORES, 0666);
    if (sem_id == -1)
    {
        perror("erorr in semget (delete_sem_set call)");
        return -1;
    }
        
    errno = 0;
    int error_state = semctl(sem_id, 0, IPC_RMID);
    if (error_state == -1)
    {
        perror("error in semctl (delete_sem_set call)");
        return -1;
    }
        
    return 0;
}

int delete_shmem(int key, int size)
{
    errno = 0;
    int shmem_id = shmget(key, size * sizeof(char), 0);
    if (shmem_id == -1)
    {
        perror("erorr in shmget (delete_shmem call)");
        return -1;
    }

    errno = 0;
    int error_state = shmctl(shmem_id, IPC_RMID, NULL);
    if (error_state == -1)
    {
        perror("error in shmctl (delete_shmem call)");
        return -1;
    }

    return 0;
}

stack_t* attach_stack(key_t key, int size)
{
    if (size <= 0)
        return NULL;
    if (key == -1)
        return NULL;

    stack_t* stack = (stack_t*) calloc(1, sizeof(struct stack_t)); // to return from the function
    if (stack == NULL)
        return NULL;

    int creator_status = -1;

    int sem_id = attach_sem_set(key); // attach semaphore set
    if (sem_id == -1)
        return NULL;

    int shmem_id = shmget(key, size * sizeof(void *), IPC_CREAT | IPC_EXCL | 0666); // try to create new shared memory
    if (shmem_id == -1) // it has already been created => just attach it
    {
        fprintf(stdout, "Stack exists, attaching the stack\n");

        shmem_id = shmget(key, size * sizeof(void *), 0666);
        if (shmem_id == -1)
        {
            perror("error in shmget while attaching stack");
            return NULL;
        }

        stack->memory = (void *) shmat(shmem_id, NULL, 0);
        if (stack->memory == (void *) -1)
        {
            perror("error in shmat while attaching stack");
            return NULL;
        }

        stack->capacity = *((int *) (stack->memory));
        stack->size     = *((int *) (stack->memory + sizeof(void*)));
    }
    else
    {
        fprintf(stdout, "No stack exists, creating new stack\n");
        
        shmem_id = shmget(key, size * sizeof(char), 0666);
        if (shmem_id == -1)
        {
            perror("error in shmget while creating stack");
            delete_sem_set(key);

            return NULL;
        }

        stack->memory = shmat(shmem_id, NULL, 0);
        if (stack->memory == (void *) -1)
        {
            perror("error in shmat while creating stack (its memory)");
            delete_sem_set(key);
            delete_shmem(key, size);

            return NULL;
        }

        int max_size = size;
        int initial_size = 0;

        int res_op1 = semctl(sem_id, 0, SETVAL, 1);
        int res_op2 = semctl(sem_id, 1, SETVAL, max_size);
        int res_op3 = semctl(sem_id, 2, SETVAL, 0);

        if (res_op1 == -1 || res_op2 == -1 || res_op3 == -1)
        {
            perror("error in semctl while setting initial values to semaphores set");
            delete_sem_set(key);
            delete_shmem(key, size);

            return NULL;
        }
            
        memcpy((void*) stack->memory, &max_size, sizeof(int));
        memcpy((void*) stack->memory + sizeof(void *), &initial_size, sizeof(int));

        stack->capacity = *((int *) (stack->memory));
        stack->size     = *((int *) (stack->memory + sizeof(void *)));
    }

    return stack;
}

void print_stack(stack_t* stack, FILE* fout)
{
    if (stack == NULL || fout == NULL)
        return;

    // BASE INFO
    fprintf(fout, "\n\n\nStack[%p]:\n", stack);
    fprintf(fout, "Stack max size: %d\n", stack->capacity);
    fprintf(fout, "Stack cur size: %d\n", stack->size);
    fprintf(fout, "Stack shared memory beginning: %p\n", stack->memory);

    // MEMORY INFO
    if (stack->memory)
    {
        fprintf(fout, "Stack shared memory[0]: %d\n", *((int*)  stack->memory));
        fprintf(fout, "Stack shared memory[1]: %d\n", *((int*) (stack->memory + sizeof(void*)) ));

        for (size_t i = 0; i < stack->size; ++i)
            fprintf(fout, "Stack [%d] = %d\n", i, *((int*) (stack->memory + (2 + i) * sizeof(void*) ) ));
    }
    
    fprintf(fout, "\n\n\n");
}
