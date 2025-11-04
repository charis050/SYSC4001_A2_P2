#ifndef PROCESS_H
#define PROCESS_H

#define NAME "/SHARED_MEM"

#define SIZE sizeof(shared_data)

// Sempahor Key Value
#define SEM_KEY 1234

// struct going through shared memory from Process 1 (p1) to Process 2 (p2)
typedef struct
{
    int value_of_multiple;
    int value_of_counter;
} shared_data;

void print_process_count(int process_num, int cycle, int counter, int divisor)
{
    if ((counter % divisor) == 0)
    {
        printf("Process %d Cycle #: %d <--> %d is a multiple of %d\n", process_num, cycle, counter, divisor);
    }
    else
    {
        printf("Process %d Cycle #: %d\n", process_num, cycle);
    }
}

// wait on the semaphore until it is available then take it
void waitSemaphore(int semaphoreID)
{
    struct sembuf sem_op = {0, -1, 0};
    semop(semaphoreID, &sem_op, 1);
}

// signal the semaphore to release it so another process can take it
void signalSemaphore(int semaphoreID)
{
    struct sembuf sem_op = {0, 1, 0};
    semop(semaphoreID, &sem_op, 1);
}

shared_data *createSharedMem();

#endif
