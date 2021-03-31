//
//  main.cpp
//  CS5500 Assignment 3 
//
//  Created by Noah Huff
//  UCM ID# 700715656
//  “I certify that the code in the method functions including method function
//  main of this project are entirely my own work.”

#include <iostream>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHM_PERM 0600

int main(int argc, const char * argv[]) {
    //shared memory
    //I added one more slot to hold a value for the offset.
    //This way, I can implement a LIFO stack for the consumer and producer
    size_t shm_size = 11;
    int shm_id = shmget(IPC_PRIVATE, shm_size, SHM_PERM | IPC_CREAT | IPC_EXCL);
    if ( shm_id < 0 ) {
        printf("Failed to create shared memory...\n");
        exit(1);
    }
    // printf("SHM_ID %d\n", shm_id);
    // semaphores with their initial values
    sem_t *mutex = sem_open("mutex", O_CREAT | O_EXCL, 0, 1);
    sem_t *full = sem_open("full", O_CREAT | O_EXCL, 0, 0);
    sem_t *empty = sem_open("empty", O_CREAT | O_EXCL, 0, 10);
    if ( empty == SEM_FAILED || mutex == SEM_FAILED || full == SEM_FAILED) {
        printf("Failed to create the semaphores...\n");
        exit(1);
    }
    // unlinking the semaphores
    // this will destroy the semaphores when all the processes
    // that have it open close it
    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("mutex");
    srand(time_t(NULL));
    int consumer = fork();
    int parent = getpid();
    int child;
    if ( consumer < 0 ) {
        printf("CONSUMER FORK FAILED.");
    } else if ( consumer == 0 ) {
        child = getpid();
        // -------------------------------------
        // -------------------------------------
        // --------------CONSUMER---------------
        // -------------------------------------
        // -------------------------------------
        sem_wait(mutex);
        printf("CONSUMER PID: %d\n", child);
        sem_post(mutex);
        int * cons_buf;
        cons_buf = (int*)shmat(shm_id, (int*)0, 0);
        int * start_point = cons_buf;
        int total = 0;
        int offset = 0;
        //the for-loop is just for consuming the items, not for waiting.
        for ( int i = 0; i < 1000; i++ ) {
            //creating a random sleep time and interval
            int sleep_time = rand() % 3;
            if ( i % 3 == sleep_time * 10) {
                printf("$$$$$$$$$$Consumer Sleeping$$$$$$$$$$\n");
                usleep(sleep_time);
            }
            sem_wait(full);
            sem_wait(mutex);
            printf("---Consumer Critical Section---\n");
            /*------------------consume items------------------*/
            // go to the first element to get the offset
            cons_buf = start_point;
            offset = *cons_buf;
            printf("Consumer Offset: %d\n", *cons_buf);
            //decrement the offset
            *cons_buf = offset - 1;
            //go to the correct memory slot
            cons_buf = (start_point + offset);
            //printf("Consumer Buffer Position: %p Value: %d\n", cons_buf, *cons_buf);
            //add item to total
            total += *cons_buf;
            printf("Current Total: %d\n", total);
            /*-------------end of comsuming items--------------*/
            printf("---Exiting Critical Section---\n");
            //the sem_post function will signal any processes waiting on the semaphore
            sem_post(mutex);
            sem_post(empty);
            // close all the semaphores
        }
        if (sem_close(mutex) == 0 && sem_close(empty) == 0 && sem_close(full) == 0) {
            printf("Consumer Process closed the semaphores.\n");
        } else {
            printf("Error closing semaphores...\n");
            exit(1);
        }
        exit(0);
    } else if ( getpid() == parent ) {
        // -------------------------------------
        // -------------------------------------
        // -------------PRODUCER----------------
        // -------------------------------------
        // -------------------------------------
        printf("PRODUCER PID: %d\n", parent);
        int * prod_buf;
        prod_buf = (int*)shmat(shm_id, (int*)0, 0);
        int * start_point = prod_buf;
        int offset = 0;
        int item_produced = 1;
        for( int i = 0; i < 1000; i++ ) {
            //creating a random sleep time and interval
            int sleep_time = rand() % 3;
            if ( i % 2 == sleep_time * 700) {
                printf("**********Producer Sleeping**********\n");
                usleep(sleep_time);
            }
            sem_wait(empty);
            sem_wait(mutex);
            printf("+++Producer Critical Section+++\n");
            /*---------------produce items----------------*/
            // go to the first element to get the offset
            prod_buf = start_point;
            offset = *prod_buf;
            //increment the offset
            *prod_buf = offset + 1;
            printf("Number Of Items In The Buffer: %d\n", *prod_buf);
            //go to the correct memory slot
            prod_buf = start_point + *prod_buf;
            //add item_produced in the correct location
            *prod_buf = item_produced;
            //printf("Producer Buffer Position: %p Value: %d\n", prod_buf, *prod_buf);
            //increment the count for the next item
            item_produced++;
            /*-----------end of producing items-------------*/
            printf("+++Exiting Critical Section+++\n");
            sem_post(mutex);
            sem_post(full);
        }
        if (sem_close(mutex) == 0 && sem_close(empty) == 0 && sem_close(full) == 0) {
            printf("Producer Process closed the semaphores.\n");
        } else {
            printf("Error closing semaphores...\n");
            exit(1);
        }
    }
    if(consumer > 0)
    {
        int status;
        printf("\nProducer Finished, Waiting For The Consumer\n\n");
        wait(&status);
    }
    parent = wait(&child);
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}
