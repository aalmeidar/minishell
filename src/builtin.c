#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "builtin.h"
#include "background.h"

void cd(char* dir) {
	// Se comprueba si hay argumento.
	if (dir == NULL) {
		// Si no hay argumento se cambia al directorio en la variable de entorno
		// HOME
		dir = getenv("HOME");
		if (dir == NULL) {
			fprintf(stderr, RED "[!] Error variable HOME not found\n" RESET);
			exit(EXIT_FAILURE);
		}
		if (chdir(dir) == -1) {
			fprintf(stderr, RED "[!] Error: %s\n" RESET, strerror(errno));
		}
		printf("%s\n", dir);
	}
	// Se ejecuta con el arguemento pasado
	if (chdir(dir) == -1) {
		fprintf(stderr, RED "[!] Error: %s\n" RESET, strerror(errno));
	}
}

void jobs() {
	check_jobs(1);
}
void fg(char* arg) {
	printf("fg");	
}
