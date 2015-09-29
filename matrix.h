#ifndef __matrix_pli
#define __matrix_pli

#include <glpk.h>

#define M 1
#define N 10

typedef struct{
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

typedef struct __list{
    struct __list *next;
    _instance     *ins;
} _list;

_list*       list_init();
_instance*   list_pop(_list *);
void         list_insert(_list *, _instance*);

glp_prob* build_model(_instance*);
double    solve_model(_instance*, glp_prob*);

int is_int(double);
int is_solved(_instance*);

_instance* branch_up(_instance*, int);
_instance* branch_down(_instance*, int);

void free_instance(_instance*);

void print_instance(_instance*);

_instance* save_the_best(_instance* best, _instance* candidate, int *flag);

#endif
