#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

#include <mpi.h>

#include "mpi_bnb.h"

#include "matrix.h"

extern     sem_t *safeguard;
extern     MPI_Datatype dist_instance;
extern int *occupied;

void babysitter(_thread_param *p){
    int pos;

    pos  = ( p->pos);

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

