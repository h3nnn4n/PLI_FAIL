#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

// Errorno
#include <errno.h>

#include <glpk.h> 

#include "matrix.h"

#include "utils.h"

#include "list.h"

#include "forks.h"

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

#ifndef NPROCESS
#define NPROCESS 2
#endif 

#ifndef SHM_KEY
#define SHM_KEY 666
#endif

int shm_id[NPROCESS];

int main(){
    _instance *lp    = read_instance();
    _instance *best  = NULL;
    _list     *queue = list_init();
    int i = 0;
    char sem_name[256];
    int fpid[NPROCESS];
    sem_t            **semaphores;
    _shared_instance  *pp;
    pthread_t         *threads;
    _thread_param     *threads_param;
    int               *threads_id;

    semaphores = (sem_t**) malloc ( sizeof (sem_t*) * NPROCESS );
    if ( semaphores == NULL ) {
        handle_error("Semaphore memory init failed");
    }

    for ( i = 0 ; i < NPROCESS ; i++){ // Forking + shared mem + named semaphores
        sprintf(sem_name, "__pli_%d", i);
        semaphores[i] = sem_open(sem_name, O_CREAT, 0600, 0);

        if (semaphores[i] == SEM_FAILED){
            handle_error("Failed on sem_open");
            exit(-1);
        }

        if ((shm_id[i] = shmget(SHM_KEY + i, sizeof(_shared_instance), IPC_CREAT | 0666)) < 0) {
            handle_error("Failed on shmget");
            exit(1);
        }

        fpid[i] = fork();

        if (fpid[i] == -1){
            handle_error("Failed on fork");
        }

        if (fpid[i] == 0){ // children code;
            slave(i);
            
            exit( 1 );
            break;
        }
    }

    threads       = (pthread_t     *) malloc ( sizeof (pthread_t    ) * NPROCESS );
    threads_param = (_thread_param *) malloc ( sizeof (_thread_param) * NPROCESS );
    threads_id    = (int           *) malloc ( sizeof (int          ) * NPROCESS );

    if ( threads == NULL || threads_param == NULL || threads_id ) {
        handle_error("Allocating memory for threads failed");
    }

    for ( i = 0 ; i < NPROCESS ; i++){ // Babysitter threads
        threads_id[i] = pthread_create(&threads[i], NULL,
                               (void*) &babysitter, &threads_param[i]);         

        if (threads_id[i] != 0){
            handle_error("Thread init failed");
        }
    }

    time_t t_clock = clock();
    glp_prob *lpp = build_model(lp);

    solve_model(lp, lpp);

    list_insert(queue, &lp);

    _instance *ins = list_pop(queue);

    for ( i = 0 ; i < NPROCESS ; i++){
        pp = (_shared_instance*) shmat (shm_id[i], (void*) 0, 0);
        memcpy(&pp->p1, ins, sizeof(_shared_instance));
        sem_post(semaphores[i]);
        if ( i == 0)
            (&pp->p1)->obj = 666;
    }

    wait(NULL);
    exit(0);

    while ( 1 ){
        _instance *ins = list_pop(queue);

        if ( ++i % 100 == 0 ) {
#ifdef __progress
            printf("%d %d   \t %.3f %.3f\n", i, list_size(queue) + 1, best != NULL ? best->obj : 0.0, ins->obj);
#endif
        }

        // Stores the best
        int flag;
        flag = save_the_best(&best, &ins);
        if        (flag ==  1){
#ifdef __bound
            bound(queue, best); 
#endif
            continue;
        } else if (flag ==  2){
            continue;
        } else if (flag == -1){
            break;
        } else if (flag ==  0){
            branch(queue, &ins, &best);
        }

        free_instance(&ins);
    }

    t_clock = clock() - t_clock;

#ifdef __output_answer
    print_instance(best);
#endif

#ifdef __output_obj
    puts("\n-----------");
    print_obj(best);
#endif

    free(best);

    wait(NULL);

    // Total Time
    fprintf(stdout, " %f %f %d %f\n", (double)t_clock/CLOCKS_PER_SEC, ((double)t_clock/CLOCKS_PER_SEC)/i, i, i/(((double)t_clock/CLOCKS_PER_SEC)));

    for ( i = 0 ; i < NPROCESS ; i++){
        sprintf(sem_name, "__pli_%d", i);
        sem_unlink(sem_name);
    }

    return EXIT_SUCCESS;
}
