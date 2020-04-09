all: main.c
	gcc -o BackItUp main.c -lbsd -pthread

test: main.c
	make all
