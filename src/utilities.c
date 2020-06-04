#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <stdint.h>
#include "../include/process_scheduling.h"
#include "../include/utilities.h"

#define THROUGHPUT_INTERVAL 60
#define N_THROUGHPUT_METRIC 3
#define N_OVERHEAD_METRIC 2
#define MEM_UNLIMITED "u"

int get_turnaround_time(struct datalog_t *log);
uint32_t *get_throughput(struct datalog_t *log, uint32_t cpu_clock);
double *get_overhead(struct datalog_t *log);

/*
initialises the datalog
@return
struct datalog *, the initialised datalog
*/
struct datalog_t *init_datalog()
{
    struct datalog_t *log = malloc(sizeof(struct datalog_t));

    log->n_proc_fin = 0;
    log->finished_process = NULL;

    return log;
}

/*
Updates finished process' stats into the log
@params
log, struct datalog_t *, the datalog struct
process, struct datalog_t *, the finished process

@return
stuct datalog_t *, the updated log
*/
struct datalog_t *add_fin_process(struct datalog_t *log, struct process_t *process)
{
    struct process_t *curr = log->finished_process;
    log->n_proc_fin += 1;

    if (log->finished_process == NULL)
    {
        log->finished_process = process;
        return log;
    }

    while (curr->next != NULL)
    {
        curr = curr-> next;
    }

    curr->next = process;

    return log;
}

/*
Frees up the datalog_t data struct
@params
log, struct datalog_t, the log to be free
*/
void free_datalog(struct datalog_t *log)
{
    free_list(log->finished_process);
    free(log);
}

/*
Prints out the transcript as listed in project specs for RUNNING
!! Usable for both Unlimited Memory and Limited Memory
@params
cpu_clock, uint32_t, representation of CPU clock in Seconds
flag_unlimited, int, flag to represent Unlimited Memory 
load_time, uint32_t, the time in seconds, to load the pages into memory
    , ignored if mem_option=="u"
mem_usage, int, rounded up percentage of memory usage, ignored if mem_option=="u"
n_mem_addr, uint32_t, max size of mem_addr array
process, struct process_t *, the process linked list, prints first element only
*/
void print_process_run(uint32_t cpu_clock, char *mem_alloc, uint32_t load_time, int mem_usage, 
uint32_t n_mem_addr, struct process_t *process)
{
    //If the scheduler is running on unlimited memory mode
    if ((strcmp(mem_alloc, MEM_UNLIMITED) == 0))
    {
        printf("%"PRIu32", RUNNING, id=%d, remaining-time=%d\n", cpu_clock, process->pid, process->time_required);
    }
    else
    {
        printf("%"PRIu32", RUNNING, id=%"PRIu32", remaining-time=%"PRIu32", load-time=%"PRIu32", mem-usage=%d%%, mem-addresses=",
         cpu_clock, process->pid ,process->time_required, load_time, mem_usage);

        printf("[%"PRIu32, process->memory_address[0]);
    
        for (uint32_t i = 1; i < n_mem_addr; i++)
        {
            if (process->memory_address[i] == UINT32_MAX)
            {
                break;
            }
            printf(",%"PRIu32, process->memory_address[i]);
        }
        printf("]\n");

    }
}

/*
Prints out the transcript as listed in project specs for EVICTED 
@params
cpu_clock, uint32_t, representation of CPU clock in Seconds
mem_address, uint32_t *, array of evicted memory addresses
n_mem_addr, uint32_t, size of mem_address
*/
void print_memory_evict(uint32_t cpu_clock, uint32_t *mem_address, uint32_t n_mem_addr) 
{
    printf("%"PRIu32", EVICTED, mem-addresses=[%"PRIu32, cpu_clock, mem_address[0]);
    for (uint32_t i = 1; i < n_mem_addr; i++)
    {
        if (mem_address[i] == UINT32_MAX)
        {
            break;
        }
        printf(",%d", mem_address[i]);
    }
    printf("]\n");
}

