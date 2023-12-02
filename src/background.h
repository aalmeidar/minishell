#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "job.h"

void init_jobs();
void save_job(job_t* j);
void check_jobs(int output);
void delete_jobs();
#endif
