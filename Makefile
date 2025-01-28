all: example test

example: example.c
	gcc -ggdb -std=c99 -Wall -Wextra -o example_app example.c

test: tests/test.c
	gcc -ggdb -Wall -Wextra -o test_app tests/test.c -I.
	./test_app

bench: tests/bench.c
	gcc -ggdb -Wall -Wextra -o bench_app tests/bench.c -I.
	./bench_app
