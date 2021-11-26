#ifndef VOKERLEE_STACK_H_
#define VOKERLEE_STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> 
//#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define MUTEX_SEM    0
#define FULL_SEM     1
#define EMPTY_SEM    2
//#define INIT_SEM     3
//#define N_PROC_SEM   4
//#define DESTRUCT_SEM 5

#define N_STACK_SEMAPHORES 6

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun
{
      int val;                  /* value for SETVAL */
      struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
      unsigned short *array;    /* array for GETALL, SETALL */
                                /* Linux specific part: */
      struct seminfo *__buf;    /* buffer for IPC_INFO */
};
#endif

typedef struct stack_t
{
    int capacity;
    int size;

    struct timespec timeout;
    int wait_type;

    key_t stack_key;
    int shmem_id;
    int sem_id;

    void** memory;
} stack_t;

/* Attach (create if needed) shared memory stack to the process.
Returns stack_t* in case of success. Returns NULL on failure. */
stack_t* attach_stack(key_t key, int size);

/* Detaches existing stack from process. 
Operations on detached stack are not permitted since stack pointer becomes invalid. Returns 
 0 on success and -1 if error occures */
int detach_stack(stack_t* stack);

/* Marks stack to be destroed. Destruction is done after all detaches have been completed */ 
int mark_destruct(stack_t* stack);

/* Returns stack maximum size. */
int get_size(stack_t* stack);

/* Returns current stack size. */
int get_count(stack_t* stack);

/* Push val into stack. */
int push(stack_t* stack, void* val);

/* Pop val from stack into memory */
int pop(stack_t* stack, void** val);

void print_stack(stack_t* stack, FILE* fout);

/* Deletes manually    int sval = 0;
    int resop = 0; segment */
void shmdel(int key, int size);
//---------------------------------------------
/* Additional tasks */

/* Control timeout on push and pop operations in case stack is full or empty.
val == -1 Operations return immediatly, probably with errors.
val == 0  Operations wait infinitely.
val == 1  Operations wait timeout time.
*/
int set_wait(stack_t* stack, int val, struct timespec* timeout);

/* Deleting semaphore set */
void semdel(int key);


#endif // !VOKERLEE_STACK_H_
