#ifndef __forks_pli
#define __forks_pli

typedef struct{
   int pid; 
} _thread_param;

void slave(int);
void babysitter(_thread_param param);

#endif /* __forks_pli */
