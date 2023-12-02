#include "job.h"
#include <string.h>
#include <sys/types.h>

void set_pid(job_t* j, pid_t pid) {
	j->pid = pid;
}

void set_command(job_t* j, char* command) {
	strcpy(j->command, command);
}

pid_t get_pid(job_t* j) {
	return  j->pid;
}

void get_command(job_t* j, char* command) {
	strcpy(command, j->command);
}

int equal_pid(job_t* j, pid_t* pid) {
	return j->pid == *(pid);
}
