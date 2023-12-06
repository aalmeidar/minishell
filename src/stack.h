#ifndef STACK_H
#define STACK_H

#include "background.h"
#include "job.h"

typedef struct node {
	job_t job;
	struct node* nextNode;
} node_t;


typedef struct stack{
	node_t* top;
	int size;
} stackJobs_t;

stackJobs_t* init_stackJobs();
void push(stackJobs_t* s, job_t b);
job_t pop(stackJobs_t* s);
job_t get(stackJobs_t* s, int index);
job_t peek(stackJobs_t* s);
int size_stack(stackJobs_t* s);
int is_empty(stackJobs_t* s);
void delete_stack(stackJobs_t* s);
void get_all_pids(stackJobs_t* s, pid_t* pids);
void pop_pid(stackJobs_t* s, pid_t pid);
void check_jobs_stack(stackJobs_t* s, int output);
void fg_job_stack(char * arg, stackJobs_t * s);
#endif

