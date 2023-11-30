#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "parser.h"

void exec(tcommand * c, int ncommands);

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
        exec(line->commands, line->ncommands);
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


void exec(tcommand *commands, int ncommands) {
    int pipes[ncommands - 1][2], i, j;
    // Se crean los pipes de la matriz de pipes y se comprueba si ha ocurrido algún error
    for (i = 0; i < ncommands - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            fprintf(stderr, "Error creando la tubería.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Por cada comando se crea un proceso
    for (i = 0; i < ncommands; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Fallo el fork().\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            if (i != 0) {
                // Si el hijo no es el primero comando leo la entrada del extremo de salida pipe i - 1
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i != ncommands - 1) {
                // Si el hijo no es el último comando escribo la salida del extremo de entrada pipe i
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Se cierran ambos extremos de todos los pipes
            for (j = 0; j < ncommands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Se ejecuta el comando correspondiente a la iteración con sus argumentos
            execvp(commands[i].argv[0], commands[i].argv);
            fprintf(stderr, "Error al ejecutar el comando %s.\n", commands[i].argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Se cierran ambos extremos de todos los pipes
    for (i = 0; i < ncommands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Se espera a que todos los hijos acaben
    for (i = 0; i < ncommands; i++) {
        wait(NULL);
    }
}
