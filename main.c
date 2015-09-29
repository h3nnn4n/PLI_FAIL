#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glpk.h> 

#include "matrix.h"

#include "utils.h"

int main(){
    _instance *lp    = read_instance();
    _instance *best  = NULL;
    _list     *queue = list_init();
    int i;
    int j;

    i = 0;

    glp_prob *lpp = build_model(lp);

    solve_model(lp, lpp);

    list_insert(queue, lp);

    while ( i++ < 200 ){
        /*puts("--------------------------");*/
        _instance *ins = list_pop(queue);

        int flag;
        best = save_the_best(best, ins, &flag);
        if ( flag == 1){
            continue;
        } else if (flag == -1){
            break;
        }

        for ( j = 0 ; j < N ; j++ ){
            if ( is_int(ins->x[j]) == 0 ){
                list_insert(queue, branch_down(ins, j));
                list_insert(queue, branch_up  (ins, j));
                break;
            }
        }

        free_instance(ins);
    }

    puts("--------------------------");
    print_instance(best);

    return EXIT_SUCCESS;
}
