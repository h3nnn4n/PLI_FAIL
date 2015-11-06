#include <pthread.h>
#include "list.h"
#include "matrix.h"
#include "threads.h"

extern pthread_t       *workers;
extern pthread_mutex_t *mutex_giver;
extern pthread_mutex_t *mutex_taker;
extern pthread_mutex_t  mumu;
extern pthread_mutex_t  best_m;
extern _list           *queue;
extern _instance       *best;

void number_crusher(){
    int flag;

    while ( 1 ){
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
    }

    return ;
}

