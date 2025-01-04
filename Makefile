all: example test

example:
	gcc -ggdb -std=c99 -Wall -Wextra -o example example.c

test:
	gcc -ggdb -Wall -Wextra -o testexec tests/test.c -I.
	./testexec
