#ifndef EXEC_H
#define EXEC_H

#include "parser.h"
void set_pgid_fg(pid_t pgid);
void exec_line(tline* line);
void sig_handler(int sig);
#endif
