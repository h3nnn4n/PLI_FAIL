#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glpk.h> 

#include "matrix.h"

#include "utils.h"

#include "list.h"

#include "threads.h"

pthread_t       *workers;
pthread_mutex_t mumu;
pthread_mutex_t best_m;
thread_param    *t_param;

int main(int argc, char *argv[]){
    _instance *lp    = read_instance();
    _list           *queue;
    _instance       *best;

    int np, i;

    if ( argc == 2 ){
        np = atoi(argv[1]);
    } else {
        np = 4;
    }

    queue = list_init();
    best  = NULL;
    
    // Init mutexes and worker threads
    t_param     = (thread_param   *) malloc ( sizeof (thread_param   ) * np );
    workers     = (pthread_t      *) malloc ( sizeof (pthread_t      ) * np );

    pthread_mutex_init(&mumu, NULL);
    pthread_mutex_init(&best_m, NULL);

    // Init finished

    // Init the pool
    i = 0;

    time_t t_clock = clock();
    glp_prob *lpp = build_model(lp);

    solve_model(lp, lpp);

    list_insert(queue, &lp);
    // Finished pool init

    // Populates the queue

    int flag;

    while ( list_size(queue) != np ){
        _instance *ins = list_pop(queue);

        flag = save_the_best(&best, &ins);

        if        (flag ==  1){
            continue;
        } else if (flag ==  2){
            continue;
        } else if (flag == -1){
            break;
        } else if (flag ==  0){
            branch(queue, &ins, &best);
        }

        free_instance(&ins);
        printf("list size = %d\n", list_size(queue));
    }

    // Start the workers
    for ( i = 0 ; i < np ; i++ ){
        t_param[i].id      =  i;
        t_param[i].queue   =  queue;
        t_param[i].best    =  best;
        t_param[i].best_pp = &best;
        pthread_create(&workers[i], NULL, (void*) &number_crusher, (void*) &t_param[i]);
    }

    // Wait for the workers to finish
    for ( i = 0 ; i < np ; i++ ){
        pthread_join(workers[i], NULL);
        printf("Stoped thread %d\n", i);
    }

    puts("here");

    t_clock = clock() - t_clock;

#ifdef __output_answer
    print_instance(best);
#endif

#ifdef __output_obj
    puts("\n-----------");
    print_obj(best);
#endif

    free(best);

    // Total Time
    fprintf(stdout, " %f %f %d %f\n", (double)t_clock/CLOCKS_PER_SEC, ((double)t_clock/CLOCKS_PER_SEC)/i, i, i/(((double)t_clock/CLOCKS_PER_SEC)));

    return EXIT_SUCCESS;
}
