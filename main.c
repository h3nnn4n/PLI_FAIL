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
int          *occupied;

int main(int argc, char *argv[]){
    MPI_Status status;
    int        err;
    int        my_rank;
    int        naoInt;
    int        np;

    //err = MPI_Init( &argc, &argv );
    err = MPI_Init_thread( &argc, &argv, MPI_THREAD_MULTIPLE, &naoInt );

    //MPI_Barrier(MPI_COMM_WORLD);

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

    if ( my_rank == 0 ) { // Then I am master ov universe
        _instance  *best  = NULL;
        printf("%d Master reporting for duty\n", my_rank);

        _list      *queue = list_init();
        _instance  *lp    = read_instance();

        int i = 0;

        glp_prob *lpp = build_model(lp);

        solve_model(lp, lpp);

        list_insert(queue, lp);

        printf(" Queue size is %d\n", list_size(queue));

        puts("Populating the queue");
        puts("");

        // Populates the queue
        while ( list_size(queue) != (np - 1) && list_size(queue) > 0 ){
            _instance *ins = list_pop(queue);

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

            /*printf(" Queue size is %d with %.2f\n", list_size(queue)+1, ins->obj);*/

            branch(queue, ins, best);

            free_instance(ins);

        }

        puts("--------------------\n");

        // TODO
        // makes it able to start with more or less instances than mpi nodes
        // Not so important for now
        if ( list_size(queue) != np - 1 ){
            printf("Solution was found during initialization!\n");
            print_obj(best);
            exit(-1);
        } else {
            printf("Starting with %d slaves and %d instances\n", np-1, list_size(queue));
        }

        // Parallel processing starts here
        _thread_param *params = (_thread_param* ) malloc (sizeof(_thread_param ) * np);
        pthread_t *t          = (pthread_t*     ) malloc (sizeof(pthread_t     ) * np);
        safeguard             = (sem_t*         ) malloc (sizeof(sem_t         ) * np);
        occupied              = (int*           ) malloc (sizeof(int           ) * np);

        if ( params != NULL && t != NULL && occupied != NULL ){
            puts("Memory allocated");
        } else {
            fprintf(stderr, "Boom!\n");
            exit(-1);
        }

        int flag;

        for ( i = 1 ; i < np ; i++ ){
            printf("Building semaphore %d\n", i);

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
            printf("Starting thread %d\n", i);

            if ( pthread_create(&t[i], NULL, (void*) &babysitter, (void*) &params[i]) != 0 ){
                fprintf(stderr, "Failed at thread #%d\b", i);
                exit(-1);
            }

        }

        puts("\nStartging main loop");

        int dead = 0;
        while ( dead == 0 ){
            flag = 1;

            while ( flag ){
                dead = 1;

                if ( np == 0 ) {
                    dead = 1;
                    break;
                }

                for ( i = 1 ; i < np ; i++){
                    /*printf("%d %d %d ded\n", i, occupied[i], np);*/
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
                            printf(" => Sending %.2f to %d\n", ins->obj, i);

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

        puts("Finishing...");

        for (i = 1; i < np; i++) {
            pthread_join(t[i], NULL);
        }

        // and ends here

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

        if ( best != NULL ) {
            print_obj(best);
        }

    } else {  // Ortherwise slave.
        printf(":  Slave %d reporting for duty\n", my_rank);
        _instance  *best   = NULL;
        _instance  *get    = (_instance*) malloc ( sizeof(_instance)    );
        _list      *queue  = list_init();

        int ww = 1;

        while ( 1 ){
            MPI_Recv(get, 1, dist_instance, 0, 0, MPI_COMM_WORLD, &status);

            list_insert(queue, get);

            printf(":  Slave %d see, slave %d do. \n", my_rank, my_rank);

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

                    if ( (ww)%50 == 0 ){
                        bound(queue, best);
                    }

                    if ( (ww++)%25000 == 0 ){
                        printf("\n --> Worker %d did %d iterations. %d nodes left. Best obj is %.2f\n", my_rank, ww-1, list_size(queue), best != NULL ? best->obj : 0.0 );
                        /*printf("%d\n", ww);*/
                        _list *aa = queue->next;
                        for ( ; aa != NULL; aa = aa->next){

                            printf(" %.2f", aa->ins->obj);
                        }
                        puts("");
                    }
                }
            }

            if ( best == NULL ) {
                printf("No Feasible solution Found in slave %d! Giving up...", my_rank);
                _instance *nop = (_instance*) malloc ( sizeof(_instance) );
                if ( nop == NULL ){
                    exit(-666);
                }

                nop->obj = -1;

                MPI_Send(nop, 1, dist_instance, 0, 0, MPI_COMM_WORLD);

                printf("Rip...\n");
                break;
            } else {
                printf("\n --> Worker %d found on the %d iteration solution %.2f\n\n", my_rank, ww, best->obj);
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

    return EXIT_SUCCESS;
}

