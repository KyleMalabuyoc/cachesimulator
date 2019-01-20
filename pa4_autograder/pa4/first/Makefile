all: first

first: first.c
	gcc -g -Wall -Werror -std=c99 -fsanitize=address first.c -o first

clean:
	rm -rf first
