#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "../include/process_scheduling.h"
#include "../include/utilities.h"

#define UNLIMITED_MEMORY "u"

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
mem_option, char *, parameter of memory allocation, as in spec
mem_usage, int, rounded up percentage of memory usage, ignored if mem_option=="u"
process, struct process_t *, the process linked list, prints first element only
*/
void print_process_run(uint32_t cpu_clock, char *mem_option, int mem_usage, struct process_t *process)
{
    //If the scheduler is running on unlimited memory mode
    if (strcmp(mem_option, UNLIMITED_MEMORY) == 0)
    {
        printf("%"PRIu32", RUNNING, id=%d, remaining-time=%d\n", cpu_clock, process->pid, process->time_required);
    }
    else
    {
        printf("%"PRIu32", RUNNING, id=%d, remaining-time=%d, load-time=%d, mem-usage=%d%%, mem-addresses=[%s]\n",
         cpu_clock, process->pid ,process->time_required, 0, mem_usage, "TODO");
    }
}

/*
Prints out the transcript as listed in project specs for EVICTED 
@params
cpu_clock, uint32_t, representation of CPU clock in Seconds
mem_address, TODO
n_mem_addr, TODO
*/
void print_memory_evict(uint32_t cpu_clock, int *mem_address, int n_mem_addr) 
{
    printf("%"PRIu32", EVICTED, mem-addresses=[%d", cpu_clock, mem_address[0]);
    for (int i = 1; i < n_mem_addr; i++)
    {
        printf(", %d", mem_address[i]);
    }
    printf("\n");
}

/*
Prints out the transcript as listed in project specs for FINISHED
@params
cpu_clock, uint32_t, representation of CPU clock in Seconds
process, struct process_t *, the process linked list, prints first element only
*/
void print_process_finish(uint32_t cpu_clock, struct process_t *process) 
{
    printf("%"PRIu32", FINISHED, id=%d, proc-remaining=%d\n", cpu_clock, process->pid, count_processes(process)-1);
}

void print_performance_stats(uint32_t cpu_clock, struct datalog_t *log)
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

    printf("Throughput %d, %d, %d\n", -1, -1, -1);
    printf("Turnaround time %d\n", turnaround);
    printf("Time overhead %2f %2f\n", -1.0, -1.0);
    printf("Makespan %"PRIu32"\n", cpu_clock);
}

