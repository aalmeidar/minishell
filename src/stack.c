#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "job.h"
#include "stack.h"
#include "exec.h"

void push(stackJobs_t* s, job_t j) {
	node_t* newNode = (node_t*) malloc(sizeof(node_t));
	newNode->job = j;
	newNode->nextNode = s->top;
	s->top = newNode;
	s->size++;
}

job_t get(stackJobs_t* s, int index){
    node_t *node;
    int i;
    i = 0;
    node = s->top;
    while(node != NULL && i < index) {
        node = node->nextNode;
        i++;
    }
    return node->job;
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

void pop_pid(stackJobs_t *s, pid_t pid) {
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
	pid_t pid, i, j, finished, error, * pids;
	char c;
	char command[1024];
	node_t *node;
	i = size_stack(s);


	node = s->top;
	while (node != NULL) {
        finished = 0;
        error = 0;
        pids = get_pids(&(node->job));
        for (j = 0; j < node->job.index; j++){
            pid = waitpid(pids[j], NULL, WNOHANG);
            if (pid < 0) {
                // Error
                error++;
                continue;
            }else if (pid != 0){
                finished++;
            }
        }
        c = ' ';
        if(node == s->top->nextNode) {
            c = '-';
        } else if (node == s->top) {
            c = '+';
        }
        if (finished == node->job.index || error == node->job.index) { // El proceso ha terminado
			if(output) {
                get_command(&(node->job), command);
                printf("[%d]%c %d  Hecho\t\t\t%s\n", i, c, pids[node->job.index-1], command);
            }
			pop_pid(s, get_pids(&(node->job))[node->job.index-1]);
		} else if (output != 0) {
            get_command(&(node->job), command);
            printf("[%d]%c %d  Ejecutando\t\t\t%s\n", i, c, pids[node->job.index-1], command);
		}
        node = node->nextNode;
		i--;
	}
}


void fg_job_stack(char * arg, stackJobs_t * s){
    int size, i;
    job_t j;
    pid_t * pids;
    size = size_stack(s);
    if (arg == NULL){
        j = peek(s);
    }else{
        char * endptr;
        long parsed = strtol(arg, &endptr, 10);
        if (*endptr != '\0' || parsed < 1 || parsed > size || errno == ERANGE) {
            printf("[!] Número de trabajo en segundo plano inválido\n");
            return;
        }
        j = get(s ,((int)parsed) - 1);
    }
    pids = get_pids(&(j));
    pop_pid(s, pids[j.index-1]);
    signal(SIGINT, sig_handler);
    for (i = 0; i < j.index; i++){
        waitpid(pids[i], NULL, 0);
    }
}


stackJobs_t* init_stackJobs() {
    stackJobs_t* s = (stackJobs_t *) malloc(sizeof(stackJobs_t));
    s->top = NULL;
    s->size = 0;
    return s;
}



