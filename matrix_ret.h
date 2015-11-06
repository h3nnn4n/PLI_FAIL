#ifndef __matrix_pli
#define __matrix_pli

#include <glpk.h>

#include "list.h"

#ifndef M
#define M 1
#endif

#ifndef N
#define N 250
#endif

typedef struct __list _list;

typedef struct __instance{
    int    ia  [(M + 1) * (N + 1)]; // "Sparse" matrix 
    int    ja  [(M + 1) * (N + 1)]; // "Sparse" matrix 
    double ar  [(M + 1) * (N + 1)]; // "Sparse" matrix 
  
    int    x_lb[N + 1];             // Variable Constraints
    int    x_ub[N + 1];             // Variable Constraints

    int    b   [M + 1];             // Constraints
    int    c   [N + 1];             // Obj function
    double x   [N + 1];             // Decision variables

    double obj;                     // Obj function value
} _instance;

glp_prob*  build_model(_instance*);
double     solve_model(_instance*, glp_prob*);

int        is_int(double);
int        is_solved(_instance*);

_instance* branch_up(_instance*, int);
_instance* branch_down(_instance*, int);
void       branch(_list*, _instance**, _instance**);

void       bound(_list*, _instance*);

void       free_instance(_instance**);

void       print_instance(_instance*);
void       print_obj(_instance*);

int        save_the_best(_instance**, _instance**);

#endif
