#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/utilities.h"
#include "../include/process_scheduling.h"
#include "../include/memory.h"

//Constants
#define PARAM_FILE "-f"
#define PARAM_ALGO "-a"
#define PARAM_MEMALLOC "-m"
#define PARAM_MEMSIZE "-s"
#define PARAM_QUANTUM "-q"

#define SIZE_INPUTFILE 1000
#define SIZE_ALGO 3
#define SIZE_MEMALLOC 3
#define SIZE_BUFFER 256
#define SIZE_PROCESSES 100

int main(int argc, char **argv) 
{
    char input_file[SIZE_INPUTFILE];
    char sched_algo[SIZE_ALGO];
    char mem_alloc[SIZE_MEMALLOC];
    int mem_size = 0;
    int quantum = 0;
    FILE *file;
    
    struct process_t *curr_process_list = NULL;
    struct process_t *incoming_processes = malloc(sizeof(struct process_t));
    int cpu_clock = 0;

    //Read input and params from CL arguments
    for (int i=1; i < argc; i++) 
    {
        //Checks if CL param is file input
        if (strcmp(argv[i], PARAM_FILE) == 0)
        {
            strncpy(input_file, argv[i+1], strlen(argv[i+1])+1);
            input_file[strlen(argv[i+1])] = '\0';
        }
        //Checks if CL param is scheduling algorithm
        else if (strcmp(argv[i], PARAM_ALGO) == 0)
        {
            strncpy(sched_algo, argv[i+1], strlen(argv[i+1])+1);
            sched_algo[strlen(argv[i+1])] = '\0';
        }
        //Checks if CL param is memory allocation
        else if (strcmp(argv[i], PARAM_MEMALLOC) == 0)
        {
            strncpy(mem_alloc, argv[i+1], strlen(argv[i+1])+1);
            mem_alloc[strlen(argv[i+1])] = '\0';
        }
        //Checks if CL param is memory size
        else if (strcmp(argv[i], PARAM_MEMSIZE) == 0)
        {
            mem_size = atoi(argv[i+1]);
        }
        //Checks if CL param is quantum
        else if (strcmp(argv[i], PARAM_QUANTUM) == 0)
        {
            quantum = atoi(argv[i+1]);
        }
        else
        {
            continue;
        }
    }

    //Reads from the stated file_input
    if ((file = fopen(input_file, "r")) == NULL)
    {
        fprintf(stderr, "Unable to open file!\n");
        exit(1);
    }

    //Init all processes into linked list for better simulation
    incoming_processes = get_all_processes(file);

    //Start CPU simulation
    while(1)
    {
        printf("CLOCK: %d\n", cpu_clock);
        //Run first process at time 0
        if (cpu_clock == 0)
        {
            curr_process_list = malloc(sizeof(struct process_t));
            
            if(!curr_process_list)
            {
                fprintf(stderr, "Malloc failed!\n");
                exit(1);
            }

            curr_process_list = list_pop(&incoming_processes);
        }

        //Checks if cpu_clock corresponds to a newly arrived process, adds to processing queue
        //if matches
        if (incoming_processes && has_process_arrived(cpu_clock, incoming_processes))
        {
            printf("HI\n");
            while(incoming_processes && cpu_clock == incoming_processes->arrival_time)
            {
                curr_process_list = list_push(curr_process_list, list_pop(&incoming_processes));
                
            }
            printf("Time: %d, running process: %d\n", cpu_clock, curr_process_list->pid);
        }
        printf("MASTER: \n");
        print_list(incoming_processes);
        printf("RUNNING: \n");
        print_list(curr_process_list);
        
        //Rearrange currently running processes based on scheduling algorithm

        //Run process
        execute_process(cpu_clock, &curr_process_list);

        //If no more processes to run, stop simulation.
        if (!incoming_processes && !curr_process_list)
        {
            break;
        }
      
        //Increment cpu clock
        cpu_clock += 1;
    }

    return 0;
}