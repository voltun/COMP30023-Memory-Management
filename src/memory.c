#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include "../include/memory.h"
#include "../include/utilities.h"

#define SIZE_PER_MEM_PAGE 4
#define LOADTIME_SWAPPING 2

uint32_t count_unused_mem(struct memory_t *memory);
uint32_t *add_into_memory(struct memory_t **memory, uint32_t pid, uint32_t pages, uint32_t *mem_addr);
uint32_t *evict_from_memory(struct memory_t **memory, uint32_t pid);
uint32_t find_evictee_lru(struct memory_t *memory);
void update_mem_usage(struct memory_t **memory);

/*
Initialises the memory_t struct, representation of main memory
Init values of memory is UINT32_MAX to prevent conflict with pid 0
@params
mem_size, uint32_t, max size of main memory in KB
n_total_proc, uint32_t, total number of processes loaded into 
    memory at any given time

@return
struct memory_t *, the initialised memory
*/
struct memory_t *init_memory(uint32_t mem_size, uint32_t n_total_proc)
{
    struct memory_t *mem = malloc(sizeof(struct memory_t));

    if (!mem)
    {
        fprintf(stderr, "Malloc failed!\n");
        exit(1);
    }
    mem->n_total_pages = mem_size / SIZE_PER_MEM_PAGE;
    mem->n_total_proc = n_total_proc;
    mem->mem_usage = 0;
    mem->pid_loaded = create_uint32_array(n_total_proc, UINT32_MAX);
    
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
    uint32_t *evicted_mem = NULL, *final_evict_addr = NULL;
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
        final_evict_addr = create_uint32_array((*memory)->n_total_pages, UINT32_MAX);
        //Keep evicting until available memory space
        while(free_space <= req_pages)
        {
            evicted_mem = evict_from_memory(memory, find_evictee_lru(*memory));
            final_evict_addr = add_to_array_nodup(final_evict_addr, evicted_mem, (*memory)->n_total_pages);

            // uint32_t i = 0;
            // while(final_evict_addr[i] != UINT32_MAX)
            // {
            //     printf("EVICTED ADDR: %"PRIu32"\n", final_evict_addr[i]);
            //     i += 1;
            // }

            free_space = count_unused_mem(*memory);
        }
        mem_addr = add_into_memory(memory, pid, req_pages, mem_addr);
    }

    return req_pages*LOADTIME_SWAPPING;
}

/*
Evicts all memory pages related to given process from main memory
!! ASSUMES MEMORY HAS PAGES ALREADY LOADED
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

    //Remove pid from book keeping
    for (uint32_t i = 0; i < (*memory)->n_total_proc; i++)
    {
        if ((*memory)->pid_loaded[i] == pid)
        {
            (*memory)->pid_loaded[i] = UINT32_MAX;
            break;
        }
    }

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
!! ASSUMES BY DEFAULT THE BOOKKEEPING PID IS EQUIVALENT TO RUNNING PROCESSES
@params
memory, struct memory_t *, the memory representation

@return
uint32_t, the process ID chosen to be evicted
*/
uint32_t find_evictee_lru(struct memory_t *memory)
{

    for (uint32_t i = 0; i < memory->n_total_pages; i++)
    {
        if (memory->pid_loaded[i] == UINT32_MAX)
        {
            return memory->pid_loaded[i-1];
        }
    }
    return -1;
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
    uint32_t n = 0;

    //Insert loaded pid for book keeping
    for (uint32_t i = 0; i < (*memory)->n_total_proc; i++)
    {
        if ((*memory)->pid_loaded[i] == UINT32_MAX)
        {
            (*memory)->pid_loaded[i] = pid;
            break;
        }
    }

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

    update_mem_usage(memory);
    
    return mem_addr;
}

void update_mem_usage(struct memory_t **memory)
{
    int free_space = 0;
    float usage = 0.0f;
    int total = (int) (*memory)->n_total_pages;

    free_space = (int) count_unused_mem(*memory);
    
    usage = 100.0 - ceil( (((float)free_space) / ((float)total)) * 100.0);
    
    (*memory)->mem_usage = usage;
}
