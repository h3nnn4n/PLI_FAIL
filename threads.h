#ifndef __THREADS_BNB
#define __THREADS_BNB

#include <pthread.h>

#include "list.h"
#include "matrix.h"

typedef struct {
    int         id;
    _list      *queue;
    _instance  *best;
    _instance **best_pp;
} thread_param;

void number_crusher(thread_param *);

#endif /* __THREADS_BNB */
