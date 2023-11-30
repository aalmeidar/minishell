#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "parser.h"

void one_command_exec(char * argv[]);
void two_command_exec(char * argv1[], char * argv2[]);
void several_command_exec(tcommand * c, int ncommands);

int main(void){
    char buf[1024];
    tline * line;
    int i,j;

    printf("msh > ");
    while (fgets(buf, 1024, stdin)) {

        line = tokenize(buf);
        if (line==NULL) {
            continue;
        }
        if (line->redirect_input != NULL) {
            printf("redirección de entrada: %s\n", line->redirect_input);
        }
        if (line->redirect_output != NULL) {
            printf("redirección de salida: %s\n", line->redirect_output);
        }
        if (line->redirect_error != NULL) {
            printf("redirección de error: %s\n", line->redirect_error);
        }
        if (line->background) {
            printf("comando a ejecutarse en background\n");
        }
        if (line->ncommands == 1){
            one_command_exec(line->commands[0].argv);
        }
        if (line->ncommands == 2){
            two_command_exec(line->commands[0].argv, line->commands[1].argv);
        }
        if (line->ncommands > 2){
            several_command_exec(line->commands, line->ncommands);
        }
        for (i=0; i<line->ncommands; i++) {
            printf("orden %d (%s):\n", i, line->commands[i].filename);

            for (j=0; j<line->commands[i].argc; j++) {
                printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
            }
        }
        printf("msh > ");
    }
    return 0;
}

void one_command_exec(char * argv[]){
    pid_t pid;
    pid = fork();
    if (pid < 0){
        fprintf(stderr, "Falló el fork()");
        exit(-1);
    }else if (pid == 0){
        execvp(argv[0], argv);
        fprintf(stderr, "Se ha producido un error.\n");
        exit(1);
    } else {
        wait(NULL);
    }
}


void two_command_exec(char * argv1[], char * argv2[]){
    pid_t pid,pipe_c1_c2[2];
    if (pipe(pipe_c1_c2) == -1) {
        fprintf(stderr, "Error al crear las tuberias");
        exit(-1);
    }

    pid = fork();
    if (pid < 0){
        fprintf(stderr, "Fallo el fork()");
        exit(-1);
    }else if (pid == 0){
        close(pipe_c1_c2[0]);
        dup2(pipe_c1_c2[1], STDOUT_FILENO);
        execvp(argv1[0], argv1);
        fprintf(stderr, "Se ha producido un error.\n");
        exit(1);
    }

    pid = fork();
    if (pid < 0){
        fprintf(stderr, "Fallo el fork()");
        exit(-1);
    }else if (pid == 0){
        close(pipe_c1_c2[1]);
        dup2(pipe_c1_c2[0], STDIN_FILENO);
        execvp(argv2[0], argv2);
        fprintf(stderr, "Se ha producido un error.\n");
        exit(1);
    }

    close(pipe_c1_c2[0]);
    close(pipe_c1_c2[1]);
    wait(NULL);
    wait(NULL);
}

void several_command_exec(tcommand * commands, int ncommands){
    //tcommand * commands = (tcommand*) malloc(sizeof(tcommand)*ncommands);
    pid_t pid, first_pipe[2], second_pipe[2];
    for (int i = 0; i < ncommands; i++){
        pid = fork();
        if (pid < 0){
            fprintf(stderr, "Fallo el fork()");
            exit(-1);
        }else if (pid == 0){
            if (i == 0){
                close(first_pipe[0]);
                close(second_pipe[0]);
                close(second_pipe[1]);
                dup2(first_pipe[1], STDOUT_FILENO);
            }else if (i+1 == ncommands){
                close(first_pipe[0]);
                close(first_pipe[1]);
                close(second_pipe[1]);
                dup2(second_pipe[0], STDIN_FILENO);
            }else{
                close(first_pipe[1]);
                close(second_pipe[0]);
                dup2(first_pipe[0], STDIN_FILENO);
                dup2(second_pipe[1], STDOUT_FILENO);
            }
            execvp(commands[i].argv[0], commands[i].argv);
            fprintf(stderr, "Se ha producido un error.\n");
            exit(1);
        }else {
            wait(NULL);
        }
    }

    close(first_pipe[0]);
    close(first_pipe[1]);
    close(second_pipe[0]);
    close(second_pipe[1]);
}