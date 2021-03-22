//
//  main.cpp
//  CS5500 Assignment 3
//
//  Created by Noah Huff on 3/22/21.
//  ID# 700715656
//  “I certify that the code in the method functions including method function
//  main of this project are entirely my own work.”

#include <iostream>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#include <stdio.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>

#define SEM_PERM 0600
#define SHM_PERM 0600
void check_sem_error(int ret_value);

int main(int argc, const char * argv[]) {
    // create semaphores
    union semun all_sem;
    int sem_id = semget(IPC_PRIVATE, 3, SEM_PERM);
    // i created three semaphores, one for full, one for empty, and the last
    // one for the mutex. Here, I assign names to the three semaphores
    // to make using them less confusing
    int sem_full = 0;
    int sem_empty = 1;
    int sem_mutex = 2;
    
    // set semaphore initial values
    int ret_value;
    int full = 0;
    int empty = 10;
    const int mutex_using = 1;
    const int mutex_release = 0;

    all_sem.val = full;
    ret_value = semctl(sem_id, sem_full, SETVAL, all_sem.val);
    check_sem_error(ret_value);
    all_sem.val = empty;
    ret_value = semctl(sem_id, sem_empty, SETVAL, all_sem.val);
    check_sem_error(ret_value);
    all_sem.val = mutex_using;
    ret_value = semctl(sem_id, sem_mutex, SETVAL, all_sem.val);
    check_sem_error(ret_value);
    
    printf("SEM_FULL %d\n", semctl(sem_id, sem_full, GETVAL, 0));
    printf("SEM_EMPTY %d\n", semctl(sem_id, sem_empty, GETVAL, 0));
    printf("SEM_MUTEX %d\n", semctl(sem_id, sem_mutex, GETVAL, 0));

    

    // create shared memory segment
    size_t shm_size = 40; // int size = 4 bytes, needed = 10, so size = 40
    int shm_id = shmget(IPC_PRIVATE, shm_size, SHM_PERM);
    
    printf("SHM_ID %d\n", shm_id);
    
    
    // creating fork
    pid_t parent_process = getpid();
    pid_t proc_id;
    proc_id = fork();
    pid_t child_process;
    if ( proc_id < 0 ) {
        fprintf(stderr, "Child Process Creation Failed");
        return 1;
    } else if ( proc_id == 0 ) {
        // child process
        child_process = getpid();
        printf("Child Process %d\n", child_process);
    } else {
        // parent process
        printf("Parent Process %d\n", parent_process);
    }
    
    
    return 0;
}

// check for errors while setting semaphore values
void check_sem_error(int ret_value) {
    if ( ret_value == -1 ) {
        perror("CHECK_SEM_ERROR");
        exit(1);
    }
}
