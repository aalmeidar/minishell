#include <stdio.h>
#include <string.h>
#include "exec.h"
#include "builtin.h"

int main(void){
    char buf[1024];
    tline * line;

    printf("msh > ");
    while (fgets(buf, 1024, stdin)) {
        line = tokenize(buf);
        if (line==NULL) {
            continue;
        }
		if (strcmp(line->commands[0].argv[0], "cd") == 0) {
			cd(line->commands[0].argv[1]);
        	printf("msh > ");
			continue;
		}
        exec_line(line);
        printf("msh > ");
    }
    return 0;
}
