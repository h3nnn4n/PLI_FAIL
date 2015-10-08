#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <glpk.h>

#include "matrix.h"

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

_list *list_init(){
    _list *new = (_list*) malloc ( sizeof(_list) );

    new->next = NULL;
    new->ins  = NULL;

    return new;
}

void list_insert(_list *h, _instance *a){
    _instance *data = (_instance*) malloc ( sizeof(_instance) );
    _list     *new  = (_list*    ) malloc ( sizeof(_list    ) );
    _list     *aux;
    _list     *tmp;

    memcpy(data, a, sizeof(_instance));

    for ( aux = h ; aux->next != NULL && aux->ins != NULL && aux->ins->obj > data->obj ; aux = aux->next );

    tmp       = aux->next;
    aux->next = new;
    new->ins  = data;
    new->next = tmp;

    free_instance(a);

    return;
}

_instance *list_pop(_list *h){
    _list *a;
    _instance *data = NULL;

    a = h->next;
    if (a != NULL){
        data = a->ins;
        h->next = a->next;
    }

    return data;
}

int is_int(double x){
    return x - (int)x == 0.0 ? 1 : 0;
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

void free_instance(_instance *a){
    return; 

    free(a);

    a = NULL;

    return;
}

void print_obj(_instance *ins){
    printf("|  Obj: %.1lf\n", ins->obj);

    return;
}

void print_instance(_instance *ins){
    /*return;*/
    int j;

    puts("");
    puts("+-------------------------");

    printf("|  Obj: %.1lf\n", ins->obj);
    printf("|\n");

    for ( j = 0 ; j < N ; j++ ){
        printf("| x%d = %.1f\n", j, ins->x[j]);
    }

    puts("+-------------------------");

    return;
}

int save_the_best(_instance** best, _instance* candidate){
    if (candidate == NULL){
        return -1;
    } else {
        if (is_solved(candidate) && candidate->obj > 0){
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

            }

            free_instance(candidate);
            return 1;
        }
    }

    return 0;
}

void branch(_list* queue, _instance* ins, _instance* best){
    int j; 

    _instance *aux = NULL;

    for ( j = 0 ; j < N ; j++ ){
        if ( is_int(ins->x[j]) == 0 ){
            aux = branch_down(ins, j);

            // Bounding
            if ( save_the_best(&best, ins) == 0 ){
                list_insert(queue, aux);
            }

            free_instance(aux);

            aux = branch_up(ins, j);

            // Bounding
            if ( save_the_best(&best, ins) == 0 ){
                list_insert(queue, aux);
            }

            free_instance(aux);

            break;
        }
    }

    return;
}

int list_size(_list *h){
    _list *a;
    int i;

    for ( a = h->next, i = 0 ; a != NULL ; a = a->next, i++);

    return i;
}

