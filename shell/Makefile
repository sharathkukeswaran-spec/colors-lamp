CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11

.PHONY: all clean

all: mysh

mysh: mysh.c
	$(CC) $(CFLAGS) -o mysh mysh.c

clean:
	rm -f mysh
