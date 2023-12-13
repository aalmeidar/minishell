#ifndef EXEC_H
#define EXEC_H

#include "parser.h"
void set_pid_fg(pid_t * pid_fg, int n);
void exec_line(tline* line);
void sig_handler(int sig);
#endif
