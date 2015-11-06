#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "list.h"

#include "matrix.h"

extern pthread_mutex_t  mumu;

_list *list_init(){
    _list *new = (_list*) malloc ( sizeof(_list) );

    new->next = NULL;
    new->ins  = NULL;

    return new;
}

void list_insert(_list *h, _instance **a){
    _instance *data = (_instance*) malloc ( sizeof(_instance) );
    _list     *new  = (_list*    ) malloc ( sizeof(_list    ) );
    _list     *aux;
    _list     *tmp;

    pthread_mutex_lock(&mumu);

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

    free_instance(a);

    pthread_mutex_unlock(&mumu);

    return;
}

_instance *list_pop(_list *h){
    _list *a;
    _instance *data = NULL;

    pthread_mutex_lock(&mumu);

    a = h->next;
    if (a != NULL){
        data = a->ins;
        h->next = a->next;
    }

    pthread_mutex_unlock(&mumu);

    return data;
}

int is_int(double x){
    return x - (int)x == 0.0 ? 1 : 0;
}

int list_size(_list *h){
    _list *a;
    int i;

    pthread_mutex_lock(&mumu);
    for ( a = h->next, i = 0 ; a != NULL ; a = a->next, i++);
    pthread_mutex_unlock(&mumu);

    return i;
}

