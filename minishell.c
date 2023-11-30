#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "parser.h"

void exec(tline * line);

int main(void){
    char buf[1024];
    tline * line;

    printf("msh > ");
    while (fgets(buf, 1024, stdin)) {
        line = tokenize(buf);
        if (line==NULL) {
            continue;
        }
        exec(line);
        printf("msh > ");
    }
    return 0;
}


void exec(tline * line) {
    int pipes[line->ncommands - 1][2], i, j;

    if (line->redirect_input != NULL) {
        int input_fd = open(line->redirect_input, O_RDONLY);
        if (input_fd < 0) {
            fprintf(stderr, "[!] Error al abrir el archivo de entrada %s.\n", line->redirect_input);
            exit(EXIT_FAILURE);
        }
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }


    if (line->redirect_output != NULL) {
        int output_fd = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd < 0) {
            fprintf(stderr, "[!] Error al abrir el archivo de salida %s.\n", line->redirect_output);
            exit(EXIT_FAILURE);
        }
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }

    if (line->redirect_error != NULL) {
        int output_fd = open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd < 0) {
            fprintf(stderr, "[!] Error al abrir el archivo de salida %s.\n", line->redirect_error);
            exit(EXIT_FAILURE);
        }
        dup2(output_fd, STDERR_FILENO);
        close(output_fd);
    }

    // Se crean los pipes de la matriz de pipes y se comprueba si ha ocurrido algún error
    for (i = 0; i < line->ncommands - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            fprintf(stderr, "Error creando la tubería.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Por cada comando se crea un proceso
    for (i = 0; i < line->ncommands; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Fallo el fork().\n");
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
            fprintf(stderr, "Error al ejecutar el comando %s.\n", line->commands[i].argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Se cierran ambos extremos de todos los pipes en el proceso padre
    for (i = 0; i < line->ncommands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Se espera a que todos los hijos acaben
    for (i = 0; i < line->ncommands; i++) {
        wait(NULL);
    }
}

