#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include "../include/memory.h"
#include "../include/utilities.h"

#define SIZE_PER_MEM_PAGE 4
#define LOADTIME_SWAPPING 2
#define SIZE_VMEM_MIN_RUN 16

uint32_t count_unused_mem(struct memory_t *memory);
uint32_t *add_into_memory(struct memory_t **memory, uint32_t pid, uint32_t pages, uint32_t *mem_addr);
uint32_t find_evictee_lru(struct memory_t *memory);
void update_mem_usage(struct memory_t **memory);
uint32_t has_been_loaded(struct memory_t *memory, uint32_t pid);
uint32_t *evict_one_by_one(struct memory_t **memory, uint32_t pid);

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
    mem->main_memory = create_uint32_array(mem->n_total_pages, UINT32_MAX);
    mem->reference_bit = create_uint32_array(mem->n_total_pages, 0);

    return mem;
}

/*
Loads the given process' memory pages into the main memory
!! ASSUMES SWAPPING-X
@params
memory, struct memory_t **, pointer to the memory_t * to allow modification
pid, uint32_t, Process ID of requesting process
mem_size, uint32_t, size of memory to be allocated in KB    
cpu_clock, uint32_t, representation of CPU clock in Seconds

@return
uint32_t, the time required to load given process' pages into memory, in Seconds
*/
uint32_t load_into_memory_p(struct memory_t **memory, uint32_t pid, uint32_t mem_size, uint32_t *mem_addr, uint32_t cpu_clock)
{
    uint32_t *evicted_mem = NULL, *final_evict_addr = NULL;
    uint32_t evictee = 0;
    uint32_t req_pages = mem_size / SIZE_PER_MEM_PAGE;
    uint32_t free_space = count_unused_mem(*memory);

    //Check if process was suspended prior and has all its file already loaded
    if (has_been_loaded(*memory, pid) == req_pages)
    {
        return 0;
    }
    
    mem_addr = reinit_uint32_array(mem_addr, (*memory)->n_total_pages, UINT32_MAX);

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
        while(free_space < req_pages)
        {
            //Find pid of evictee, UINT32_MAX if none found
            evictee = find_evictee_lru(*memory);

            evicted_mem = evict_from_memory(memory, evictee);
            final_evict_addr = add_to_array_nodup(final_evict_addr, evicted_mem, (*memory)->n_total_pages);

            free_space = count_unused_mem(*memory);
        }
        print_memory_evict(cpu_clock, final_evict_addr, (*memory)->n_total_pages);
        mem_addr = add_into_memory(memory, pid, req_pages, mem_addr);   
    }
    return req_pages*LOADTIME_SWAPPING;
}

/*
Loads the given process' memory pages into the main memory
!! ASSUMES VIRTUAL MEMORY
@params
memory, struct memory_t **, pointer to the memory_t * to allow modification
pid, uint32_t, Process ID of requesting process
mem_size, uint32_t, size of memory to be allocated in KB    
page_fault, uint32_t *, pointer to time penalty offset for page faults in Seconds
cpu_clock, uint32_t, representation of CPU clock in Seconds

@return
uint32_t, the time required to load given process' pages into memory, in Seconds
*/
uint32_t load_into_memory_v(struct memory_t **memory, uint32_t pid, uint32_t mem_size,
 uint32_t *mem_addr, uint32_t *fault, uint32_t cpu_clock)
{
    uint32_t *final_evict_addr = NULL, *evicted_mem = NULL;
    uint32_t min_exec_pages = SIZE_VMEM_MIN_RUN / SIZE_PER_MEM_PAGE;
    uint32_t evictee = 0, loaded_pages = 0, n_loaded = 0;
    uint32_t req_pages = mem_size / SIZE_PER_MEM_PAGE;
    uint32_t free_space = count_unused_mem(*memory);
    int cont_flag = 0;

    loaded_pages = has_been_loaded(*memory, pid);
    *fault = 0;
    //If the minimum required pages to run given process met, don't need to insert more
    //OR if total number of pages required by process is less than minimum required and already
    //in memory, return
    if (loaded_pages >= min_exec_pages)
    {
        //Page faults
        if (loaded_pages < req_pages)
        {
            *fault = 1;
        }
        return 0;
    }
    if ((req_pages < min_exec_pages) && (loaded_pages >= req_pages))
    {
        return 0;
    }

    mem_addr = reinit_uint32_array(mem_addr, (*memory)->n_total_pages, UINT32_MAX);

    //Loads all process pages into memory if available space
    if (free_space >= (req_pages - loaded_pages))
    {
        n_loaded = req_pages - loaded_pages;
    }
    //Loads as much pages as possible if free space meets minimum execution pages
    //but not enough free space to load all process pages
    else if (free_space >= min_exec_pages)
    {
        *fault = 1;
        n_loaded = free_space;
    }
    //Evicts some/all processes until minimum execution requirement
    else
    {
        *fault = 1;
        final_evict_addr = create_uint32_array((*memory)->n_total_pages, UINT32_MAX);
        //Keep evicting until minimum execution pages met or process has enough to fit all if less
        //memory needed
        while (free_space < min_exec_pages && free_space < (req_pages - loaded_pages))
        {
            //Finds new pid only after eviction has been done for a previous pid
            //and still not enough space
            if (!cont_flag)
            {
                //Find pid of evictee, UINT32_MAX if none found
                evictee = find_evictee_lru(*memory);
                cont_flag = 1;
            }
            
            evicted_mem = evict_one_by_one(memory, evictee);

            //Did not evict any memory address means no more pages in memory
            if (evicted_mem == NULL)
            {
                cont_flag = 0;
                continue;
            }
            final_evict_addr = add_to_array_nodup(final_evict_addr, evicted_mem, (*memory)->n_total_pages);

            free_space = count_unused_mem(*memory);
        }

        print_memory_evict(cpu_clock, final_evict_addr, (*memory)->n_total_pages);

        if ((req_pages - loaded_pages) < min_exec_pages)
        {
            n_loaded = req_pages - loaded_pages;
        }
        else
        {
            n_loaded = min_exec_pages;
        }
    }
    mem_addr = add_into_memory(memory, pid, n_loaded, mem_addr);

    update_mem_usage(memory);

    return n_loaded*LOADTIME_SWAPPING;
}

