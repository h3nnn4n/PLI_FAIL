#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"
#include "utils.h"

_instance* read_instance(){
    _instance *lp = (_instance*) malloc ( sizeof(_instance) );
    int i, j;

    for ( i = 0 ; i < M ; i++ )
        if ( ! (scanf("%d", &lp->b[i]) > 0 ) )
            exit(-1);

    for ( i = 0 ; i < N ; i++ )
        if ( ! (scanf("%d", &lp->c[i]) > 0 ) )
            exit(-1);

    for ( j = 0 ; j < M ; j++ ){
        for ( i = 0 ; i < N ; i++ ){
            if ( ! (scanf("%lf", &lp->ar[j*N + i + 1]) ) )
                exit(-1);
            lp->ia[j*N + i + 1] = j + 1;
            lp->ja[j*N + i + 1] = i + 1;
        }
    }

    for ( i = 0 ; i < N ; i++ ){
        lp->x[i]    = 0.0;
        lp->x_lb[i] = 0.0;
        lp->x_ub[i] = 4.0;
    }

    return lp;
}
