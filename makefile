N := 100
NT := 8
input_file := numbers
output_file := sorted
strat := a

all: qsp

qsp: qsp.c
	gcc -Wall -Wextra -fopenmp -pthread -o qsp qsp.c -lm
qspo: qsp.c
	gcc -Wall -Wextra -O3 -ffast-math -march=native -fopt-info-vec -fopenmp -pthread -o qsp qsp.c -lm

run: qsp
	./qsp $(N) $(input_file) $(output_file) $(NT) $(strat)

clean:
	rm -f qsp
