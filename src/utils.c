#include <stdio.h>
#include <unistd.h>
#include "utils.h"


void prompt() {
	char buf[1024];
	getcwd(buf, 1024);
	printf(BLUE "msh@%s" RESET ":" CYAN "%s" GREEN " > " RESET, getlogin(), buf);
}
