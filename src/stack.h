#ifndef STACK_H
#define STACK_H

#include <sys/types.h>
#include "job.h"

typedef struct node {
    job_t job;
    struct node *next;
    struct node *prev;
} node_t;

typedef struct {
    node_t* top;
    node_t* bot;
    int size;

} stackJobs_t;

stackJobs_t* init_stackJobs();
void push(stackJobs_t* s, job_t j);
job_t pop(stackJobs_t* s);
job_t get(stackJobs_t* s, int index);
job_t peek(stackJobs_t* s);
int size_stack(stackJobs_t* s);
int is_empty(stackJobs_t* s);
void delete_stack(stackJobs_t* s);
void pop_pid(stackJobs_t* s, pid_t pid);
void check_jobs_stack(stackJobs_t* s, int output);
void fg_job_stack(char * arg, stackJobs_t * s);


#endif
