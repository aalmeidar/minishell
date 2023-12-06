#include "job.h"
#include <string.h>
#include <sys/types.h>

job_t init_job(){
    job_t j;
    j.index = 0;
    return j;
}

void set_pid(job_t* j, pid_t pid) {
	j->pid[j->index] = pid;
    j->index++;
}

void set_command(job_t* j, char* command) {
	strcpy(j->command, command);
}

pid_t * get_pids(job_t* j) {
	return j->pid;
}

void get_command(job_t* j, char* command) {
	strcpy(command, j->command);
}

int equal_pid(job_t* j, pid_t* pid) {
	return j->pid == pid;
}
