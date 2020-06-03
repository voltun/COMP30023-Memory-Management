#ifndef UTILITIES_H
#define UTILITIES_H

#include "../include/process_scheduling.h"

typedef struct datalog_t 
{
    int n_proc_fin;
    struct process_t *finished_process;
    
} datalog_t;

struct datalog_t *init_datalog();
struct datalog_t *add_fin_process(struct datalog_t *, struct process_t *);
void print_process_run(int, char *, int, struct process_t *);
void print_process_finish(int, struct process_t *); 
void print_memory_evict(int, int *, int); 
void print_performance_stats(int, struct datalog_t *);

#endif