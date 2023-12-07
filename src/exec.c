#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "exec.h"
#include "redirect.h"
#include "utils.h"
#include "builtin.h"
#include "background.h"
#include "job.h"

pid_t pid;

void restore_line(tline* line, char* command) {
	int i, j;
	command[0] = '\0';
	for (i = 0; i < line->ncommands; i++){
            for (j = 0; j < line->commands[i].argc; j++){
                strcat(command, line->commands[i].argv[j]);
                strcat(command, " ");
            }
            if (i == 0 && line->redirect_input){
                strcat(command, "< ");
                strcat(command, line->redirect_input);
                strcat(command, " ");
            }
            if (i + 1 != line->ncommands){
                strcat(command, "| ");
            } else {
                if (line->redirect_output){
                    strcat(command, " > ");
                    strcat(command, line->redirect_output);
                    strcat(command, " ");
                }
                if (line->redirect_error){
                    strcat(command, " &> ");
                    strcat(command, line->redirect_error);
                    strcat(command, " ");
                }
            }
        }
	strcat(command, "&");
}

void sig_handler(int sig){
    if (pid == 0){
        kill(getppid(), SIGKILL);
    }
}

void exec_line(tline* line) {
	int pipes[line->ncommands - 1][2], i, j, saved_std[3];
	char command[1024];
	job_t job;

    if (line->background == 0){
        signal(SIGINT, sig_handler);
    }

	// Redireccionar STDIN, STDOUT, STDERR.
    if (line->background == 1 || line->redirect_input != NULL || line->redirect_output != NULL || line->redirect_error != NULL){

		if (line->redirect_input != NULL) {
            if ((saved_std[STDIN_FILENO] = redirect_input(line->redirect_input)) == -1) {
                fprintf(stderr, RED "[!] Error: %s" RESET, strerror(errno));
                exit(EXIT_FAILURE);
            }
		}
		if (line->redirect_output != NULL) {
            if ((saved_std[STDOUT_FILENO] = redirect_output(line->redirect_output)) == -1) {
                fprintf(stderr, RED "[!] Error: %s" RESET, strerror(errno));
                exit(EXIT_FAILURE);
            }
		}
		if (line->redirect_error != NULL) {
            if ((saved_std[STDERR_FILENO] = redirect_error(line->redirect_error)) == -1) {
                fprintf(stderr, RED "[!] Error: %s" RESET, strerror(errno));
                exit(EXIT_FAILURE);
            }
		}
    }

	// Redireccionar los errores si está indicado
    if (line->redirect_error != NULL) {
    	saved_std[STDERR_FILENO] = redirect_error(line->redirect_error);
		if (saved_std[STDERR_FILENO] == -1) {
			fprintf(stderr,  RED "[!] Error: %s" RESET, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

    // Se crean los pipes de la matriz de pipes y se comprueba si ha ocurrido algún error
    for (i = 0; i < line->ncommands - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            fprintf(stderr, RED "[!] Error creando la tubería.\n" RESET);
            exit(EXIT_FAILURE);
        }
    }

    // Por cada comando se crea un proceso
    for (i = 0; i < line->ncommands; i++) {
        job = init_job();
        pid = fork();

        if (pid < 0) {
            fprintf(stderr, RED "[!] Fallo el fork().\n" RESET);
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
            fprintf(stderr, RED "[!] Error al ejecutar el comando %s.\n" RESET, line->commands[i].argv[0]);
            exit(EXIT_FAILURE);
        }else {
			if (line->background == 1) {
                set_pid(&job, pid);
			}
		}
    }

    // Se cierran ambos extremos de todos los pipes en el proceso padre
    for (i = 0; i < line->ncommands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
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

	// Se espera a que todos los hijos acaben si no esta en background
	if (line->background == 0) {
		for(i = 0; i < line->ncommands; i++) {
			wait(NULL);
		}
	} else {
		restore_line(line, command);
        printf("[%d]+ Running\t\t\t", pid);
        printf("%s\n", command);
		set_command(&job, command);
		save_job(&job);
	}
}
