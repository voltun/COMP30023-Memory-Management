#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "../include/memory.h"
#include "../include/utilities.h"

#define SIZE_PER_MEM_PAGE 4
#define LOADTIME_SWAPPING 2

uint32_t count_unused_mem(struct memory_t *memory);
uint32_t *add_into_memory(struct memory_t **memory, uint32_t pid, uint32_t pages, uint32_t *mem_addr);
uint32_t *evict_from_memory(struct memory_t **memory, uint32_t pid);
struct process_t *find_evictee_lru(struct memory_t *memory);

/*
Initialises the memory_t struct, representation of main memory
Init values of memory is UINT32_MAX to prevent conflict with pid 0
@params
mem_size, uint32_t, max size of main memory in KB
n_max_pid, uint32_t, total number of processes loaded into 
    memory at any given time

@return
struct memory_t *, the initialised memory
*/
struct memory_t *init_memory(uint32_t mem_size, uint32_t n_max_pid)
{
    struct memory_t *mem = malloc(sizeof(struct memory_t));

    if (!mem)
    {
        fprintf(stderr, "Malloc failed!\n");
        exit(1);
    }
    mem->n_total_pages = mem_size / SIZE_PER_MEM_PAGE;
    mem->n_total_proc = n_max_pid;
    mem->pid_loaded = create_uint32_array(n_max_pid, UINT32_MAX);
    
    // if (!(mem->process_loaded))
    // {
    //     fprintf(stderr, "Malloc failed!\n");
    //     exit(1);
    // }
    
    mem->main_memory = create_uint32_array(mem->n_total_pages, UINT32_MAX);

    return mem;
}

/*
Loads the given process' memory pages into the main memory
!! ASSUMES SWAPPING-X
@params
memory, struct memory_t **, pointer to the memory_t * to allow modification
pid, uint32_t, Process ID of requesting process
mem_size, uint32_t, size of memory to be allocated in KB

@return
uint32_t, the time required to load given process' pages into memory, in Seconds
*/
uint32_t load_into_memory_p(struct memory_t **memory, uint32_t pid, uint32_t mem_size, uint32_t *mem_addr)
{
    uint32_t *evicted_mem = NULL;
    uint32_t req_pages = mem_size / SIZE_PER_MEM_PAGE;
    uint32_t free_space = count_unused_mem(*memory);
    

    mem_addr = reinit_uint32_array(mem_addr, (*memory)->n_total_pages, UINT32_MAX);

    // process->memory_address = create_uint32_array((*memory)->n_total_pages, UINT32_MAX);
    
    // printf("LOADING PID: %"PRIu32"\n", process->pid);
    // printf("REQ PAGES: %"PRIu32", FREE PAGES: %"PRIu32"\n", req_pages, free_space);
    //Loads process pages into memory if available space
    if (free_space >= req_pages)
    {
        mem_addr = add_into_memory(memory, pid, req_pages, mem_addr);
    }
    //Evict processes by least-recently-used
    else
    {
        //Keep evicting until available free space
        // while(free_space <= req_pages)
        // {
        //     evicted_mem = evict_from_memory(memory, find_evictee_lru(*memory));
        // }
    }

    return req_pages*LOADTIME_SWAPPING;
}

/*
Evicts all memory pages related to given process from main memory
@params
memory, struct memory_t **, pointer to the memory_t * to allow modification
pid, uint32_t, process id of evictee :(

@return
uint32_t *, an array of memory space address freed, due to the eviction
*/
uint32_t *evict_from_memory(struct memory_t **memory, uint32_t pid)
{
    uint32_t counter = 0;
    uint32_t *evicted_mem_addr = NULL;

    evicted_mem_addr = create_uint32_array((*memory)->n_total_pages, UINT32_MAX);

    //Start evicting process' pages from memory
    for (uint32_t i = 0; i < (*memory)->n_total_pages; i++)
    {
        if ((*memory)->main_memory[i] == pid)
        {
            evicted_mem_addr[counter] = i;
            counter += 1;
            (*memory)->main_memory[i] = UINT32_MAX;
        }
    }
    
    // (*memory)->process_loaded = list_remove((*memory)->process_loaded, victim);
    return evicted_mem_addr;
}

/*
Finds the process to evict based on least recently used algorithm
@params
memory, struct memory_t *, the memory representation

@return
struct process_t *, the process chosen to be evicted
*/
struct process_t *find_evictee_lru(struct memory_t *memory)
{
    return NULL;
}

void evict_memory_fin(struct memory_t **memory, struct process_t *process)
{

}

/*
Counts number of unused memory pages in the main memory
@params
memory, struct memory_t *, memory representation

@return
uint32_t, number of unused memory pages
*/
uint32_t count_unused_mem(struct memory_t *memory)
{
    uint32_t unused_pages = 0;
    for (uint32_t i = 0; i < memory->n_total_pages; i++)
    {
        if (memory->main_memory[i] == UINT32_MAX)
        {
            unused_pages += 1;
        }
    }

    return unused_pages;
}

/*
Updates pid into memory and load pages into the main memory
!! ASSUMES AVAILABLE FREE SPACE TO LOAD PAGES INTO MEMORY
@params
memory, struct memory_t **, pointer to the memory_t * to allow modification
pid, uint32_t, Process ID of requesting process
pages, uint32_t, number of pages to load
mem_addr, uint32_t *, Must be initialised, stores assigned memory addresses

@return
uint32_t *, array of allocated memory addresses
*/
uint32_t *add_into_memory(struct memory_t **memory, uint32_t pid, uint32_t pages, uint32_t *mem_addr)
{
    // struct process_t *dup = malloc(sizeof(process_t));
    uint32_t n = 0;

    // if (!dup)
    // {
    //     fprintf(stderr, "Malloc failed!\n");
    //     exit(1);
    // }

    // dup = process;
    // dup->next = NULL;
    
    // //Stores duplicate process info for better book keeping
    // struct process_t *curr = get_last_process((*memory)->process_loaded);
    // curr->next = dup;
    // printf("HEHE\n");
    // (*memory)->process_loaded = list_push((*memory)->process_loaded, dup);

    // print_list((*memory)->process_loaded);
    
    //Insert pages into memory
    for (uint32_t i = 0; i < (*memory)->n_total_pages; i++)
    {
        if ((*memory)->main_memory[i] == UINT32_MAX)
        {
            (*memory)->main_memory[i] = pid;
            mem_addr[n] = i;
            pages -= 1;
            n += 1;
            // printf("INSERTING A PAGE. REMAINING: %"PRIu32"\n", pages);
            // printf("memory address given: %"PRIu32"\n", mem_addr[n-1]);
        }

        //breaks after all pages inserted into memory
        if (pages <= 0)
        {
            break;
        }
    }
    
    return mem_addr;
}
