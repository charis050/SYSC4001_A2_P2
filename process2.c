#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <sys/ipc.h>

#include "process.h"

shared_data *accessSharedMemory()
{
    // open the shared memory object created by Process 1 (p1)
    int shm_fd = shm_open(NAME, O_RDONLY, 0666);
    if (shm_fd == -1)
    {
        fprintf(stderr, "Failed to open shared memory object %s...\n", NAME);
        return (shared_data *)-1; // return error code
    }

    // Memory map the shared memory object
    shared_data *ptr = (shared_data *)mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        fprintf(stderr, "Failed to map the shared memory object %s...\n", NAME);
        return (shared_data *)-1; // return error code
    }

    // return mapping pointer
    return ptr;
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0); // disable buffering to print immediately w/o delay

    // create a semaphore set with SEM_KEY (1234) that contains 1 semaphore
    int sem_id = semget(SEM_KEY, 1, 0666);
    if (sem_id == -1)
    {
        printf("Failed to create the semaphore set in Process 2\n");
        exit(1);
    }

    // wait on the semaphore until it is available then take it
    waitSemaphore(sem_id);

    // Create pointer to shared memory object, exit the program with failure if it returns an Error Code
    shared_data *ptr_to_mem = accessSharedMemory();
    if (ptr_to_mem == (shared_data *)-1)
    {
        exit(1); // exit program with error if error occured during shared mem operations
    }

    // signal the semaphore to release it so another process can take it
    signalSemaphore(sem_id);

    // infite loop that will decrement p2_counter by 1 each loop and will print:
    // "Process 2 Cycle #: x <--> is a multiple of 3" if p2_counter is multiple of 3, OR:
    // "Process 2 Cycle: x" if p2_counter is not a multiple of 3
    while (1)
    {
        // when p2_counter reaches under 500, exit program
        if (ptr_to_mem->value_of_counter == 500)
        {
            print_process_count(2, ptr_to_mem->value_of_counter, ptr_to_mem->value_of_counter, ptr_to_mem->value_of_multiple);
            printf("Exiting Process 2 with PID: %d\n", getpid());

            // remove name of shared memory object if it exists, no error occurs if not
            shm_unlink(NAME);

            exit(0); // exit Process 2 w/o error
        }
        print_process_count(2, ptr_to_mem->value_of_counter, ptr_to_mem->value_of_counter, ptr_to_mem->value_of_multiple);

        usleep(10000); // delay each print out of 0.01 seconds for easier readability of program execution
    }
}