/*
Loads the given process' memory pages into the main memory
!! ASSUMES SECOND CHANCE
@params
memory, struct memory_t **, pointer to the memory_t * to allow modification
pid, uint32_t, Process ID of requesting process
mem_size, uint32_t, size of memory to be allocated in KB    
page_fault, uint32_t *, pointer to time penalty offset for page faults in Seconds
cpu_clock, uint32_t, representation of CPU clock in Seconds

@return
uint32_t, the time required to load given process' pages into memory, in Seconds
*/
uint32_t load_into_memory_cm(struct memory_t **memory, uint32_t pid, uint32_t mem_size,
 uint32_t *mem_addr, uint32_t *fault, uint32_t cpu_clock)
{
    uint32_t *final_evict_addr = NULL, *evicted_mem = NULL;
    uint32_t min_exec_pages = SIZE_VMEM_MIN_RUN / SIZE_PER_MEM_PAGE;
    uint32_t loaded_pages = 0, n_loaded = 0, prev_proc = UINT32_MAX;
    uint32_t req_pages = mem_size / SIZE_PER_MEM_PAGE;
    uint32_t free_space = count_unused_mem(*memory);
    int found_flag = 0;
    
    loaded_pages = has_been_loaded(*memory, pid);
    *fault = 0;
    //If the minimum required pages to run given process met, don't need to insert more
    //OR if total number of pages required by process is less than minimum required and already
    //in memory, return
    if (loaded_pages >= min_exec_pages)
    {
        //Page faults
        if (loaded_pages < req_pages)
        {
            *fault = 1;
        }
        return 0;
    }
    if ((req_pages < min_exec_pages) && (loaded_pages >= req_pages))
    {
        return 0;
    }
    
    mem_addr = reinit_uint32_array(mem_addr, (*memory)->n_total_pages, UINT32_MAX);
    
    //Loads all process pages into memory if available space
    if (free_space >= (req_pages - loaded_pages))
    {       
        n_loaded = req_pages - loaded_pages;
        mem_addr = add_into_memory(memory, pid, n_loaded, mem_addr);
    }
    //Loads as much pages as possible if free space meets minimum execution pages
    //but not enough free space to load all process pages
    else if (free_space >= min_exec_pages)
    {
        *fault = 1;
        n_loaded = free_space;
        mem_addr = add_into_memory(memory, pid, n_loaded, mem_addr);
    }
    //Evicts some/all processes until minimum execution requirement
    else
    {
        final_evict_addr = create_uint32_array((*memory)->n_total_pages, UINT32_MAX);
        *fault = 1;
        
        //Loop until enough space to insert required pages
        for (uint32_t _n = 0; _n < (req_pages - loaded_pages) && _n < min_exec_pages; _n ++)
        {
            
            while(!found_flag)
            {
                //Loop through second chance
                for (uint32_t i = 0; i < (*memory)->n_total_pages; i++)
                {
                    //skip page frame if owned by executing process
                    if ((*memory)->main_memory[i] == pid)
                    {
                        continue;
                    }
                    //Replace page with no second chance
                    if ((*memory)->reference_bit[i] == 0)
                    {
                        found_flag = 1;
                        prev_proc = (*memory)->main_memory[i];
                        (*memory)->main_memory[i] = pid;
                        evicted_mem = create_uint32_array((*memory)->n_total_pages, UINT32_MAX);
                        evicted_mem[0] = i;
                        n_loaded += 1;

                        //Check if evicted process still has any pages in memory
                        if (has_been_loaded(*memory, prev_proc) <= 0)
                        {
                            //Remove pid from book keeping
                            for (uint32_t j = 0; i < (*memory)->n_total_proc; i++)
                            {
                                if ((*memory)->pid_loaded[j] == prev_proc)
                                {
                                    (*memory)->pid_loaded[j] = UINT32_MAX;
                                    break;
                                }
                            }
                        }
                        final_evict_addr = add_to_array_nodup(final_evict_addr, evicted_mem, (*memory)->n_total_pages);

                        break;
                    }
                    //Set reference bit to 0 
                    else
                    {
                        (*memory)->reference_bit[i] = 0;
                    }
                    
                }
            }
            found_flag = 0;
        }   
        *fault = 1;

        print_memory_evict(cpu_clock, final_evict_addr, (*memory)->n_total_pages);
    }
    
    update_mem_usage(memory);

    return n_loaded*LOADTIME_SWAPPING;
}

