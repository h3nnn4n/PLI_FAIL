#ifndef __list_pli
#define __list_pli

#include "matrix.h"

typedef struct __instance _instance;

typedef struct __list{
    struct __list *next;
    _instance     *ins;
} _list;


_list*       list_init();
_instance*   list_pop(_list *);
int          list_size(_list *);
void         list_insert(_list *, _instance**);

#endif // __list_pli
