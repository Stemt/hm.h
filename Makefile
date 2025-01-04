all: test
	gcc -ggdb -std=c99 -Wall -Wextra -o app example.c

test:
	gcc -ggdb -Wall -Wextra -o testexec tests/test.c -I.
	./testexec
