#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Threads magic
#include <pthread.h>

// fork() stuff
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h> 

// Shared mem
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "forks.h"

#include "matrix.h"

#include "utils.h"

#ifndef NPROCESS
#define NPROCESS 2
#endif 

extern int                 shm_id[NPROCESS];
extern pthread_barrier_t   go_horse;

void slave(int pid_n){
    char sem_name[256];
    sprintf(sem_name, "__pli_%d", pid_n);
    sem_t *semaphore = sem_open(sem_name, O_CREAT, 0600, 0);

    printf("Waiting\n");
    sem_wait(semaphore);
    printf("Starting\n");

    _shared_instance *p = (_shared_instance*) shmat (shm_id[pid_n], (void*) 0, 0);

    printf("%d has %d \t | %.3f \n", getpid(), pid_n, (&p->p1)->obj);

    return;
}

void babysitter(_thread_param *param){
    int i;

    i = param->tid;
    pthread_barrier_wait(&go_horse);

    printf("%d %p\n", i, param->semaphores[i]);

    _shared_instance *pp;

    _instance *ins;

   
    ins = list_pop(param->queue);

    pp = (_shared_instance*) shmat (shm_id[i], (void*) 0, 0);
    memcpy(&pp->p1, ins, sizeof(_shared_instance));

    sem_post(param->semaphores[i]);

    if ( i == 0 ) {
        (&pp->p1)->obj = 666;
    }

    return;
}

