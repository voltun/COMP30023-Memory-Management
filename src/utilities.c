#include <stdio.h>
#include <stdlib.h>
#include "../include/process_scheduling.h"

/*
Prints out the transcript as listed in project specs for RUNNING
*/
void print_process_run(int current_time, int is_mem_u, struct process_t *process)
{
    // printf("%d, RUNNING, id=%d, remaining-time=%d, load-time=%d, mem-usage=%d%%, mem-addresses=
    //     [%s]\n", current_time, process.pid ,process.curr_runtime);
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

