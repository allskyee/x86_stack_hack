all : main.c
	gcc -m32 -g -o test main.c -ldl
