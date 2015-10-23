#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "matrix.h"
#include "utils.h"

double gettime(struct timeval start, struct timeval end){
    struct timeval temp;

    if ((end.tv_usec - start.tv_usec) < 0) {
        temp.tv_sec  = end.tv_sec - start.tv_sec - 1;
        temp.tv_usec = 1000000000 + end.tv_usec-start.tv_usec;
    } else {
        temp.tv_sec  = end.tv_sec  - start.tv_sec;
        temp.tv_usec = end.tv_usec - start.tv_usec;
    }

    return temp.tv_sec + temp.tv_usec / 1000000.0;
}

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
        lp->x_ub[i] = 1.0;
    }

    return lp;
}
