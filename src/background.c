#include "job.h"
#include "background.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "stack.h"

stackJobs_t* jobs_stack;

void init_jobs(){
    jobs_stack = init_stackJobs();
}

void save_job(job_t* j) {
    push(jobs_stack, *j);
}


void check_jobs(int output) {
	check_jobs_stack(jobs_stack, output);
}

void delete_jobs() {
	delete_stack(jobs_stack);
}

void fg_job(char * arg){
    if (!is_empty(jobs_stack)){
        fg_job_stack(arg, jobs_stack);
    }
}

