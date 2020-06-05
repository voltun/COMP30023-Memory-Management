#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/utilities.h"
#include "../include/process_scheduling.h"
#include "../include/memory.h"

//Constants
#define PARAM_FILE "-f"
#define PARAM_ALGO "-a"
#define PARAM_MEMALLOC "-m"
#define PARAM_MEMSIZE "-s"
#define PARAM_QUANTUM "-q"
#define ALGO_ROUNDROBIN "rr"
#define ALGO_FCOME_FSERVED "ff"
#define ALGO_CUSTOM "cs"
#define MEM_SWAPPING_X "p"
#define MEM_UNLIMITED "u"
#define MEM_VIRTUAL_MEM "v"
#define MEM_CUSTOM "cm"

#define SIZE_INPUTFILE 1000
#define SIZE_ALGO 3
#define SIZE_MEMALLOC 4
#define SIZE_BUFFER 256
#define SIZE_PROCESSES 100
#define SIZE_PER_MEM_PAGE 4

int main(int argc, char **argv) 
{
    char input_file[SIZE_INPUTFILE];
    char sched_algo[SIZE_ALGO];
    char *mem_alloc = NULL;
    uint32_t mem_size = 0;
    int fin_flag = 0;
    int quantum = 0, quantum_clock = 0;
    FILE *file;
    
    struct datalog_t *log = NULL;
    struct process_t *curr_process_list = NULL;
    struct process_t *incoming_processes = malloc(sizeof(struct process_t));
    struct memory_t *memory = NULL;
    uint32_t *memory_addr = NULL, *evicted_mem = NULL;
    uint32_t cpu_clock = 0, load_penalty = 0;

    log = init_datalog();
    sched_algo[0] = '\0';
    mem_alloc = malloc(sizeof(char) * SIZE_MEMALLOC);

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
            sscanf(argv[i+1], "%"SCNd32, &mem_size);
        }
        //Checks if CL param is quantum
        else if (strcmp(argv[i], PARAM_QUANTUM) == 0)
        {
            quantum = atoi(argv[i+1]);
            quantum_clock = quantum;
        }
        else
        {
            continue;
        }
    }

    memory = init_memory(mem_size, SIZE_PROCESSES);
    

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
        // printf("CLOCK: %d\n", cpu_clock);

        //If a process finished running, print RUNNING transcript now
        if (fin_flag && curr_process_list)
        {
            struct process_t *junk;
            curr_process_list->time_finished = cpu_clock;
            
            //Handles memory eviction if not in unlimited memory mode
            if (strcmp(mem_alloc, MEM_UNLIMITED) != 0)
            {
                evicted_mem = create_uint32_array(memory->n_total_pages, UINT32_MAX);
                evicted_mem = evict_from_memory(&memory, curr_process_list->pid);      
                print_memory_evict(cpu_clock, evicted_mem, memory->n_total_pages);
                free(evicted_mem);          
            }
            print_process_finish(cpu_clock, curr_process_list);
            junk = list_pop(&curr_process_list);
            //For performance statistics
            add_fin_process(log, junk);
            
            //If no more processes to run, stop simulation.
            if (!incoming_processes && !curr_process_list)
            {

                print_performance_stats(cpu_clock, log);
                break;
            }
            
            //Loads memory and calculate loading time penalty if not in Unlimited
            //Memory mode
            if (strcmp(mem_alloc, MEM_UNLIMITED) != 0)
            {
                memory_addr = create_uint32_array(memory->n_total_pages, UINT32_MAX);
                load_penalty = load_into_memory_p(&memory, curr_process_list->pid, curr_process_list->memory_required, 
                memory_addr, cpu_clock);
                curr_process_list->time_load_penalty = load_penalty;
                curr_process_list->memory_address = memory_addr;
            }

            print_process_run(cpu_clock, mem_alloc, curr_process_list->time_load_penalty, memory->mem_usage,
             memory->n_total_pages, curr_process_list);

            
            fin_flag = 0; 
            quantum_clock = quantum;        
        }

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
            
            //To sort pid if at time 0 has > 1 processes arriving
            while(incoming_processes && cpu_clock == incoming_processes->arrival_time)
            {
                curr_process_list = list_push(curr_process_list, list_pop(&incoming_processes));              
            }

            //Loads memory and calculate loading time penalty if not in Unlimited
            //Memory mode
            if (strcmp(mem_alloc, MEM_UNLIMITED) != 0)
            {
                memory_addr = create_uint32_array(memory->n_total_pages, UINT32_MAX);
                load_penalty = load_into_memory_p(&memory, curr_process_list->pid, curr_process_list->memory_required,
                 memory_addr, cpu_clock);
                curr_process_list->time_load_penalty = load_penalty;
                curr_process_list->memory_address = memory_addr;
            }
            print_process_run(cpu_clock, mem_alloc, curr_process_list->time_load_penalty, memory->mem_usage,
             memory->n_total_pages, curr_process_list);
        }
          
        //Checks if cpu_clock corresponds to a newly arrived process, adds to processing queue
        //if matches
        if (incoming_processes && has_process_arrived(cpu_clock, incoming_processes))
        {
            while(incoming_processes && cpu_clock == incoming_processes->arrival_time)
            {
                struct process_t *popped_proc = list_pop(&incoming_processes);
                curr_process_list = list_push(curr_process_list, popped_proc);
            }
        }
        
        // printf("MASTER: \n");
        // print_list(incoming_processes);
        // printf("RUNNING: \n");
        // print_list(curr_process_list);
        
        //Rearrange currently running processes based on scheduling algorithm

        //ROUND ROBIN SCHEDULING
        if (strcmp(sched_algo, ALGO_ROUNDROBIN) == 0 && curr_process_list->time_load_penalty <= 0)
        {
            // printf("quantum time\n");
            //Update quantum time
            if (quantum_clock > 0)
            {
                quantum_clock -= 1; 
            }
            else
            {
                quantum_clock = quantum;
                curr_process_list = round_robin_shuffle(curr_process_list, &memory);
                
                
                //Loads memory and calculate loading time penalty if not in Unlimited
                //Memory mode
                
                if (strcmp(mem_alloc, MEM_UNLIMITED) != 0)
                {
                    memory_addr = create_uint32_array(memory->n_total_pages, UINT32_MAX);
                    
                    load_penalty = load_into_memory_p(&memory, curr_process_list->pid, curr_process_list->memory_required,
                     memory_addr, cpu_clock);
                    curr_process_list->time_load_penalty = load_penalty;
                    curr_process_list->memory_address = memory_addr;
                }
                print_process_run(cpu_clock, mem_alloc, curr_process_list->time_load_penalty, memory->mem_usage,
                 memory->n_total_pages, curr_process_list);
            }
        }
        //Run process, and adds finished process for performance stats
        fin_flag = execute_process(cpu_clock, &curr_process_list);
        
        

        //Update clocks and timers
        cpu_clock += 1;
    }

    free_datalog(log);

    return 0;
}