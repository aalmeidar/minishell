#include "builtin.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cd(char* dir) {
	if (dir == NULL) {
		dir = getenv("HOME");
		if (dir == NULL) {
			fprintf(stderr, "[!] Error variable HOME not found\n");
			exit(EXIT_FAILURE);
		}
		if (chdir(dir) == -1) {
			fprintf(stderr, "[!] Error: %s\n", strerror(errno));
		}
		printf("%s\n", dir);
	}
	
	if (chdir(dir) == -1) {
		fprintf(stderr, "[!] Error: %s\n", strerror(errno));
	}
}
