#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdint.h>
#include "../include/process_scheduling.h"

typedef struct datalog_t 
{
    uint32_t n_proc_fin;
    struct process_t *finished_process;
    
} datalog_t;

struct datalog_t *init_datalog();
void free_datalog(struct datalog_t *);
uint32_t *create_uint32_array(uint32_t, uint32_t);
uint32_t *reinit_uint32_array(uint32_t *, uint32_t, uint32_t);
struct datalog_t *add_fin_process(struct datalog_t *, struct process_t *);
void print_process_run(uint32_t, char *, uint32_t, int, uint32_t, struct process_t *);
void print_process_finish(uint32_t, struct process_t *); 
void print_memory_evict(uint32_t, uint32_t *, uint32_t); 
void print_performance_stats(uint32_t, struct datalog_t *);

#endif