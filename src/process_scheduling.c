#include <stdio.h>
#include <stdlib.h>

#define SIZE_BUFFER 256

typedef struct process_t
{
    int pid;
    int arrival_time;
    int memory_required;
    int time_required;
    int curr_runtime;

    struct process_t *next;

} process_t;

struct process_t *get_next_process(FILE *fptr);
struct process_t *list_pop(struct process_t *list);
void list_append(struct process_t *list, struct process_t *item);

/*
Creates a new process linked list head of type process_t
@params
pid, int, the process ID
arrival, int, the arrival time of process in Seconds
mem_needed, int, the amount of memory needed in KB
time_to_fin, int, the required time for process to finish in Seconds

@return
a process_t linked list head pointer
*/
struct process_t *create_process(int pid, int arrival, int mem_needed, int time_to_fin)
{
    struct process_t *new_p = malloc(sizeof(struct process_t));

    if (new_p == NULL)
    {
        fprintf(stderr, "Malloc failed!\n");
        exit(1);
    }

    new_p->pid = pid;
    new_p->arrival_time = arrival;
    new_p->memory_required = mem_needed;
    new_p->time_required = time_to_fin;
    new_p->next = NULL;

    return new_p; 
}

/*
Gets next process from the input file
@params
fptr, FILE *, the file pointer of the input file

@return
NULL, if EOF for the file has already been reached
a process_t struct of the new process read
*/
struct process_t *get_next_process(FILE *fptr)
{
    char c;
    int n = 0;
    int time, pid, mem, time_fin;
    int is_EOF = 1;
    char *buffer = (char *)malloc(sizeof(char)* SIZE_BUFFER);

    while (1)
    {
        c = getc(fptr);

        //returns NULL if file has already reached EOF
        if (is_EOF && c == EOF)
        {
            return NULL;
        }

        //If a newline or EOF is reached, process the entire line as ARRIVAL_TIME, PID, MEM_REQ, TIME_REQ
        if (c != '\n' && c != EOF)
        {
            buffer[n] = c;
            n ++;
            is_EOF = 0;
        }
        else
        {
            buffer[n] = '\0';
            sscanf(buffer, "%d %d %d %d", &time, &pid, &mem, &time_fin);
            printf("%d %d %d %d\n", time, pid, mem, time_fin);
            
            return create_process(pid, time, mem, time_fin);
        }
        
    }
}

struct process_t *get_last_process(struct process_t *list)
{
    struct process_t *curr = list;

    while (curr->next != NULL)
    {
        curr = curr->next;
    }

    return curr;
}

/*
Translates all process entries in the input file into process_t linked lists
!! FOR USE IN POPPING PROCESSES WHEN CPU CLOCK CORRESPONDS TO ARRIVAL TIME
!! WHEN RUNNING SIMULATION
@params
fptr, FILE *, the input file
list, struct process_t *, the head of a process_t linked list
*/
void get_all_processes(FILE *fptr, struct process_t *list)
{
    struct process_t *curr = list;

    while (1)
    {
        //Init the linked list if head is null
        if (curr == NULL)
        {
            curr = get_next_process(fptr);
            continue;
        }

        curr = curr->next;
        curr = get_next_process(fptr);

        //Check if file reached EOF
        if (curr == NULL)
        {
            return;
        }

    }
}

/*
Checks if cpu_clock is equal to a new process' arrival time,
if true, pops the process from a stored master list and appends to current process list
!! WILL ADD ALL SUBSEQUENT PROCESSES WITH SAME ARRIVAL TIME IF ABOVE IS TRUE
@params
cpu_clock, int, representation of CPU clock in Seconds
master, struct process_t *, a master linked list of all future processes of the simulation
curr_processes, struct process_t *, the linked list of current processes queued by CPU simulation

@return
an int, 0 if no new process arrival corresponds to cpu_clock, 1 if new processes added
*/
int has_process_arrived(int cpu_clock, struct process_t *master, struct process_t *curr_processes)
{
    struct process_t *last = NULL;

    //A new process has arrived
    if (cpu_clock == master->arrival_time)
    {
        list_append(curr_processes, list_pop(master));
    }

    return 0;
}

/*
Pops the first element from the linked list provided
@params
list, struct process_t *, a linked list

@return
a struct process_t *, the popped item from linked list
*/
struct process_t *list_pop(struct process_t *list)
{
    struct process_t *temp = list;

    list = list->next;

    return temp;
}

/*
Appends a process to the end of a linked list
@params
list, struct process_t *, the linked list to append to
item, struct process_t *, the process to append
*/
void list_append(struct process_t *list, struct process_t *item)
{
    struct process_t *last = get_last_process(list);

    last->next = item;
}