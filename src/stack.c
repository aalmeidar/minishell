#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "job.h"
#include "stack.h"

void push(stackJobs_t* s, job_t j) {
	node_t* newNode = (node_t*) malloc(sizeof(node_t));
	newNode->job = j;
	newNode->nextNode = s->top;
	s->top = newNode;
	s->size++;
}

job_t pop(stackJobs_t* s) {
	node_t* node;
	job_t job;
	node = s->top;
	s->top = node->nextNode;
	s->size--;
	job = node->job;
	free(node);
	return job;
}

job_t peek(stackJobs_t* s) {
	return s->top->job;
}

int size_stack(stackJobs_t* s) {
	return s->size;
}

int is_empty(stackJobs_t* s) {
	return s->top == NULL && s->size == 0;
}

void delete_stack(stackJobs_t* s) {
	while(!is_empty(s)) {
		pop(s);
	}
    free(s);
}

//void get_all_pids(stackJobs_t *s, pid_t *pids) {
//	int i;
//	node_t* node;
//	node = s->top;
//	for(i = 0; i < size_stack(s); i++) {
//		pids[i] = get_pid(&(node->job));
//		node = node->nextNode;
//	}
//}

void pop_pid(stackJobs_t *s, pid_t* pid) {
	node_t *prev, *node;
    int deleted;
    deleted = 0;
	prev = NULL;
	node = s->top;
	while(node != NULL && !deleted) {
		if(equal_pid(&(node->job), pid)) {
            if (prev != NULL) {
                prev->nextNode = node->nextNode;
            } else {
                s->top = node->nextNode;
            }
            deleted = 1;
            free(node);
            s->size--;
		} else {
			prev = node;
			node = node->nextNode;
		}
	}
}

// Si output es distinto de 0, se saca por STDOUT log.
void check_jobs_stack(stackJobs_t* s, int output) {
	pid_t pid, i, j, finished = 0, * pids;
	char c;
	char command[1024];
	node_t *node;
	i = size_stack(s);


	node = s->top;
	while (node != NULL) {
        pids = get_pids(&(node->job));
        for (j = 0; j < node->job.index; j++){
            pid = waitpid(pids[j], NULL, WNOHANG);
            if (pid < 0) {
                // Error
                continue;
            }else if (pid != 0){
                finished++;
            }
        }
        if (finished == node->job.index) { // El proceso ha terminado
			if(output) {
                get_command(&(node->job), command);
                printf("[%d] %d  Hecho\t\t\t%s\n", i, pid, command);
            }
			pop_pid(s, get_pids(&(node->job)));
		} else if (output != 0) {
			c = ' ';
			if(node == s->top->nextNode) {
				c = '-';
			} else if (node == s->top) {
				c = '+';
			}
            get_command(&(node->job), command);
            printf("[%d]%c %d  Ejecutando\t\t\t%s\n", i, c, pid, command);
		}
        node = node->nextNode;
		i--;
	}
}
stackJobs_t* init_stackJobs() {
    stackJobs_t* s = (stackJobs_t *) malloc(sizeof(stackJobs_t));
    s->top = NULL;
    s->size = 0;
    return s;
}


