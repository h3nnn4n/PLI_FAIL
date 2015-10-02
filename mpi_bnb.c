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

    _instance *ans = (_instance*) malloc ( sizeof(_instance) );

    while ( 1 ){
        MPI_Recv(ans, 1, dist_instance, pos, 0, MPI_COMM_WORLD, &status);

        occupied[pos] = 0;

        sem_wait(&safeguard[pos]);
    }

    return;
}

