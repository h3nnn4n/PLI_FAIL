#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <time.h> 
#include <pthread.h>
#include <semaphore.h>

#include <mpi.h>

#include <glpk.h> 

#include "matrix.h"

#include "mpi_bnb.h"

#include "utils.h"

#include "list.h"

MPI_Datatype dist_instance;
sem_t           *safeguard_best;
sem_t           *safeguard;
sem_t           *sem_printer;
pthread_mutex_t *bcaster;
pthread_mutex_t *bchecker;
int             *occupied;

int main(int argc, char *argv[]){
    MPI_Status status;
    int        err;
    int        my_rank;
    int        naoInt;
    int        np;

    time_t t_total = clock(); 
    time_t t_queue = clock();

    // Initialization
    err = MPI_Init_thread( &argc, &argv, MPI_THREAD_MULTIPLE, &naoInt );

    if ( err == MPI_ERR_OTHER ) {
        fprintf(stderr, "MPI_Init returned error!!\n");
        exit(-1);
    }

    MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &np      );

    MPI_Type_contiguous( sizeof(_instance), MPI_BYTE, &dist_instance);
    MPI_Type_commit(&dist_instance);

    if (np < 2){
        fprintf(stderr, "Not enought working horses!\n");
        exit(-1);
    }

    time_t t_master = clock(); 
    time_t t_main = clock(); 

    // End of MPI initialization

    if ( my_rank == 0 ) { // Then I am master ov universe
        _instance  *best  = NULL;

#ifdef __SLAVE_HAIL
        printf(" | %.6f | %d Master reporting for duty\n", (double)(clock()-t_total)/CLOCKS_PER_SEC, my_rank);
        fprintf(stdout, "%f\n", );
#endif

        // Makes the queue for the master and reads the problems to be solved
        _list      *queue = list_init();
        _instance  *lp    = read_instance();

        t_queue = clock(); 

        int i = 0;

        glp_prob *lpp = build_model(lp);

        solve_model(lp, lpp);

        list_insert(queue, lp);

#ifdef __QUEUE_PROGRESS
        printf(" Queue size is %d\n", list_size(queue));

        puts("Populating the queue");
        puts("");
#endif

        // Populates the queue
        while ( list_size(queue) != (np - 1) && list_size(queue) > 0 ){
            _instance *ins = list_pop(queue);
#ifdef __QUEUE_PROGRESS
            printf(" Queue size is %d with %.2f\n", list_size(queue), ins->obj);
#endif

            // Stores the best
            int flag;
            flag = save_the_best(&best, ins);
            if ( flag == 1){
                continue;
            } else if (flag == -1){
                puts("Boom/11111\n");
                exit(-1);
                break;
            }

            branch(queue, ins, best);

            bound(queue, best);

            free_instance(ins);
        }

        t_queue  = clock() - t_queue; 

#ifdef __QUEUE_START
        _list *aux;
        puts(  "+-------------------");
        printf("| ");
        for ( aux = queue->next ; aux != NULL ; aux = aux->next) {
            printf("%.2f ", aux->ins->obj); 
        }
        puts("\n+-------------------\n");
#endif

        // TODO
        // makes it able to start with more or less instances than mpi nodes
        // Not so important for now
        if ( list_size(queue) != np - 1 ){
#ifdef __FAIL_INIT
            printf("Solution was found during initialization!\n");
            print_obj(best);
#endif
            exit(-1);
        } else {
#ifdef __AMMOUNT_O_WORKERS
            printf("Starting with %d slaves and %d instances\n", np-1, list_size(queue));
#endif
        }

  // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
 // // // Initialization // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 

        // Parallel processing initialization starts here
        // Those are the babysitter
        _thread_param *params         = (_thread_param*   ) malloc ( sizeof(_thread_param   ) * np );
        pthread_t     *t              = (pthread_t*       ) malloc ( sizeof(pthread_t       ) * np );
                       safeguard      = (sem_t*           ) malloc ( sizeof(sem_t           ) * np );
                       occupied       = (int*             ) malloc ( sizeof(int             ) * np );

        // Those are for the bchecker
        _thread_param *params_checher = (_thread_param*   ) malloc ( sizeof(_thread_param   ) * np );
        pthread_t     *t_checker      = (pthread_t*       ) malloc ( sizeof(pthread_t       ) * np );
                       bchecker       = (pthread_mutex_t* ) malloc ( sizeof(pthread_mutex_t ) * np );
        int           *new_best       = (int*             ) malloc ( sizeof(int             ) * np );

        if ( params != NULL && t != NULL && occupied != NULL ){
#ifdef __INIT_PROGRESS
            puts("Memory allocated");
#endif
        } else {
            fprintf(stderr, "Boom!\n");
            exit(-1);
        }

        int flag;

        for ( i = 1 ; i < np ; i++ ){
#ifdef __INIT_PROGRESS
            printf("Building semaphore and mutex %d\n", i);
#endif

            // TODO
            // Check for error
            /*pthread_mutex_init(&bcaster[i], NULL);*/

            if ( pthread_mutex_init(&bchecker[i], NULL) != 0 ){
                fprintf(stderr, "Failed at semaphore #%d\b", i);
                exit(-1);
            }

            if ( sem_init(&safeguard[i], 0, 0) != 0 ){
                fprintf(stderr, "Failed at semaphore #%d\b", i);
                exit(-1);
            }
        }

        for ( i = 0 ; i < np ; i++ ){
            params[i].pos   = i;
            params[i].size  = np;
            params[i].v     = occupied;
            params[i].ans   = (_instance*) malloc ( sizeof(_instance) * 1);
            params[i].aans  = NULL;
            params[i].queue = NULL;

            params_checher[i].pos   = i;
            params_checher[i].size  = np;
            params_checher[i].v     = new_best;
            params_checher[i].ans   = NULL;
            params_checher[i].aans  = &best;
            params_checher[i].queue = NULL;

            new_best[i] = 0;
            occupied[i] = 0;
        }

        for ( i = 1 ; i < np ; i++ ){
#ifdef __INIT_PROGRESS
            printf("Starting thread %d\n", i);
#endif

            if ( pthread_create(&t_checker[i], NULL, (void*) &bchecker_func, (void*) &params_checher[i]) != 0 ){
                fprintf(stderr, "Failed at thread #%d\b", i);
                exit(-1);
            }

            if ( pthread_create(&t[i], NULL, (void*) &babysitter, (void*) &params[i]) != 0 ){
                fprintf(stderr, "Failed at thread #%d\b", i);
                exit(-1);
            }
        }
        
  // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
 // // // Initialization finished // // // // // // // // // // // // // // // // // // // // // // // // 
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 

#ifdef __INIT_PROGRESS
        puts("\nStartging main loop");
#endif

        // Main Loop
        t_main = clock(); 
        int dead = 0;
        while ( dead == 0 ){
            flag = 1;

            while ( flag ){


  // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
 // // // Checks if can die // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
                dead = 1;

                for ( i = 1 ; i < np ; i++){
                    if ( occupied[i] != -1 ) {
                        dead = 0;
                        break;
                    }
                }

                if ( dead ){
                    break;
                }
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 

                for ( i = 1 ; i < np ; i++){
                    // Send stuff

                    if ( new_best[i] == 1 ) {
                        int j;

                        for ( j = 1 ; j < np ; j++){
                            if ( j == i || best == NULL || occupied[j] == 666 ) {
                                continue;
                            }

                            pthread_mutex_lock(&bchecker[j]);

                            MPI_Send(best, 1, dist_instance, j, 1, MPI_COMM_WORLD);
#ifdef __BCAST_BEST
                            fprintf(stderr, " Master sending a new best to slave %d = %.2f\n", j, best->obj);
#endif

                            pthread_mutex_unlock(&bchecker[j]);
                        }

                        new_best[i] = 0;
                    }

                    if (occupied[i] == 0){

                        _instance *ins = list_pop(queue);

                        if ( ins != NULL ) {
#ifdef __SEND_PROGRESS
                            printf(" => Sending %.2f to %d\n", ins->obj, i);
#endif

                            MPI_Send(ins, 1, dist_instance, i, 0, MPI_COMM_WORLD);

                            occupied[i] = 1;
                            dead = 0;
                            flag = 0;
                            sem_post(&safeguard[i]);
                        }
                        break;
                    }
                }
            }
        }
        // End of main loop

#ifdef __SEND_PROGRESS
        puts("Finishing...");
#endif

        // Babysitter sincronization
        for (i = 1; i < np; i++) {
            pthread_join(t[i], NULL);
        }

        t_main = clock() - t_main; 

        for ( i = 1 ; i < np ; i++){
            if (is_solved(params[i].ans) && params[i].ans->obj > 0){
                if ( (best == NULL) ){
                    best = (_instance*) malloc ( sizeof(_instance) );
                    memcpy(best, params[i].ans, sizeof(_instance));

                } else if ( params[i].ans->obj > (best)->obj ) {
                    memcpy(best, params[i].ans, sizeof(_instance));
                }
            }
        }

#ifdef __output_answer
        if ( best != NULL ) {
            puts("\n--------------------------");
            print_obj(best);
            puts(  "--------------------------");
        }
#endif

    } else {  // Ortherwise slave.
#ifdef __SLAVE_PROGRESS
        printf(" | %.6f | :  Slave %d reporting for duty\n", (double)(clock()-t_total)/CLOCKS_PER_SEC, my_rank);
#endif
                       bcaster   = (pthread_mutex_t*) malloc ( sizeof(pthread_mutex_t)    );
        pthread_t     *bcastert  = (pthread_t*      ) malloc ( sizeof(pthread_t)          );
        _thread_param *params    = (_thread_param*  ) malloc ( sizeof(_thread_param )     );
        _instance     *get       = (_instance*      ) malloc ( sizeof(_instance)          );
        _list         *queue     = list_init();
        _instance     *best      = NULL;

        params->pos   = my_rank;
        params->size  = np;
        params->v     = NULL;
        params->ans   = NULL;
        params->aans  = &best;
        params->queue = queue;

        pthread_mutex_init(bcaster, NULL);
        pthread_create(bcastert, NULL, (void*) bcaster_func, (void*) params); 

        int ww = 1;

        while ( 1 ){
            MPI_Recv(get, 1, dist_instance, 0, 0, MPI_COMM_WORLD, &status);

            list_insert(queue, get);

#ifdef __SLAVE_PROGRESS
            printf(" | %.6f | :  Slave %d see, slave %d do. \n", (double)(clock()-t_total)/CLOCKS_PER_SEC, my_rank, my_rank);
#endif

            while ( list_size(queue) > 0 ){
                pthread_mutex_lock(bcaster);
                _instance *ins = list_pop(queue);

                int flag = -1;
                flag = save_the_best(&best, ins);        // Checks for a new best
                if ( flag == 1){                         // Found a new best
#ifdef __BCAST_BEST
                    fprintf(stderr, " | %.6f | -> Slave %d found new best, sending... %.3f\n", (double)(clock()-t_total)/CLOCKS_PER_SEC, my_rank, best->obj);
#endif

                    MPI_Send(best, 1, dist_instance, 0, 2, MPI_COMM_WORLD);
                    pthread_mutex_unlock(bcaster);

                    //continue;
                } else if (flag == 2 ){                  // nonononononononono
                    // Do nothing
                    //pthread_mutex_unlock(bcaster);
                    //continue;
                } else if (flag == -1){                  // nonononononononono
                    //pthread_mutex_unlock(bcaster);
                    //break;
                } else if (flag == 0 && is_solved(ins)){ // Found a feasible solution but it is worse than the best
                    free_instance(ins);
                    //pthread_mutex_unlock(bcaster);
                } else {                                 // Continue the brnaching

                    branch(queue, ins, best);

                    free_instance(ins);

                    if ( (ww)%2 == 0 ){                 // Bounds each 10 cycles
                        bound(queue, best);
                    }

#ifdef __SLAVE_PROGRESS
                    if ( (ww++)%10 == 0 ){
                        printf("\n | %.6f |  --> Worker %d did %d iterations. %d nodes left. Best obj is %.2f\n", (double)(clock()-t_total)/CLOCKS_PER_SEC, my_rank, ww-1, list_size(queue), best != NULL ? best->obj : 0.0 );
                        _list *aa = queue->next;
                        int cc;
                        for ( cc = 0; aa != NULL && cc<10; cc++,  aa = aa->next){
                            printf(" %.2f", aa->ins->obj);
                        }
                        puts("");
                    }
#endif
                }
                pthread_mutex_unlock(bcaster);
            }

            if ( best == NULL ) {
#ifdef __SLAVE_PROGRESS
                printf(" | %.6f | No Feasible solution Found in slave %d! Giving up...", (double)(clock()-t_total)/CLOCKS_PER_SEC, my_rank);
#endif
                _instance *nop = (_instance*) malloc ( sizeof(_instance) );
                if ( nop == NULL ){
                    exit( -666 );
                }

                nop->obj = -1;

                MPI_Send(nop, 1, dist_instance, 0, 0, MPI_COMM_WORLD);

                free(nop);

#ifdef __SLAVE_PROGRESS
                printf(" Rip...\n");
#endif
            } else {
#ifdef __SLAVE_PROGRESS
                printf("\n | %.6f |  --> Worker %d found on the %d iteration solution %.2f\n\n", (double)(clock()-t_total)/CLOCKS_PER_SEC, my_rank, ww, best->obj);
#endif
                MPI_Send(best, 1, dist_instance, 0, 0, MPI_COMM_WORLD);
            }
            break;
        }

        // TODO
        // 
        // puts("TODO! Kill mutex + thread");
        //
        // Kill mutex and thread
        //

        pthread_mutex_lock(bcaster);

        pthread_cancel(*bcastert);
        pthread_join(*bcastert, NULL);

        pthread_mutex_unlock(bcaster);

        //free(get);

        //free(best);
    }

#ifdef __output_answer
    print_instance(best);
#endif

#ifdef __output_obj
    print_obj(best);
#endif

    /*free(best);*/

    err = MPI_Finalize();

    t_total  = clock() - t_total; 
    t_master = clock() - t_master; 

    sem_printer = (sem_t*) malloc (sizeof(sem_t) * 1);

    if ( sem_init(sem_printer, 0, 1) != 0 ){
        fprintf(stderr, "Failed at print semaphore\b");
        exit(-1);
    }

    sem_wait(sem_printer);
    if ( my_rank == 0 ) {                               // PID |  size | total time                   |    master process time         |     main time                | Initialization time
        fprintf(stdout, "%d %d \t %f \t %f \t %f \t %f\n", my_rank, N, (double)t_total/CLOCKS_PER_SEC, (double)t_master/CLOCKS_PER_SEC, (double)t_main/CLOCKS_PER_SEC, (double)t_queue/CLOCKS_PER_SEC);
    }
    sem_post(sem_printer);

    return EXIT_SUCCESS;
}

