#ifndef JOB_H
#define JOB_H
#include <sys/types.h>

typedef struct {
    pid_t pid[100];
    int index;
    char command[1024];
} job_t;

job_t init_job();
void set_pid(job_t* j, pid_t pid);
void set_command(job_t* j, char* command);
pid_t * get_pids(job_t* j);
void get_command(job_t* j, char* command);
int equal_pid(job_t* j, pid_t* pid);
#endif
