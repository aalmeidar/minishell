#ifndef REDIRECT_H
#define REDIRECT_H

int redirect_input(char* file);
int redirect_output(char* file);
int redirect_error(char* file);
int redirect_std_file(int std, char* file);
int redirect_std_fd(int std, int fd);
#endif
