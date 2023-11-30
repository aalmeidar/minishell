#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "exec.h"
#include "redirect.h"

void exec_line(tline* line) {
	int pipes[line->ncommands - 1][2], i, j, saved_std[3], dev_null;

    if (line->background == 1){
        dev_null = open("/dev/null", O_WRONLY);
        saved_std[0] = dup(0);
        if (line->redirect_output == NULL) {
            saved_std[1] = dup(1);
            dup2(dev_null, 1);
        }
        saved_std[2] = dup(2);
        dup2(dev_null, 0);
        dup2(dev_null, 2);
    }
	
	// Redireccionar el input si está indicado
    if (line->redirect_input != NULL) {
    	saved_std[STDIN_FILENO] = redirect_input(line->redirect_input);
		if (saved_std[STDIN_FILENO] == -1) {
			fprintf(stderr, "[!] Error: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	// Redireccionar el output si está indicado
    if (line->redirect_output != NULL) {
    	saved_std[STDOUT_FILENO] = redirect_output(line->redirect_output);
		if (saved_std[STDOUT_FILENO] == -1) {
			fprintf(stderr, "[!] Error: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
    }

	// Redireccionar los errores si está indicado
    if (line->redirect_error != NULL) {
    	saved_std[STDERR_FILENO] = redirect_error(line->redirect_error);
		if (saved_std[STDERR_FILENO] == -1) {
			fprintf(stderr, "[!] Error: %s", strerror(errno));
			exit(EXIT_FAILURE);
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
        }
    }

    // Se cierran ambos extremos de todos los pipes en el proceso padre
    for (i = 0; i < line->ncommands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Se espera a que todos los hijos acaben si no esta en background
    if (line->background == 0){
        for (i = 0; i < line->ncommands; i++) {
            wait(NULL);
        }
    }else{
        printf("[%d]+ Running ", getpid());
        for (i = 0; i < line->ncommands; i++){
            for (j = 0; j < line->commands[i].argc; j++){
                printf("%s ", line->commands[i].argv[j]);
            }
            if (i + 1 != line->ncommands){
                printf("| ");
            }
        }
        printf("&\n");
        dup2(saved_std[0], 0);
        dup2(saved_std[1], 2);
        dup2(saved_std[2], 2);
        close(saved_std[0]);
        close(saved_std[1]);
        close(saved_std[2]);
    }

	// Si se ha redireccionado el input, volver al estado original
    if (line->redirect_input != NULL){
    	if (redirect_std_fd(STDIN_FILENO, saved_std[STDIN_FILENO]) == -1){
			fprintf(stderr, "[!] Error: %s", strerror(errno));	
			exit(EXIT_FAILURE);
		}
	}
	// Si se ha redireccionado el output, volver al estado original
    if (line->redirect_output != NULL){
    	if (redirect_std_fd(STDOUT_FILENO, saved_std[STDOUT_FILENO]) == -1){
			fprintf(stderr, "[!] Error: %s", strerror(errno));	
			exit(EXIT_FAILURE);
		}
    }
	// Si se han redireccionado los errores, volver al estado original
    if (line->redirect_error != NULL){
    	if (redirect_std_fd(STDERR_FILENO, saved_std[STDERR_FILENO]) == -1){
			fprintf(stderr, "[!] Error: %s", strerror(errno));	
			exit(EXIT_FAILURE);
		}
    }
}
