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

MPI_Datatype dist_instance;
sem_t        *safeguard;
sem_t        *sem_printer;
int          *occupied;

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

    // End of initialization

    if ( my_rank == 0 ) { // Then I am master ov universe
        _instance  *best  = NULL;

#ifdef __SLAVE_HAIL
        printf("%d Master reporting for duty\n", my_rank);
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

#ifdef __QUEUE_PROGRESS
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

        // Parallel processing initialization starts here
        _thread_param *params = (_thread_param* ) malloc (sizeof(_thread_param ) * np);
        pthread_t *t          = (pthread_t*     ) malloc (sizeof(pthread_t     ) * np);
        safeguard             = (sem_t*         ) malloc (sizeof(sem_t         ) * np);
        occupied              = (int*           ) malloc (sizeof(int           ) * np);

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
            printf("Building semaphore %d\n", i);
#endif

            if ( sem_init(&safeguard[i], 0, 0) != 0 ){
                fprintf(stderr, "Failed at semaphore #%d\b", i);
                exit(-1);
            }
        }

        for ( i = 1 ; i < np ; i++ ){
            occupied[i]    = 0;
        }

        for ( i = 1 ; i < np ; i++ ){
            params[i].pos  = i;
            params[i].size = np;
            params[i].v    = occupied;
            params[i].ans  = (_instance*) malloc ( sizeof(_instance) * 1);
        }

        for ( i = 1 ; i < np ; i++ ){
#ifdef __INIT_PROGRESS
            printf("Starting thread %d\n", i);
#endif

            if ( pthread_create(&t[i], NULL, (void*) &babysitter, (void*) &params[i]) != 0 ){
                fprintf(stderr, "Failed at thread #%d\b", i);
                exit(-1);
            }
        }
        
        // Initialization finished

#ifdef __INIT_PROGRESS
        puts("\nStartging main loop");
#endif

        // Main Loop
        t_main = clock(); 
        int dead = 0;
        while ( dead == 0 ){
            flag = 1;

            while ( flag ){
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

                for ( i = 1 ; i < np ; i++){
                    // Send stuff
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
        printf(":  Slave %d reporting for duty\n", my_rank);
#endif
        _instance  *get      = (_instance*) malloc ( sizeof(_instance) );
        _instance  *best     = NULL;
        _list      *queue    = list_init();

        int ww = 1;

        while ( 1 ){
            MPI_Recv(get, 1, dist_instance, 0, 0, MPI_COMM_WORLD, &status);

            list_insert(queue, get);

#ifdef __SLAVE_PROGRESS
            printf(":  Slave %d see, slave %d do. \n", my_rank, my_rank);
#endif

            while ( list_size(queue) > 0 ){
                _instance *ins = list_pop(queue);

                // Stores the best
                int flag;
                flag = save_the_best(&best, ins);
                if ( flag == 1){
                    continue;
                } else if (flag == -1){
                    break;
                } else if (flag == 0 && is_solved(ins)){
                    free_instance(ins);
                } else {

                    branch(queue, ins, best);

                    free_instance(ins);

                    if ( (ww)%10 == 0 ){
                        bound(queue, best);
                    }

#ifdef __SLAVE_PROGRESS
                    if ( (ww++)%25000 == 0 ){
                        printf("\n --> Worker %d did %d iterations. %d nodes left. Best obj is %.2f\n", my_rank, ww-1, list_size(queue), best != NULL ? best->obj : 0.0 );
                        /*printf("%d\n", ww);*/
                        _list *aa = queue->next;
                        for ( ; aa != NULL; aa = aa->next){

                            printf(" %.2f", aa->ins->obj);
                        }
                        puts("");
                    }
#endif
                }
            }

            if ( best == NULL ) {
#ifdef __SLAVE_PROGRESS
                printf("No Feasible solution Found in slave %d! Giving up...", my_rank);
#endif
                _instance *nop = (_instance*) malloc ( sizeof(_instance) );
                if ( nop == NULL ){
                    exit(-666);
                }

                nop->obj = -1;

                MPI_Send(nop, 1, dist_instance, 0, 0, MPI_COMM_WORLD);

#ifdef __SLAVE_PROGRESS
                printf("Rip...\n");
#endif
                break;
            } else {
#ifdef __SLAVE_PROGRESS
                printf("\n --> Worker %d found on the %d iteration solution %.2f\n\n", my_rank, ww, best->obj);
#endif
                MPI_Send(best, 1, dist_instance, 0, 0, MPI_COMM_WORLD);
                break;
            }
        }

        free(get);
        /*free(best);*/
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
    if ( my_rank == 0 ) {
        fprintf(stdout, "%d %d \t %f \t %f \t %f \t %f\n", my_rank, N, (double)t_total/CLOCKS_PER_SEC, (double)t_master/CLOCKS_PER_SEC, (double)t_main/CLOCKS_PER_SEC, (double)t_queue/CLOCKS_PER_SEC);
    }
    sem_post(sem_printer);

    return EXIT_SUCCESS;
}

