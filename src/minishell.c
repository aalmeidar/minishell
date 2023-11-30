#include <stdio.h>
#include "exec.h"

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
        exec_line(line);
        printf("msh > ");
    }
    return 0;
}
