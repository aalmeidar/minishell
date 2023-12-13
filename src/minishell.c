#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "exec.h"
#include "builtin.h"
#include "utils.h"
#include "background.h"

int main(void){
    init_jobs();
    char buf[1024];
    tline * line;

	prompt();
    while (fgets(buf, 1024, stdin)) {
        signal(SIGINT, SIG_IGN);
        line = tokenize(buf);
        if (line == NULL || line->commands==NULL ) {
            prompt();
            continue;
        }

        // Comprobar si el comando a ejecutar es jobs
        if (strcmp(line->commands[0].argv[0], "jobs") == 0) {
            jobs();
            prompt();
            continue;
        }

        check_jobs(0);

		// Comprobar si el comando a ejecutar es cd
		if (strcmp(line->commands[0].argv[0], "cd") == 0) {
			cd(line->commands[0].argv[1]);
			prompt();
			continue;
		}

		// Comprobar si el comando a ejecutar es fg
    	if (strcmp(line->commands[0].argv[0], "fg") == 0) {
            fg(line->commands[0].argv[1]);
			prompt();
        	continue;
    	}

        if (strcmp(line->commands[0].argv[0], "exit") == 0) {
            quit();
        }
		// Ejecutar la linea entera
        exec_line(line);
		prompt();
    }
    delete_jobs();
    return 0;
}
