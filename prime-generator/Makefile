
all: trial-division sieve segmented-sieve

trial-division: trial-division.c Makefile
	gcc -lm -O3 trial-division.c -o trial-division

sieve: sieve.c Makefile
	gcc -std=gnu99 -O3 sieve.c -o sieve

segmented-sieve: segmented-sieve.c Makefile
	gcc -std=gnu99 -O3 segmented-sieve.c -o segmented-sieve
