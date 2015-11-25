#ifndef __forks_pli
#define __forks_pli

#include "list.h"

typedef struct{
   int      pid; 
   int      tid;
   _list   *queue;
   sem_t  **semaphores_slave;
   sem_t  **semaphores_babysitter;
} _thread_param;

void slave(int);
void babysitter(_thread_param *param);

#endif /* __forks_pli */
