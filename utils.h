#ifndef __utils
#define __utils

#include "matrix.h"

typedef struct{
    int is_valid_p1;
    int is_valid_p2;

    _instance p1;
    _instance p2;
} _shared_instance;

_instance* read_instance();

#endif
