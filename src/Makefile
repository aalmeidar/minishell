CC = gcc
CFLAGS = -Wall -Wextra 
TARGET = minishell
OBJS = minishell.o background.o builtin.o exec.o job.o redirect.o stack.o utils.o 

$(TARGET) : $(OBJS)
		$(CC) $(CFLAGS) $(OBJS) libparser_64.a -o $(TARGET) -g -static

%.o : %.c
	$(CC) -g -c $(CFLAGS) $< -o $@

.PHONY: clean
clean :
	rm -f *.o $(TARGET)
