#ifndef __babysitter_pli
#define __babysitter_pli

#include "matrix.h"

typedef struct {
    int *v;
    int pos;
    int size;
    _instance *ans;
} _thread_param;

void babysitter(_thread_param *p);

#endif
