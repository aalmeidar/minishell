#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "redirect.h"


int redirect_input(char *file) {
	return redirect_std_file(STDIN_FILENO, file);
}

int redirect_output(char *file) {
	return redirect_std_file(STDOUT_FILENO, file);
}

int redirect_error(char *file) {
	return redirect_std_file(STDERR_FILENO, file);
}

int redirect_std_file(int std, char* file) { 
	int fd, saved_fd;
	if (std ==  STDIN_FILENO) {
		fd = open(file, O_RDONLY);
	} else if (std == STDOUT_FILENO || std == STDERR_FILENO) {
		fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	}
	if (fd < 0) {
		return -1;
	}
	saved_fd = dup(std);
	if (redirect_std_fd(std, fd) == -1) {
		return -1;
	}
	return saved_fd;
}

int redirect_std_fd(int std, int fd) {
	int newfd;
	newfd = dup2(fd, std);
	if (close(fd) == -1) {
		return -1;
	}
	return newfd;
}
