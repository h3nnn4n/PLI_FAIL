#include <pthread.h>
#include <stdio.h>

#include <time.h>

#include "list.h"
#include "matrix.h"
#include "threads.h"

extern pthread_t       *workers;
extern pthread_mutex_t  mumu;
extern pthread_mutex_t  best_m;

void number_crusher(thread_param *t){
    int flag;

/*#ifdef __progress*/
    /*int i = 0;*/
/*#endif*/

    printf("Started thread %d\n", t->id);

    while ( 1 ){
/*#ifdef __progress*/
        /*if ( ++i % 10 == 0 ) {*/
        /*printf("%d %d\n", i++, list_size(queue));*/
            /*printf(" thread %d -> %d %d\n", t->id, i, list_size(t->queue));*/
        /*}*/
/*#endif*/

        _instance *ins = list_pop(t->queue);

        flag = save_the_best(t->best_pp, &ins);

        printf("%d\n", flag);

        if        (flag ==  1){
            puts("found new");
            continue;
        } else if (flag ==  2){
            continue;
        } else if (flag == -1){
            /*puts("nononono");*/
            /*break;*/
            /*pthread_yield();*/
            /*sleep(1);*/
            continue;
        } else if (flag ==  0){
            branch(t->queue, &ins, t->best_pp);
        }

        free_instance(&ins);
    }

    return ;
}

