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
    int j;
    char sem_name[256];
    sprintf(sem_name, "__pli_slave_%d", pid_n);
    sem_t *semaphore_slave = sem_open(sem_name, O_CREAT, 0600, 0);

    sprintf(sem_name, "__pli_babysitter_%d", pid_n);
    sem_t *semaphore_babysitter = sem_open(sem_name, O_CREAT, 0600, 0);

    _instance *aux = NULL;

    sem_wait(semaphore_slave);

    printf("Starting %d\n", pid_n);

    _shared_instance *p = (_shared_instance*) shmat (shm_id[pid_n], (void*) 0, 0);

    while ( 1 ) {

        printf("%d has %d \t | %.3f \n", getpid(), pid_n, (&p->p1)->obj);

        for ( j = 0 ; j < N ; j++ ){
            if ( is_int((p->p1).x[j]) == 0 ){
                aux = branch_down(&p->p1, j);
                memcpy(&p->p1, aux, sizeof(_instance));

                aux = branch_up(&p->p1, j);
                memcpy(&p->p2, aux, sizeof(_instance));
            }
        }

        sem_post(semaphore_babysitter);
        sem_wait(semaphore_slave);
    }

    return;
}

void babysitter(_thread_param *param){
    int i;

    i = param->tid;

    pthread_barrier_wait(&go_horse);

    _shared_instance *pp;

   
    while ( 1 ) {
        _instance *ins;
        //if ( list_size(param->queue) > 0 ) {
            ins = list_pop(param->queue);

            printf(" %d %p\n", i, ins);

            if ( ins != NULL ) {
                pp = (_shared_instance*) shmat (shm_id[i], (void*) 0, 0);

                memcpy(&pp->p1, ins, sizeof(_shared_instance));

                sem_post(param->semaphores_slave[i]);
                sem_wait(param->semaphores_babysitter[i]);

                if ( (pp->p1).obj > 0.0 ) {
                    _instance *tmp;
                    tmp = &pp->p1;
                    list_insert(param->queue, &tmp);
                }

                if ( (pp->p2).obj > 0.0 ) {
                    _instance *tmp;
                    tmp = &pp->p2;
                    list_insert(param->queue, &tmp);
                }
            }
        //}
    }

    return;
}

