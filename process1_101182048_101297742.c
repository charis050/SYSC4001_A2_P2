#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <sys/ipc.h>

#include "process_101182048_101297742.h"

shared_data *createSharedMem()
{
    // remove name of shared memory object if it exists, no error occurs if not
    shm_unlink(NAME);

    // create the shared memory object
    int shm_fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        fprintf(stderr, "Failed to create the shared memory object!\n");
        return (shared_data *)-1;
    }

    // configure the size of the shared memory object
    int truncate_mem = ftruncate(shm_fd, SIZE);
    if (truncate_mem == -1)
    {
        fprintf(stderr, "Failed to configure the size of the shared memory object!\n");
        return (shared_data *)-1;
    }

    // map the shared memory object
    shared_data *mem_ptr = (shared_data *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mem_ptr == MAP_FAILED)
    {
        fprintf(stderr, "Failed to map the shared memory object!\n");
        return (shared_data *)-1;
    }

    // return the shared memory mapping pointer
    return mem_ptr;
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0); // disable buffering to print immediately w/o delay

    int multiple_value;

    // read from the command to get the multiple value, if not value provided, set to default 3
    if (argc > 1) // user supplied an argument
    {
        multiple_value = atoi(argv[1]); // convert string to int
    }
    else
    {
        multiple_value = 3; // default value if no argument
    }

    int shared_counter = 0;

    // create a semaphore set with SEM_KEY (1234) that contains 1 semaphore
    int sem_id = semget(SEM_KEY, 1, 0666 | IPC_CREAT); // <-- ensure it gets created
    if (sem_id == -1)
    {
        printf("Failed to create the semaphore set in Process 1\n");
        exit(1);
    }
    // set semaphore to 1 which means unlocked
    int semaphore_init = semctl(sem_id, 0, SETVAL, 1);
    if (semaphore_init == -1)
    {
        printf("Failed to initialize the semaphore in Process 1");
        exit(1);
    }

    // create pointer to shared memory object, exit the program with failure if it returned '-1' from createSharedMem()
    shared_data *ptr_to_mem = createSharedMem();
    if (ptr_to_mem == (shared_data *)-1)
    {
        exit(1);
    }

    // initialize shared values
    ptr_to_mem->value_of_multiple = multiple_value;
    ptr_to_mem->value_of_counter = shared_counter;

    pid_t pid_p2 = fork(); // create child process - Process 2
    printf("Created child process - Process 2 with pid: %d\n", pid_p2);

    if (pid_p2 < 0)
    {
        printf("Failed to create Process 2");
        exit(1); // exit with error
    }
    else if (pid_p2 == 0)
    {
        // wait until shared counter reaches 100, then start process 2 (p2)
        while (1)
        {
            if (ptr_to_mem->value_of_counter >= 100)
            {
                int execReturn = execl("./process2", "process2", (char *)NULL);
                if (execReturn == -1)
                {
                    fprintf(stderr, "Failed to begin process 2, exiting now!");
                    exit(1);
                }
            }
            usleep(10000); // delay each print out of 0.01 seconds for easier readability of program execution
        }
    }
    // execute parent process
    else
    {
        // increment shared counter until 500 and write value to shared memory, once it reaches 500, exit program
        while (1)
        {
            // wait on the semaphore until it is available then take it
            waitSemaphore(sem_id);

            print_process_count(1, shared_counter, shared_counter, multiple_value); // print the process count and if it's divisible by the supplied command line arg, or 3 by default
            shared_counter++;                                                       // increment shared counter by 1 each loop

            // store values in shared memory struct
            ptr_to_mem->value_of_multiple = multiple_value;
            ptr_to_mem->value_of_counter = shared_counter;

            // signal the semaphore to release it so another process can take it
            signalSemaphore(sem_id);

            // once shared counter reaches 500, break out of the loop and proceed
            if (shared_counter >= 500)
                break;

            usleep(10000); // delay each print out of 0.01 seconds for easier readability of program execution
        }

        // wait until Process 2 (p2) finishes running, then exit Process 1 too
        int status;
        wait(&status); // wait for child Process 2 to exit
        printf("Exiting Process 1 with PID: %d\n", getpid());
        exit(0);
    }
}