/*
Prints out the transcript as listed in project specs for FINISHED
@params
cpu_clock, uint32_t, representation of CPU clock in Seconds
process, struct process_t *, the process linked list, prints first element only
*/
void print_process_finish(uint32_t cpu_clock, struct process_t *process) 
{
    printf("%"PRIu32", FINISHED, id=%"PRIu32", proc-remaining=%"PRIu32"\n", cpu_clock, process->pid, count_processes(process)-1);
}

void print_performance_stats(uint32_t cpu_clock, struct datalog_t *log)
{
    int turnaround = 0;
    uint32_t *throughput = NULL;
    double *overhead = NULL;

    //Calculate throughput
    throughput = get_throughput(log, cpu_clock);

    //Calculate turnaround time
    turnaround = get_turnaround_time(log);

    //Calculate overhead
    overhead = get_overhead(log);

    printf("Throughput %"PRIu32", %"PRIu32", %"PRIu32"\n", throughput[0], throughput[1], throughput[2]);
    printf("Turnaround time %"PRIu32"\n", turnaround);
    printf("Time overhead %.2f %.2f\n", overhead[0], overhead[1]);
    printf("Makespan %"PRIu32"\n", cpu_clock);
}

/*
Calculate average time (in seconds, rounded up to an integer) between the time when the
process completed and when it arrived
@params
log, struct memory_t *, datalog

@return
int, turnaround time
*/
int get_turnaround_time(struct datalog_t *log)
{
    double sum_for_turnaround = 0.0;
    int turnaround = 0;

    //Calculating turnaround time
    for (struct process_t *curr = log->finished_process; curr != NULL; curr=curr->next)
    {
        sum_for_turnaround += (curr->time_finished - curr->arrival_time);
    }

    sum_for_turnaround /= log->n_proc_fin;
    turnaround = ceil(sum_for_turnaround);

    return turnaround;
}

/*
Calculate average (rounded up to an integer), minimum and maximum number of processes
completed in sequential non-overlapping 60 second intervals
@params
log, struct memory_t *, datalog
cpu_clock, uint32_t, representation of CPU clock in Seconds

@return
uint32_t *, [average, min, max]
*/
uint32_t *get_throughput(struct datalog_t *log, uint32_t cpu_clock)
{
    uint32_t *ret_val = NULL;
    //Determine how many intervals the array should have
    uint32_t size = (uint32_t) floor(cpu_clock / THROUGHPUT_INTERVAL);
    uint32_t *interval_list = create_uint32_array(size, 0);
    uint32_t index = 0;
    uint32_t sum = 0, smallest = UINT32_MAX, biggest = 0, avg = 0;

    //Calculate number of processes for each interval
    for (struct process_t *curr = log->finished_process; curr != NULL; curr=curr->next)
    {
        index = ((uint32_t) floor((curr->time_finished - 1) / THROUGHPUT_INTERVAL));
        
        interval_list[index] += 1;
    }

    //Calculate throughput metrics
    for (uint32_t i = 0; i < size; i++)
    {
        //Store sum for average
        sum += interval_list[i];

        //Find minimum number of processes per interval
        if (interval_list[i] < smallest)
        {
            smallest = interval_list[i];
        }

        //Find maximum number of processes per interval
        if (interval_list[i] > biggest)
        {
            biggest = interval_list[i];
        }
    }

    //Calculate average
    avg = (uint32_t) ceil(sum / size);

    //Package data for return
    ret_val = malloc(sizeof(uint32_t)*N_THROUGHPUT_METRIC);
    ret_val[0] = avg;
    ret_val[1] = smallest;
    ret_val[2] = biggest;

    return ret_val;
}

