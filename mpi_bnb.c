#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <pthread.h>
#include <semaphore.h>

#include <mpi.h>

#include "mpi_bnb.h"

#include "matrix.h"

#ifndef QUANTUM_R
#define QUANTUM_R 10000
#endif

extern int             *occupied;
extern sem_t           *safeguard;
extern sem_t           *safeguard_best;
extern pthread_mutex_t *bcaster;
extern pthread_mutex_t *bchecker;
extern MPI_Datatype     dist_instance;

int save_the_best_no_free(_instance** best, _instance* candidate){
    if (candidate == NULL){
        return -1;
    } else {
        if (is_solved(candidate) && candidate->obj > 0){
            int flag = 1;
            if ( (*best == NULL) ){
                *best = (_instance*) malloc ( sizeof(_instance) );
                memcpy(*best, candidate, sizeof(_instance));

#ifdef __output_progress
                print_obj(*best);
#endif

            } else if ( candidate->obj > (*best)->obj ) {
                memcpy(*best, candidate, sizeof(_instance));

#ifdef __output_progress
                print_obj(*best);
#endif

            } else {
                flag = 2;
            }

            return flag;
        }
    }

    return 0;
}

// Checks if the master sent a new best
void bcaster_func(_thread_param *p){
    _instance *get = (_instance*) malloc ( sizeof(_instance) );

#ifdef IPROBE
    int flag = 0;

    struct timespec slower;
    slower.tv_sec  = 0;
    slower.tv_nsec = QUANTUM_R;
#endif

    MPI_Status status;

    while ( 1 ){
#ifdef IPROBE
        //pthread_mutex_lock(bcaster);
        while ( !flag ){
            //puts("sleeping");
            clock_nanosleep(CLOCK_MONOTONIC, 0, &slower, NULL);
            //puts("waking n checking");
            MPI_Iprobe(0, 1, MPI_COMM_WORLD, &flag, &status);
            //puts("checked");
        }
        flag = 0;
        //pthread_mutex_unlock(bcaster);
#endif

        MPI_Recv(get, 1, dist_instance, 0, 1, MPI_COMM_WORLD, &status);
#ifdef __BCAST_BEST
        fprintf(stderr, "   Slave %d got a new best from master best = %.2f\n", p->pos, get->obj);
#endif

        pthread_mutex_lock(bcaster);

        int ret = save_the_best_no_free(p->aans, get);
        printf(" --------------------- %d %d %.3f\n", p->pos, ret, get->obj);

        pthread_mutex_unlock(bcaster);
    } 

    free(get);

    pthread_exit(NULL);
}

// Checks if a slave found a new best
void bchecker_func(_thread_param *p){
    _instance *get = (_instance*) malloc ( sizeof(_instance) );
    int pos = ( p->pos);

    MPI_Status status;

#ifdef IPROBE
    int flag = 0;

    struct timespec slower;
    slower.tv_sec  = 0;
    slower.tv_nsec = QUANTUM_R;
#endif

    while ( 1 ) {
#ifdef IPROBE
        //pthread_mutex_lock(&bchecker[pos]);
        while ( !flag ){
            //puts("sleeping");
            clock_nanosleep(CLOCK_MONOTONIC, 0, &slower, NULL);
            //puts("waking n checking");
            MPI_Iprobe(pos, 2, MPI_COMM_WORLD, &flag, &status);
            //puts("checked");
        }
        flag = 0;
        //pthread_mutex_unlock(&bchecker[pos]);
#endif

        if ( get == NULL ){
            puts("U w0t m8??");puts("U w0t m8??");puts("U w0t m8??");puts("U w0t m8??");puts("U w0t m8??");
        }
            
        MPI_Recv(get, 1, dist_instance, pos, 2, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&bchecker[pos]);

#ifdef __BCAST_BEST
        fprintf(stderr, "  Got a new best from slave %d = %.2f\n", p->pos, get->obj);
#endif


        int ret = save_the_best_no_free(p->aans, get);

        if ( ret == 1 ){
            p->v[pos] = 1;
        }

        pthread_mutex_unlock(&bchecker[pos]);
    }

    free(get);
    pthread_exit(NULL);
}

void babysitter(_thread_param *p){
    int pos = ( p->pos);

    MPI_Status status;

#ifdef __BABYSITTER
    printf("Babysitter %p %d\n", p, p->pos);
    fprintf(stdout, " babysitter %d reciving\n", pos);
#endif

#ifdef IPROBE
    int flag = 0;

    struct timespec slower;
    slower.tv_sec  = 0;
    slower.tv_nsec = QUANTUM_R;

    while ( !flag ){
        clock_nanosleep(CLOCK_MONOTONIC, 0, &slower, NULL);
        MPI_Iprobe(pos, 0, MPI_COMM_WORLD, &flag, &status);
    }
    flag = 0;
#endif

    MPI_Recv(p->ans, 1, dist_instance, pos, 0, MPI_COMM_WORLD, &status);

#ifdef __BABYSITTER
    fprintf(stdout, " babysitter %d got %.3f\n", pos, p->ans->obj);
#endif

    occupied[pos] = -1;

#ifdef __BABYSITTER
    fprintf(stdout, " babysitter %d sleeping\n", pos);
#endif

    pthread_exit(NULL);
}

