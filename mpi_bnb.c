#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

#include <mpi.h>

#include "mpi_bnb.h"

#include "matrix.h"

extern int             *occupied;
extern sem_t           *safeguard;
extern sem_t           *safeguard_best;
extern pthread_mutex_t *bcaster;
extern pthread_mutex_t *bchecker;
extern MPI_Datatype     dist_instance;

// Checks if the master sent a new best
void bcaster_func(_thread_param *p){
    _instance *get = (_instance*) malloc ( sizeof(_instance) );

    MPI_Status status;

    while ( 1 ){
        MPI_Recv(get, 1, dist_instance, 0, 1, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(bcaster);

        save_the_best(p->aans, get);
        bound(p->queue, *p->aans);

        pthread_mutex_unlock(bcaster);
    } 

    free(get);

    return;
}

// Checks if a slave found a new best
void bchecker_func(_thread_param *p){
    _instance *get = (_instance*) malloc ( sizeof(_instance) );
    int pos = ( p->pos);

    MPI_Status status;

    while ( 1 ) {
        MPI_Recv(get, 1, dist_instance, pos, 1, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(bchecker);

        p->v[pos] = 1;

        save_the_best(p->aans, get);

        pthread_mutex_unlock(bchecker);
    }

    free(get);
    pthread_exit(NULL);
}

void babysitter(_thread_param *p){
    int pos = ( p->pos);

    MPI_Status status;

#ifdef __BABYSITTER
    printf("Babysitter %p %d\n", p, p->pos);
#endif
    //while ( i++ == 0 ){
#ifdef __BABYSITTER
    fprintf(stdout, " babysitter %d reciving\n", pos);
#endif

    MPI_Recv(p->ans, 1, dist_instance, pos, 0, MPI_COMM_WORLD, &status);

#ifdef __BABYSITTER
    fprintf(stdout, " babysitter %d got %.3f\n", pos, p->ans->obj);
#endif

    //if ( p->ans->obj < 0 ){
        //printf(" Slave %d died\n", p->pos); 
        //occupied[pos] = -1;
        //pthread_exit(NULL);
    //}

    occupied[pos] = -1;

#ifdef __BABYSITTER
    fprintf(stdout, " babysitter %d sleeping\n", pos);
#endif
    //sem_wait(&safeguard[pos]);
    //}

    pthread_exit(NULL);
}

