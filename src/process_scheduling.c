#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "../include/utilities.h"

#define SIZE_BUFFER 256

struct process_t *get_next_process(FILE *fptr);
struct process_t *list_pop(struct process_t **);



void print_list(struct process_t *list)
{
    struct process_t *curr = list;
    while(curr != NULL)
    {
        printf("NEW process: %d %d %d %d %d\n", curr->arrival_time, curr->pid, curr->memory_required, curr->time_required, curr->time_finished);
        curr = curr->next;
    }
    printf("\n\n");
}

/*
Creates a new process linked list head of type process_t
@params
pid, uint32_t, the process ID
arrival, uint32_t, the arrival time of process in Seconds
mem_needed, uint32_t, the amount of memory needed in KB
time_to_fin, uint32_t, the required time for process to finish in Seconds

@return
a process_t linked list head pointer
*/
struct process_t *create_process(uint32_t pid, uint32_t arrival, uint32_t mem_needed, uint32_t time_to_fin)
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
    new_p->time_finished = 0;
    new_p->next = NULL;

    return new_p; 
}

/*
Traverse to the last element of a linked list
@params
list, struct process_t *, the linked list of processes

@return
a struct process_t * of last process
*/
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
Frees up memory used by the linked list
@params
list, struct process_t *, head of linked list to be freed
*/
void free_list(struct process_t *list)
{
    struct process_t *curr = list;

    //Case for empty list
    if (!list)
    {
        return;
    }

    //Case for singleton element
    if (list->next == NULL)
    {
        free(list);
    }

    while(list != NULL)
    {
        curr = list;
        list = list->next;
        free(curr);
    }
}

/*
Counts the number of processes in a linked list
@params
list, struct process_t *, the linked list

@return
uint32_t, number of processes in linked list
*/
uint32_t count_processes(struct process_t *list)
{
    struct process_t *curr = list;
    uint32_t n = 1;

    //Returns 0 for empty list and 1 for singleton list
    if (!list)
    {
        return 0;
    }
    else if (list->next == NULL)
    {
        return n;
    }

    while(curr->next != NULL)
    {
        n += 1;
        curr = curr->next;
    }
    return n;
}

/*
Translates all process entries in the input file into process_t linked lists
!! FOR USE IN POPPING PROCESSES WHEN CPU CLOCK CORRESPONDS TO ARRIVAL TIME
!! WHEN RUNNING SIMULATION
@params
fptr, FILE *, the input file

@return
head struct pointer of type struct process_t * for linked list
*/
struct process_t *get_all_processes(FILE *fptr)
{
    struct process_t *head = malloc(sizeof(struct process_t));
    struct process_t *curr = NULL;
    struct process_t *new_process = NULL;
    uint32_t time = 0, pid = 0, mem = 0, time_fin = 0, is_head = 1;

    while (fscanf(fptr, "%"SCNd32" %"SCNd32" %"SCNd32" %"SCNd32" ", &time, &pid, &mem, &time_fin) == 4)
    {
        //Add first element into head
        if (is_head)
        {
            new_process = malloc(sizeof(struct process_t));
            new_process = create_process(pid, time, mem, time_fin);
            head = new_process;
            curr = head;
            is_head = 0;
            continue;
        }   
        
        //Iterate to end of list
        while (curr->next != NULL)
        {
            curr = curr->next;
        }
        
        //Append new process to end
        new_process = create_process(pid, time, mem, time_fin);
        curr->next = new_process;
    }

    fclose(fptr);
    return head;
}

/*
Checks if cpu_clock is equal to a new process' arrival time,
if true, pops the process from a stored master list and appends to current process list
!! WILL ADD ALL SUBSEQUENT PROCESSES WITH SAME ARRIVAL TIME IF ABOVE IS TRUE
@params
cpu_clock, uint32_t, representation of CPU clock in Seconds
master, struct process_t *, a master linked list of all future processes of the simulation
curr_processes, struct process_t *, the linked list of current processes queued by CPU simulation

@return
an int, 0 if no new process arrival corresponds to cpu_clock, 1 if new processes added
*/
int has_process_arrived(uint32_t cpu_clock, struct process_t *master)
{
    //If no new process corresponds to the cpu clock
    if (cpu_clock != master->arrival_time)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*
Shuffles the process list as per Round-Robin scheduling
@params
list, struct process_t *, linked list of processes 

@return
struct process_t *, the new linked list after round robin
*/
struct process_t *round_robin_shuffle(struct process_t *list)
{
    
    struct process_t *new_head = NULL;
    struct process_t *temp = NULL;
    struct process_t *end = list;

    //Returns if empty list
    if(!list)
    {
        return NULL;
    }

    //Case for singleton list
    if (list->next == NULL)
    {
        return list;
    }

    //Store 2nd element as the new head
    new_head = list->next;
    temp = new_head;
    
    //Iterate to the end of linked list
    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    //Insert old list head at the end of list
    temp->next = end;
    temp->next->next = NULL;

    return new_head;
}

/*
Decrement time remaining for process to use CPU.
@params
list, struct process_t **, pointer to the process_t linked list

@return
int, 1 if a process finished executing, else 0
*/
int execute_process(struct process_t **list)
{
    (*list)->time_required -= 1;

    if ((*list)->time_required <= 0)
    {
        return 1;
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
struct process_t *list_pop(struct process_t **list)
{

    struct process_t *temp = *list;

    if (temp)
    {
        *list = temp->next;
    }
    else
    {
        return NULL;
    }

    temp->next = NULL;

    return temp;
}

/*
Appends a process to the end of a linked list
!! Appends before end of list if multiple processes with same arrival time
!! (sorts on ascending pid order)
@params
list, struct process_t *, the linked list to append to
item, struct process_t *, the process to append
*/
struct process_t *list_push(struct process_t *list, struct process_t *item)
{
    struct process_t *temp = malloc(sizeof(struct process_t));
    struct process_t *curr = list;
    
    temp = item;
    
    //Inserts in ascending order of pid if more than 1 process arrived at 
    //the same time. Compares arrival time of an element already in the 
    //linked list and its subsequent process' pid (For case of 1 element)
    if (temp->arrival_time == curr->arrival_time && curr->next == NULL)
    {
        if (temp->pid < curr->pid)
        {
            temp->next = curr;
            return temp;
        }
    }

    while (curr->next != NULL)
    {
        //Inserts in ascending order of pid if more than 1 process arrived at 
        //the same time. Compares arrival time of an element already in the 
        //linked list and its subsequent process' pid
        if (temp->arrival_time == curr->next->arrival_time && curr->next->pid > temp->pid)
        {
            struct process_t *node = NULL;
            node = curr->next;

            curr->next = temp;
            temp-> next = node;

            return list;
        }
        curr = curr->next;
    }
    
    curr->next = temp;

    return list;

}