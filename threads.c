#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "matrix_ret.h"

#include "list.h"
#include "matrix.h"

#include "threads.h"

extern pthread_t       *workers;
extern pthread_mutex_t  mumu;
extern pthread_mutex_t  best_m;

void number_crusher(thread_param *t){
    int flag;

#ifdef __progress
    int i = 0;
#endif

    printf("Started thread %d\n", t->id);

    while ( 1 ){
        _instance *ins = list_pop(t->queue);

#ifdef __progress
        /*if ( ++i % 10 == 0 ) {*/
            printf(" thread %d -> %d %d  \t | %.3f \n", t->id, i++, list_size(t->queue), ins == NULL ? 0.0 : ins->obj);
        /*}*/
#endif

        if (ins != NULL){
            /*printf("  ->   %.3f\n", ins->obj);*/
        }

        flag = psave_the_best(t->best_pp, &ins);

        if        (flag ==  1){
            puts("found new");
            continue;
        } else if (flag ==  2){
            continue;
        } else if (flag == -1){
            puts("nononono");
            /*break;*/
            /*pthread_yield();*/
            continue;
        } else if (flag ==  0){
            pbranch(t->queue, &ins, t->best_pp);
        }

        free_instance(&ins);
    }

    return ;
}

