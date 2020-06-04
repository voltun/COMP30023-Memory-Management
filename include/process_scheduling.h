#ifndef PROCESS_SCHEDULING_H
#define PROCESS_SCHEDULING_H

#include <stdint.h>
#include "../include/memory.h"

typedef struct process_t
{
    uint32_t pid;
    uint32_t arrival_time;
    uint32_t memory_required;
    uint32_t time_required;
    uint32_t job_time;
    uint32_t time_last_used;
    uint32_t time_finished;
    uint32_t time_load_penalty;

    uint32_t *memory_address;
    struct process_t *next;

} process_t;

//DEBUG
void print_list(struct process_t *);

struct process_t *create_process(uint32_t, uint32_t, uint32_t, uint32_t);
struct process_t *get_all_processes(FILE *);
struct process_t *get_last_process(struct process_t *);
int has_process_arrived(uint32_t, struct process_t *);
struct process_t *list_push(struct process_t *, struct process_t *);
struct process_t *list_pop(struct process_t **);
struct process_t *list_remove(struct process_t *, struct process_t *);
uint32_t count_processes(struct process_t *);
int execute_process(uint32_t, struct process_t **);
struct process_t *round_robin_shuffle(struct process_t *, struct memory_t **);
void free_list(struct process_t *);

#endif