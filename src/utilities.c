#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process_scheduling.h"

#define UNLIMITED_MEMORY "u"

/*
Prints out the transcript as listed in project specs for RUNNING
@params
cpu_clock, int, representation of CPU clock in Seconds
mem_option, char *, parameter of memory allocation, as in spec
mem_usage, int, rounded up percentage of memory usage, ignored if mem_option=="u"
process, struct process_t *, the process linked list, prints first element only
*/
void print_process_run(int cpu_clock, char *mem_option, int mem_usage, struct process_t *process)
{
    //If the scheduler is running on unlimited memory mode
    if (strcmp(mem_option, UNLIMITED_MEMORY) == 0)
    {
        printf("%d, RUNNING, id=%d, remaining-time=%d\n", cpu_clock, process->pid, process->time_required);
    }
    else
    {
        printf("%d, RUNNING, id=%d, remaining-time=%d, load-time=%d, mem-usage=%d%%, mem-addresses=[%s]\n",
         cpu_clock, process->pid ,process->time_required, 0, mem_usage, "TODO");
    }
}

/*
Prints out the transcript as listed in project specs for EVICTED 
*/
void print_memory_evict() {}

/*
Prints out the transcript as listed in project specs for FINISHED
*/
void print_process_finish(int cpu_clock, struct process_t *process) 
{
    printf("%d, FINISHED, id=%d, proc-remaining=%d\n", cpu_clock, process->pid, count_processes(process)-1);
}

