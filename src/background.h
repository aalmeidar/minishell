#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "job.h"

void init_jobs();
void save_job(job_t* j);
void check_jobs(int output);
void fg_job(char * arg);
void delete_jobs();
#endif
