#include "stack.h"
#include "job.h"
#include "exec.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

stackJobs_t* init_stackJobs() {
    stackJobs_t *s = (stackJobs_t*) malloc(sizeof(stackJobs_t));
    s->top = NULL;
    s->bot = NULL;
    s->size = 0;
    return s;
}

void push(stackJobs_t* s, job_t j){
    node_t* new_node = (node_t*) malloc(sizeof(node_t));
    new_node->job = j;
    if (is_empty(s)) {
        s->bot = new_node;
    } else {
        s->top->prev = new_node;
    }
    new_node->next = s->top;
    new_node->prev = NULL;
    s->top = new_node;
    s->size++;
}

job_t pop(stackJobs_t* s) {
    node_t* deleted;
    job_t job;
    if (!is_empty(s)) {
        deleted = s->top;
        job = deleted->job;
        if (size_stack(s) > 1) {
            deleted->next->prev = NULL;
        } else {
            s->bot = NULL;
        }
        s->top = deleted->next;
        s->size--;
        free(deleted);
        return job;
    }
}

job_t get(stackJobs_t* s, int index) {
    int m, i;
    node_t* node;
    m = (size_stack(s)/2);
    if (index >= m && index < size_stack(s)) {
        i = size_stack(s)-1;
        node = s->top;
        while (i != index) {
            node = node->next;
            i--;
        }
    } else if (m > index) {
        i = 0;
        node = s->bot;
        while (i != index) {
            node = node->prev;
            i--;
        }
    }
    return node->job;
}

job_t peek(stackJobs_t* s) {
    return s->top->job;
}

int size_stack(stackJobs_t* s) {
    return s->size;
}


int is_empty(stackJobs_t* s) {
    return s->top == NULL && s->bot == NULL && s->size == 0;
}

void delete_stack(stackJobs_t* s) {
    while (!is_empty(s)) {
        pop(s);
    }
    free(s);
}


void pop_pid(stackJobs_t* s, pid_t pid) {
    node_t *node, *tmp;
    node = s->top;
    while (node != NULL && !equal_pid(&(node->job), pid)) {
        node = node->next;
    }

    if (node != NULL) {
        tmp = node;
        if (node->prev != NULL) {
            node->prev->next = tmp->next;
        }
        if (node->next != NULL) {
            node->next->prev = tmp->prev;
        }
        if (s->top == node) {
            s->top = tmp->next;
        }
        if (s->bot == node) {
            s->bot = tmp->prev;
        }
        free(node);
        s->size--;
    }

}

void check_jobs_stack(stackJobs_t* s, int output) {
    int i, j, error, finished, deleted;
    pid_t *pids, pid;
    node_t *node, *tmp;
    char c, command[1024];
    i = 1;
    tmp = NULL;
    node = s->bot;

    while (node != NULL) {
        finished = 0;
        error = 0;
        pids = get_pids(&(node->job));
        for (j = 0; j < node->job.index; j++) {
            pid = waitpid(pids[j], NULL, WNOHANG);
            if (pid < 0) { // Error
                error++;
                continue;
            } else if  (pid != 0) {
                finished++;
            }
        }
        tmp = node->prev;
        c = ' ';
        if (node == s->top->next) {
            c = '-';
        } else if (node == s->top) {
            c = '+';
        }
        if (finished == node->job.index || error == node->job.index) { // El proceso ha terminado
            get_command(&(node->job), command);
            printf("[%d]%c %d  Hecho\t\t\t%s\n", i, c, pids[node->job.index-1], command);

            pop_pid(s, get_pids(&(node->job))[node->job.index-1]);
        } else if (output != 0) {
            get_command(&(node->job), command);
            printf("[%d]%c %d  Ejecutando\t\t\t%s\n", i, c, pids[node->job.index-1], command);
        }
        node = tmp;
        i++;
    }
}

void fg_job_stack(char * arg, stackJobs_t * s) {
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
    printf("%s\n", j.command);
    signal(SIGINT, sig_handler);
    set_pgid_fg(pids[0]);
    for (i = 0; i < j.index; i++){
        waitpid(pids[i], NULL, 0);
    }
}
