#ifndef __babysitter_pli
#define __babysitter_pli

typedef struct {
    int *v;
    int pos;
    int size;
} _thread_param;

void babysitter(_thread_param *p);

#endif
