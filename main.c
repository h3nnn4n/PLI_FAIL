#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glpk.h> 

#include "matrix.h"

#include "utils.h"

#include "list.h"

int main(){
    _instance *lp    = read_instance();
    _instance *best  = NULL;
    _list     *queue = list_init();
    int i = 0;

    time_t t_clock = clock();
    glp_prob *lpp = build_model(lp);

    solve_model(lp, lpp);

    list_insert(queue, lp);

    while ( 1 ){
        if ( ++i % 100 == 0 ) {
#ifdef __progress
            printf("%d %d\n", i, list_size(queue));
#endif
        }

        _instance *ins = list_pop(queue);

        // Stores the best
        int flag;
        flag = save_the_best(&best, ins);
        if        (flag ==  1){
            continue;
        } else if (flag ==  2){
            continue;
        } else if (flag == -1){
            break;
        } else if (flag ==  0){
            branch(queue, ins, best);
        }

        free_instance(ins);
    }

    t_clock = clock() - t_clock;

#ifdef __output_answer
    print_instance(best);
#endif

#ifdef __output_obj
    puts("-----------");
    print_obj(best);
#endif

    free(best);

    // Total Time
    fprintf(stdout, " %f %f %d\n", (double)t_clock/CLOCKS_PER_SEC, ((double)t_clock/CLOCKS_PER_SEC)/i, i );

    return EXIT_SUCCESS;
}
