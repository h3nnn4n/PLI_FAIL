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

    int i = 0;

    printf("Babysitter %p %d\n", p, p->pos);
    while ( i++ == 0 ){
        fprintf(stdout, " babysitter %d sending\n", pos);

        MPI_Recv(&p->ans, 1, dist_instance, pos, 0, MPI_COMM_WORLD, &status);

        occupied[pos] = 0;

        fprintf(stdout, " babysitter %d sleeping\n", pos);
        sem_wait(&safeguard[pos]);
    }

    return;
}

