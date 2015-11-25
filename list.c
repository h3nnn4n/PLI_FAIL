#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "list.h"

#include "matrix.h"

extern  pthread_mutex_t    *list_mutex;

_list *list_init(){
    _list *new = (_list*) malloc ( sizeof(_list) );

    new->next = NULL;
    new->ins  = NULL;

    return new;
}

void list_insert(_list *h, _instance **a){
    pthread_mutex_lock(list_mutex);

    _instance *data = (_instance*) malloc ( sizeof(_instance) );
    _list     *new  = (_list*    ) malloc ( sizeof(_list    ) );
    _list     *aux;
    _list     *tmp;

    memcpy(data, *a, sizeof(_instance));

    if ( h->next == NULL  ){
        h->next   = new;
        new->ins  = data;
        new->next = NULL;
    } else if ( h->next->ins->obj < data->obj ){
        tmp       = h->next;
        h->next = new;
        new->ins  = data;
        new->next = tmp;
    } else {
        aux = h->next;

        while ( aux->next != NULL && data->obj < aux->next->ins->obj){
            aux = aux->next;
        }

        tmp       = aux->next;
        aux->next = new;
        new->ins  = data;
        new->next = tmp;
    }

    pthread_mutex_unlock(list_mutex);

    //free_instance(a);

    return;
}

_instance *list_pop(_list *h){
gambi:

    pthread_mutex_lock(list_mutex);

    _list *a;
    _instance *data = NULL;

    a = h->next;
    if (a != NULL){
        data = a->ins;
        h->next = a->next;
    }

    pthread_mutex_unlock(list_mutex);

    if ( data == NULL ) {
        goto gambi;
    }

    return data;
}

int is_int(double x){
    return x - (int)x == 0.0 ? 1 : 0;
}

int list_size(_list *h){
    _list *a;
    int i;

    pthread_mutex_lock(list_mutex);
    for ( a = h->next, i = 0 ; a != NULL ; a = a->next, i++);
    pthread_mutex_unlock(list_mutex);

    return i;
}

