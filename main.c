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

    i = 0;

    glp_prob *lpp = build_model(lp);

    solve_model(lp, lpp);

    list_insert(queue, lp);

    while ( i++ < 200 ){
        /*puts("--------------------------");*/
        _instance *ins = list_pop(queue);

        int flag;
        flag = save_the_best(&best, ins);
        if ( flag == 1){
            continue;
        } else if (flag == -1){
            break;
        }

        branch(queue, ins);

        free_instance(ins);
    }

    print_instance(best);

    return EXIT_SUCCESS;
}