/*
Evicts only one process memory page from the memory
!! ASSUMES MEMORY HAS PAGES ALREADY LOADED AND EVICT FIRST OCCURENCE
@params
memory, struct memory_t **, pointer to the memory_t * to allow modification
pid, uint32_t, process id of evictee :(

@return
uint32_t *, memory space address freed, due to eviction
*/
uint32_t *evict_one_by_one(struct memory_t **memory, uint32_t pid)
{
    uint32_t *ret_pack = NULL;

    //Start evicting process' pages from memory
    for (uint32_t i = 0; i < (*memory)->n_total_pages; i++)
    {
        if ((*memory)->main_memory[i] == pid)
        {
            //Creates an array, with first element as address and rest as
            //padding 
            ret_pack = create_uint32_array((*memory)->n_total_pages, UINT32_MAX);
            (*memory)->main_memory[i] = UINT32_MAX;
            ret_pack[0] = i;

            if (has_been_loaded(*memory, pid) <= 0)
            {
                //Remove pid from book keeping
                for (uint32_t i = 0; i < (*memory)->n_total_proc; i++)
                {
                    if ((*memory)->pid_loaded[i] == pid)
                    {
                        (*memory)->pid_loaded[i] = UINT32_MAX;
                        break;
                    }
                }
            }
            update_mem_usage(memory);
            break;
        }
    }

    return ret_pack;
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
            //Init array
            if (!evicted_mem_addr)
            {
                evicted_mem_addr = create_uint32_array((*memory)->n_total_pages, UINT32_MAX);
            }
            evicted_mem_addr[counter] = i;
            counter += 1;
            (*memory)->main_memory[i] = UINT32_MAX;
        }
    }
    update_mem_usage(memory);

    return evicted_mem_addr;
}

/*
Finds the process to evict based on least recently used algorithm
!! ASSUMES BY DEFAULT THE BOOKKEEPING PID IS EQUIVALENT TO RUNNING PROCESSES
@params
memory, struct memory_t *, the memory representation

@return
uint32_t, the process ID chosen to be evicted (UINT32_MAX IF NONE)
*/
uint32_t find_evictee_lru(struct memory_t *memory)
{
    for (uint32_t i = 0; i < memory->n_total_proc; i++)
    {
        //Reach end of running processes list
        if (memory->pid_loaded[i] == UINT32_MAX)
        {   
            //singleton
            if (i == 0)
            {
                return memory->pid_loaded[i];
            }
            
            return memory->pid_loaded[i-1];
        }

    }
    //Not supposed to be here
    return UINT32_MAX;
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
Counts pages that are already loaded (if any) in memory by a particular process
@params
memory, struct memory_t *, memory representation
pid, uint32_t, the Process ID 

@return
uint32_t, number of pages that corresponds to the PID, if any
*/
uint32_t has_been_loaded(struct memory_t *memory, uint32_t pid)
{
    uint32_t count = 0;

    for (uint32_t i = 0; i < memory->n_total_pages; i++)
    {
        //Found a page allocated to the process
        if (memory->main_memory[i] == pid)
        {
            count += 1;
        }
    }
    return count;
}

/*
Sets the reference bit of loaded process page frames to 1
@params
memory, struct memory_t **, pointer to the memory_t * to allow modification
flag, uint32_t, 0 or 1
pid, uint32_t, Process ID of executed process
*/
void set_reference_bits(struct memory_t **memory, uint32_t flag, uint32_t pid)
{
    for (uint32_t i = 0; i < (*memory)->n_total_pages; i++)
    {
        if ((*memory)->main_memory[i] == pid)
        {
            (*memory)->reference_bit[i] = flag;
        }
    }
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

/*
Updates memory usage in the main memory
!! Modifies in array
@params
memory, struct memory_t **, pointer to the memory_t * to allow modification
*/
void update_mem_usage(struct memory_t **memory)
{
    int free_space = 0;
    float usage = 0.0f;
    int total = (int) (*memory)->n_total_pages;

    free_space = (int) count_unused_mem(*memory);
    
    usage = ceil( (((float)(total - free_space)) / ((float)total)) * 100.0 );
    
    (*memory)->mem_usage = usage;
}

/*
Frees up the memory struct
@param
memory, struct memory_t *, the memory
*/
void free_memory(struct memory_t *memory)
{
    free(memory->pid_loaded);
    free(memory->main_memory);
    free(memory->reference_bit);
    
    free(memory);
}
