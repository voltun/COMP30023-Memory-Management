#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <inttypes.h>
#include "../include/process_scheduling.h"

typedef struct memory_t
{
    uint32_t n_total_proc;
    uint32_t n_total_pages;
    uint32_t *pid_loaded;
    uint32_t *main_memory;
    int mem_usage;
    
} memory_t;

struct memory_t *init_memory(uint32_t, uint32_t);
uint32_t load_into_memory_p(struct memory_t **, uint32_t, uint32_t, uint32_t *);
uint32_t *evict_from_memory(struct memory_t **, uint32_t);

#endif