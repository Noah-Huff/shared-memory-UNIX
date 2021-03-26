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
void print_sem_value(int id, int semaphore);

int main(int argc, const char * argv[]) {
    // create semaphores
    int sem_id = semget(IPC_PRIVATE, 3, SEM_PERM | IPC_CREAT | IPC_EXCL);
    // i created three semaphores, one for full, one for empty, and the last
    // one for the mutex. Here, I assign names to the three semaphores
    // to make using them less confusing
    const int sem_full = 0;
    const int sem_empty = 1;
    const int sem_mutex = 2;
    
    // set semaphore initial values
    union semun all_sem;
    int ret_value;
    int full = 0;
    int empty = 10;
    const int mutex_initial = 1;
    
    all_sem.val = full;
    ret_value = semctl(sem_id, sem_full, SETVAL, all_sem.val);
    check_sem_error(ret_value);
    all_sem.val = empty;
    ret_value = semctl(sem_id, sem_empty, SETVAL, all_sem.val);
    check_sem_error(ret_value);
    all_sem.val = mutex_initial;
    ret_value = semctl(sem_id, sem_mutex, SETVAL, all_sem.val);
    check_sem_error(ret_value);
    // setting the structs for each semaphore operation
    // full semaphore actions
    struct sembuf sop_full_inc = { sem_full, +1, SEM_UNDO};
    struct sembuf sop_full_dec = { sem_full, -1, SEM_UNDO};
    // empty semaphore actions
    struct sembuf sop_empty_inc = { sem_empty, +1, SEM_UNDO};
    struct sembuf sop_empty_dec = { sem_empty, -1, SEM_UNDO};
    // mutex semaphore actions
    struct sembuf sop_mutex_up = { sem_mutex, +1, SEM_UNDO};
    struct sembuf sop_mutex_down = { sem_mutex, -1, SEM_UNDO};

    printf("SEM_FULL %d\n", semctl(sem_id, sem_full, GETVAL, 0));
    printf("SEM_EMPTY %d\n", semctl(sem_id, sem_empty, GETVAL, 0));
    printf("SEM_MUTEX %d\n", semctl(sem_id, sem_mutex, GETVAL, 0));
    printf("SEM_ID %d\n", sem_id);

    // create shared memory segment
    size_t shm_size = 10;
    int shm_id = shmget(IPC_PRIVATE, shm_size, SHM_PERM | IPC_CREAT | IPC_EXCL);
    
    printf("SHM_ID %d\n", shm_id);
    
    // using fork() to create a child process
    pid_t parent_process = getpid();
    pid_t proc_id = fork();
    pid_t child_process;
    if ( proc_id < 0 ) {
        fprintf(stderr, "Child Process Creation Failed");
        return 1;
    } else if ( proc_id == 0 ) {
        // child process
        char * child_buf;
        child_buf = (char*)shmat(shm_id, (char*)0, 0);
        child_process = getpid();
        printf("Child Process PID %d\n", child_process);
        int i = 0;
        while ( i < 10 ) {
            printf("C_INT: %d\n", *child_buf);
            child_buf+=4; // imcrement 4, the size of an integer
            i++;
        }
        exit(0);
    } else {
        // parent process
        semop(sem_id, &sop_full_inc, 1);
        printf("SOP_FULL_INC %d\n", semctl(sem_id, sem_full, GETVAL, 0));
        semop(sem_id, &sop_full_dec, 1);
        printf("SOP_FULL_DEC %d\n", semctl(sem_id, sem_full, GETVAL, 0));
        int * parent_buf;
        parent_buf = (int*)shmat(shm_id, (int*)0, 0);
        printf("Parent Process PID %d\n", parent_process);
        int j = 0;
        while ( j < 10 ) {
            *parent_buf = j;
            printf("P_INT: %d\n", *parent_buf);
            parent_buf++;
            j++;
        }
    }
    
    //semctl(sem_id, sem_full, IPC_RMID, 0);
    //semctl(sem_id, sem_empty, IPC_RMID, 0);
    //semctl(sem_id, sem_mutex, IPC_RMID, 0);
    
    //shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}

// print out the value of the semaphore
void print_sem_value(int id, int semaphore) {
    printf("Semaphore value %d\n", semctl(id, semaphore, GETVAL, 0));
}

// check for semaphore errors
void check_sem_error(int ret_value) {
    if ( ret_value == -1 ) {
        perror("CHECK_SEM_ERROR");
        int err = errno;
        printf("Error #: %d\n", err);
        exit(1);
    }
}
