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

    glp_prob *lpp = build_model(lp);

    solve_model(lp, lpp);

    list_insert(queue, lp);

    while ( 1 ){
        /*puts("--------------------------");*/
        _instance *ins = list_pop(queue);

        // Stores the best
        int flag;
        flag = save_the_best(&best, ins);
        if ( flag == 1){
            continue;
        } else if (flag == -1){
            break;
        }

        branch(queue, ins, best);

        free_instance(ins);
    }

#ifdef __output_answer
    print_instance(best);
#endif

#ifdef __output_obj
    print_obj(best);
#endif

    free(best);

    return EXIT_SUCCESS;
}
