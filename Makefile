all: example test

example: example.c
	gcc -ggdb -std=c99 -Wall -Wextra -o example_app example.c

test: tests/test.c hm.h
	gcc -ggdb -Wall -Wextra -o test_app tests/test.c -I.
	./test_app

bench_app: tests/bench.c hm.h
	gcc -ggdb -Wall -Wextra -o bench_app tests/bench.c -I.

bench: bench_app
	./bench_app
