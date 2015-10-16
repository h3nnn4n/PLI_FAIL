#ifndef __babysitter_pli
#define __babysitter_pli

#include "matrix.h"

#include "list.h"

typedef struct {
    int *v;
    int pos;
    int size;
    _instance  *ans;
    _instance **aans;
    _list      *queue;
} _thread_param;

void babysitter(_thread_param *);
void bcaster_func(_thread_param *);
void bchecker_func(_thread_param *);

#endif
