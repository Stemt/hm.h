all: example test

example: example.c
	gcc -ggdb -std=c99 -Wall -Wextra -o example_app example.c

test: tests/test.c hm.h
	gcc -ggdb -Wall -Wextra -o test_app tests/test.c -I.
	./test_app

clean:
	rm -f example_app
	rm -f test_app
