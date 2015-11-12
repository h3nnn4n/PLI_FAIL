#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

extern int shm_id[NPROCESS];

void slave(int pid_n){
    char sem_name[256];
    sprintf(sem_name, "__pli_%d", pid_n);
    sem_t *semaphore = sem_open(sem_name, O_CREAT, 0600, 0);

    sem_wait(semaphore);

    _shared_instance *p = (_shared_instance*) shmat (shm_id[pid_n], (void*) 0, 0);

    printf("%d has %d \t | %.3f \n", getpid(), pid_n, (&p->p1)->obj);
    //printf("%d has %d \t |\n", getpid(), pid_n);

    //sleep(1);

    return;
}

void babysitter(_thread_param param){

    return;
}
