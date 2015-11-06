#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include <glpk.h>

#include "list.h"

#include "matrix.h"
#include "matrix_ret.h"

extern pthread_mutex_t  mumu;
extern pthread_mutex_t  best_m;

int psave_the_best(_instance** best, _instance** candidate){
    int flag = 1;

    if ((*candidate) == NULL){
        return -1;
    } else {
        pthread_mutex_lock(&best_m);
        if (is_solved((*candidate)) && (*candidate)->obj > 0){
            if ( (*best == NULL) ){
                *best = (_instance*) malloc ( sizeof(_instance) );
                memcpy(*best, (*candidate), sizeof(_instance));

#ifdef __output_progress
                puts("first");
                print_obj(*best);
#endif 
            } else if ( (*candidate)->obj > (*best)->obj ) {
                puts("not first");
                memcpy(*best, (*candidate), sizeof(_instance));

#ifdef __output_progress
                print_obj(*best);
#endif

            } else {
                flag = 2;
            }

            free_instance((candidate));
            (*candidate) = NULL;
        } else if ( (*candidate)->obj <= 0.0 ){
            flag = 2;
        }
    }
    flag = 0;

    pthread_mutex_unlock(&best_m);
    return flag;
}

void pbranch(_list* queue, _instance** ins, _instance** best){
    int j; 
    int ret = -1;

    _instance *aux = NULL;

    for ( j = 0 ; j < N ; j++ ){
       if ( is_int((*ins)->x[j]) == 0 ){

            // Bounding
            aux = branch_down(*ins, j);

            ret = save_the_best(best, ins);
            /*printf(" -> %d\n", ret);*/
            if ( ret == 0 && aux->obj > 0 ){
                pthread_mutex_lock(&best_m);
                if ( ((*best) != NULL && aux->obj > (*best)->obj) || (*best == NULL) ){
                    list_insert(queue, &aux);
                }
                pthread_mutex_unlock(&best_m);
            } else {
                free_instance(&aux);
            }

            // Bounding
            aux = branch_up(*ins, j);

            ret = save_the_best(best, ins);
            /*printf(" -> %d\n", ret);*/
            if ( ret == 0 && aux->obj > 0 ){
                pthread_mutex_lock(&best_m);
                if ( ((*best) != NULL && aux->obj > (*best)->obj) || (*best == NULL) ){
                    list_insert(queue, &aux);
                }
                pthread_mutex_unlock(&best_m);
            } else {
                free_instance(&aux);
            }

            break;
        }
    }

    return;
}
