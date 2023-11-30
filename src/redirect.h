

int redirect_input(char* file);
int redirect_output(char* file);
int redirect_error(char* file);
int redirect_std_file(int std, char* file);
int redirect_std_fd(int std, int fd);
