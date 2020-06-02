#ifndef PROCESS_SCHEDULING_H
#define PROCESS_SCHEDULING_H

typedef struct process_t
{
    int pid;
    int arrival_time;
    int memory_required;
    int time_required;
    int curr_runtime;

    struct process_t *next;

} process_t;

struct process_t *create_process(int, int, int, int);
void get_all_processes(FILE *, struct process_t *);
struct process_t *get_last_process(struct process_t *);
int has_process_arrived(int, struct process_t *, struct process_t *);

#endif