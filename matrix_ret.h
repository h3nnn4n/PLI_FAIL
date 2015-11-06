#ifndef __matrix_pli_ret
#define __matrix_pli_ret

#include <glpk.h>

#include "list.h"

#ifndef M
#define M 1
#endif

#ifndef N
#define N 250
#endif

typedef struct __list _list;

void       pbranch(_list*, _instance**, _instance**);

int        psave_the_best(_instance**, _instance**);

#endif
