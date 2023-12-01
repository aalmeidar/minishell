#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "parser.h"

typedef struct {
    pid_t pid;
    char command[1024];
} job_t;

job_t jobs_stack[30];
int num_jobs = 0;

void exec(tline * line);
void jobs();
void fg(char * arg);

int main(void){
    char buf[1024];
    tline * line;

    printf("msh > ");
    while (fgets(buf, 1024, stdin)) {
        line = tokenize(buf);
        if (line==NULL) {
            continue;
        }
        if (strcmp(line->commands[0].argv[0], "jobs") == 0) {
            jobs();
            printf("msh > ");
            continue;
        }
        if (strcmp(line->commands[0].argv[0], "fg") == 0) {
            if (line->commands[0].argc > 1){
                fg((line->commands[0].argv[1]));
            }else{
                fg(NULL);
            }
            printf("msh > ");
            continue;
        }
        exec(line);
        printf("msh > ");
    }
    return 0;
}

void jobs(){
    int i, j;
    if (num_jobs > 0){
        int state;
        for (j = 0; j < num_jobs; j++){
            state = waitpid(jobs_stack[j].pid, NULL, WNOHANG);
            if (state < 0){
                continue;
            }
            else if (state != 0 ){
                printf("[%d] %d  Hecho\t\t\t%s", j + 1, jobs_stack[j].pid, jobs_stack[j].command);
                for (i = j; i < num_jobs; i++){
                    jobs_stack[i].pid = jobs_stack[i + 1].pid;
                    strcpy(jobs_stack[i].command, jobs_stack[i + 1].command);
                }
                num_jobs--;
            }else{
                if (j + 2 == num_jobs){
                    printf("[%d]- %d  Ejecutando\t\t\t%s", j + 1, jobs_stack[j].pid, jobs_stack[j].command);
                }else if(j + 1 == num_jobs){
                    printf("[%d]+ %d  Ejecutando\t\t\t%s", j + 1, jobs_stack[j].pid, jobs_stack[j].command);
                }else{
                    printf("[%d] %d  Ejecutando\t\t\t%s", j + 1, jobs_stack[j].pid, jobs_stack[j].command);
                }
            }
        }
    }
}

void fg(char * arg){
    int pid, job_id, status;
    if (arg == NULL){
        job_id = num_jobs-1;
        pid = jobs_stack[num_jobs - 1].pid;
    }else{
        char * endptr;
        long parsed = strtol(arg, &endptr, 10);
        if (*endptr != '\0' || parsed < 1 || parsed > num_jobs || errno == ERANGE) {
            printf("Número de trabajo en segundo plano inválido\n");
            return;
        }
        job_id = ((int)parsed) - 1;
        pid = jobs_stack[job_id].pid;
    }
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)){
        for (int i = job_id; i < num_jobs; i++){
            jobs_stack[i].pid = jobs_stack[i + 1].pid;
            strcpy(jobs_stack[i].command, jobs_stack[i + 1].command);
        }
        num_jobs--;
    }
}


void exec(tline * line) {
    int pipes[line->ncommands - 1][2], i, j, saved_std[3], dev_null, input_fd, output_fd;

    if (line->background == 1 || line->redirect_input != NULL || line->redirect_output != NULL || line->redirect_error != NULL){
        saved_std[0] = dup(STDIN_FILENO);
        saved_std[1] = dup(STDOUT_FILENO);
        saved_std[2] = dup(STDERR_FILENO);
        if (line->background == 1){
            dev_null = open("/dev/null", O_WRONLY);
            dup2(dev_null, STDIN_FILENO);
            dup2(dev_null, STDOUT_FILENO);
            dup2(dev_null, STDERR_FILENO);
        }
        if (line->redirect_input != NULL) {
            input_fd = open(line->redirect_input, O_RDONLY);
            if (input_fd < 0) {
                fprintf(stderr, "[!] Error al abrir el archivo de entrada %s.\n", line->redirect_input);
                exit(EXIT_FAILURE);
            }
            dup2(input_fd, STDIN_FILENO);
        }
        if (line->redirect_output != NULL || line->redirect_error != NULL){
            output_fd = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd < 0) {
                fprintf(stderr, "[!] Error al abrir el archivo de salida %s.\n", line->redirect_output);
                exit(EXIT_FAILURE);
            }
            if (line->redirect_output != NULL) {
                dup2(output_fd, STDOUT_FILENO);
            }
            if (line->redirect_error != NULL) {
                dup2(output_fd, STDERR_FILENO);
            }
        }
    }

    // Se crean los pipes de la matriz de pipes y se comprueba si ha ocurrido algún error
    for (i = 0; i < line->ncommands - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            fprintf(stderr, "[!] Error creando la tubería.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Por cada comando se crea un proceso
    for (i = 0; i < line->ncommands; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "[!] Fallo el fork().\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            if (i != 0) {
                // Si el hijo no es el primero comando leo la entrada del extremo de salida pipe i - 1
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i != line->ncommands - 1) {
                // Si el hijo no es el último comando escribo la salida del extremo de entrada pipe i
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Se cierran ambos extremos de todos los pipes en los procesos hijos
            for (j = 0; j < line->ncommands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Se ejecuta el comando correspondiente a la iteración con sus argumentos
            execvp(line->commands[i].argv[0], line->commands[i].argv);
            fprintf(stderr, "[!] Error al ejecutar el comando %s.\n", line->commands[i].argv[0]);
            exit(EXIT_FAILURE);
        }else{
            if (line->background == 1){
                jobs_stack[num_jobs].pid = pid;
            }
        }
    }

    // Se cierran ambos extremos de todos los pipes en el proceso padre
    for (i = 0; i < line->ncommands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    if (line->background == 1 || line->redirect_input != NULL || line->redirect_output != NULL || line->redirect_error != NULL){
        dup2(saved_std[0], STDIN_FILENO);
        dup2(saved_std[1], STDOUT_FILENO);
        dup2(saved_std[2], STDERR_FILENO);
        close(saved_std[0]);
        close(saved_std[1]);
        close(saved_std[2]);
    }

    // Se espera a que todos los hijos acaben si no esta en background
    if (line->background == 0){
        for (i = 0; i < line->ncommands; i++) {
            wait(NULL);
        }
    }else{
        char command[1024];
        command[0] = '\0';
        printf("[%d]+ Running ", jobs_stack[num_jobs].pid);
        for (i = 0; i < line->ncommands; i++){
            for (j = 0; j < line->commands[i].argc; j++){
                strcat(command, line->commands[i].argv[j]);
                strcat(command, " ");
            }
            if (i + 1 != line->ncommands){
                strcat(command, "| ");
            }
        }
        strcat(command, "&\n");
        printf("%s", command);
        strncpy(jobs_stack[num_jobs].command, command, 1024);
        num_jobs++;
    }

    if (num_jobs > 0){
        int state;
        for (j = 0; j < num_jobs; j++){
            state = waitpid(jobs_stack[j].pid, NULL, WNOHANG);
            if (state < 0){
                continue;
            }
            else if (state != 0 ){
                for (i = j; i < num_jobs; i++){
                    jobs_stack[i].pid = jobs_stack[i + 1].pid;
                    strcpy(jobs_stack[i].command, jobs_stack[i + 1].command);
                }
                num_jobs--;
            }
        }
    }
}