all: main.c
	gcc -o BackItUp main.c -pthread -Wall -Werror

test: main.c
	make all
