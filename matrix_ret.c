#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include <glpk.h>

#include "list.h"

#include "matrix.h"

extern pthread_mutex_t  mumu;
extern pthread_mutex_t  best_m;

glp_prob *build_model(_instance *ins){
    int i;

    glp_prob *lp;

    lp = glp_create_prob();

    glp_set_prob_name(lp, "nothing");
    glp_set_obj_dir(lp, GLP_MAX);
    
    glp_add_rows(lp, M);
    glp_add_cols(lp, N);

    for ( i = 0 ; i < M ; i++ ){    // Set the constrains
        glp_set_row_bnds(lp, i + 1, GLP_UP, 0.0, ins->b[i]);
    }

    for ( i = 0 ; i < N ; i++ ){    // Set both variables range and obj function
        if ( ins->x_lb[i] == ins->x_ub[i] ){
            glp_set_col_bnds(lp, i + 1, GLP_FX, ins->x_lb[i], ins->x_ub[i]);
        } else {
            glp_set_col_bnds(lp, i + 1, GLP_DB, ins->x_lb[i], ins->x_ub[i]);
        }
        glp_set_obj_coef(lp, i + 1, ins->c[i]);
    }

    glp_load_matrix(lp, M*N, ins->ia, ins->ja, ins->ar);

    return lp;
}

double solve_model(_instance *ins, glp_prob *lp){
    int i;

    glp_smcp param;
    glp_init_smcp(&param);
    param.msg_lev  = GLP_MSG_OFF;
    param.meth     = GLP_DUALP;
    param.presolve = GLP_ON;

    glp_simplex(lp, &param);

    ins->obj = glp_get_obj_val(lp);

    for ( i = 0 ; i < N ; i++ ){
        ins->x[i] = glp_get_col_prim(lp, i + 1);
    }

    return ins->obj;
}

int is_solved(_instance *a){
    int i;

    for ( i = 0 ; i < N ; i++ ){
        if (is_int(a->x[i]) == 0){
            return 0;
        }
    }

    return 1;
}

_instance *branch_down(_instance *ins, int pos){
    glp_prob *aux = NULL;
    _instance *new = (_instance*) malloc ( sizeof(_instance) );
    memcpy(new, ins, sizeof(_instance));

    new->x_ub[pos] = floor(new->x[pos]);

    aux = build_model(new);

    solve_model(new, aux);

    glp_delete_prob(aux);

    return new;
}

_instance *branch_up(_instance *ins, int pos){
    glp_prob *aux = NULL;
    _instance *new = (_instance*) malloc ( sizeof(_instance) );
    memcpy(new, ins, sizeof(_instance));

    new->x_lb[pos] = ceil(new->x[pos]);

    aux = build_model(new);

    solve_model(new, aux);

    glp_delete_prob(aux);

    return new;
}

void free_instance(_instance **a){
    if ( *a != NULL ) {
        free(*a);
    }

    *a = NULL;

    return;
}

void print_obj(_instance *ins){
    printf("|  Obj: %.1lf\n", ins->obj);

    return;
}

void print_instance(_instance *ins){
    /*return;*/
    int j;

    //puts("");
    //puts("+-------------------------");

    printf("|  Obj: %.1lf\n", ins->obj);
    //printf("|\n|  ");

    for ( j = 0 ; j < N ; j++ ){
        if ( is_int(ins->x[j]) == 0) {
            /*printf("%.1f ", ins->x[j]);*/
            printf("x%d = %.1f ", j, ins->x[j]);
        }
    }

    //puts("");
    //puts("+-------------------------");

    return;
}

int save_the_best(_instance** best, _instance** candidate){
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

void branch(_list* queue, _instance** ins, _instance** best){
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
                if ( ((*best) != NULL && aux->obj > (*best)->obj) || (best == NULL) ){
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

void bound(_list *h, _instance *best){
    _list *aux;
    _list *old;

    if ( best == NULL ) {
        return;
    }

    for ( aux = h->next ; aux != NULL && aux->ins->obj > best->obj ; aux = aux->next );

    if ( aux != NULL ){
        old       = aux->next;
        aux->next = NULL;
        aux       = old;
    
        while ( aux != NULL ){
            free_instance(&aux->ins);
            old = aux->next;
            free(aux);
            aux = old;
        }
    }

    return;
}