/*
Calculate maximum and average time overhead when running a process, both rounded to the
first two decimal points, where overhead is defined as the turnaround time of the process divided
by its job-time.
@params
log, struct memory_t *, datalog

@return
uint32_t *, [max, average]
*/
double *get_overhead(struct datalog_t *log)
{
    double *ret_val = NULL;
    double *overhead_list = malloc(sizeof(double) * log->n_proc_fin);
    double turnaround_time = 0.0f, avg = 0.0f, sum = 0.0f, biggest = 0.0f;
    struct process_t *curr = log->finished_process;

    if (!overhead_list)
    {
        fprintf(stderr, "Malloc failed!\n");
        exit(1);
    }

    for (uint32_t i = 0; i < log->n_proc_fin; i++)
    {
        overhead_list[i] = 0.0f;
    }

    //Calculate each process' overhead time
    for (uint32_t i = 0; i < log->n_proc_fin; i++)
    {
        turnaround_time = curr->time_finished - curr->arrival_time;
        overhead_list[i] =  turnaround_time / ((double)curr->job_time);
        curr = curr->next;

        //For calculating average overhead time
        sum += overhead_list[i];

        //Calculate maximum overhead time
        if (overhead_list[i] > biggest)
        {
            biggest = overhead_list[i];
        }
    }

    //Calculate average
    avg = sum / log->n_proc_fin;

    //Package data for return
    ret_val = malloc(sizeof(double)*N_OVERHEAD_METRIC);
    ret_val[0] = biggest;
    ret_val[1] = avg;

    return ret_val;
}

/*
Mallocs and initialises a new uint32_t array
@params
size, uint32_t, size of the new array
init_value, uint32_t, initialising value of each entry in the array

@return
uint32_t *, the initialised uint32_t array
*/
uint32_t *create_uint32_array(uint32_t size, uint32_t init_value)
{
    uint32_t *arr = malloc(sizeof(uint32_t) * size);

    if (!arr)
    {
        fprintf(stderr, "Malloc failed!\n");
        exit(1);
    }

    for (uint32_t i = 0; i < size; i++)
    {
        arr[i] = init_value;
    }

    return arr;
}

/*
Reinitialises an existing uint32_t array
@params
array, uint32_t *, an already malloc-ed uint32_t array
size, uint32_t, size of the new array
init_value, uint32_t, initialising value of each entry in the array

@return
uint32_t *, the reinitialised array
*/
uint32_t *reinit_uint32_array(uint32_t *array, uint32_t size, uint32_t init_value)
{
    for (uint32_t i = 0; i < size; i++)
    {
        //Break out of loop if part of already initialised array has been reached
        if(array[i] == init_value)
        {
            break;
        }
        array[i] = init_value;
    }
    return array;
}

/*
Adds elements from array not in master, into master
!! ASSUMES BOTH ARRAYS ARE SAME MAX LENGTH
@params
master, uint32_t, the array to append to, if any appends
array, uint32_t, array to check 
size, uint32_t, max elements of master

@return
uint32_t *, the modified master array
*/
uint32_t *add_to_array_nodup(uint32_t *master, uint32_t *array, uint32_t size)
{
    uint32_t *new_list = create_uint32_array(size, UINT32_MAX);
    uint32_t count1 = 0, count2 = 0;

    for (uint32_t i = 0; i < size; i++)
    {
        if (master[count1] == UINT32_MAX && array[count2] == UINT32_MAX)
        {
            break;
        }
        //master array is used up
        else if (master[count1] == UINT32_MAX)
        {
            new_list[i] = array[count2];
            count2 += 1;
            continue;
        }
        //input array used up
        else if (array[count2] == UINT32_MAX)
        {
            new_list[i] = master[count1];
            count1 += 1;
            continue;
        }

        //Sequential insert 
        if (master[count1] < array[count2])
        {
            new_list[i] = master[count1];
            count1 += 1;
        }
        else
        {
            new_list[i] = array[count2];
            count2 += 1;
        }
    
    }

    return new_list;
}
