# ParAd: Parallel Adriana - utilities

GRAPGBLASINCLUDEDIR=/usr/local/include
GRAPGBLASLIBDIR=/usr/local/lib 
CFLAGS=-O3 -I$(GRAPGBLASINCLUDEDIR) -Wno-unused-result
LDFLAGS=-O3 -L$(GRAPGBLASLIBDIR) -lgraphblas -fopenmp -lm 

PROGS = gen_clans spm_bas toclans transpose mul2m
CC=gcc
FLAGS=-O3 -Wno-unused-result

all: $(PROGS)

gen_clans: gen_clans.c
	$(CC) $^ $(FLAGS) -o $@

spm_bas: spm_bas.c
	$(CC) $^ $(FLAGS) -o $@

toclans: toclans.c
	$(CC) $^ $(FLAGS) -o $@

transpose: transpose.o ../src/uti.o
	$(CC) $^ $(FLAGS) -o $@

transpose.o: transpose.c
	$(CC) $^ -c $(FLAGS)

mul2m: mul2m.o ../src/uti.o
	$(CC) mul2m.o ../src/uti.o $(LDFLAGS) -o mul2m 

mul2m.o: mul2m.c
	$(CC) $^ -c $(CFLAGS)

../src/uti.o: ../src/uti.c
	$(CC) ../src/uti.c -c $(FLAGS) -o ../src/uti.o

clean:
	rm -f $(PROGS)
	rm -f *.o

# Dependencies (.c and .h files)

gen_clans: gen_clans.c
spm_bas: spm_bas.c
toclans: toclans.c
transpose.o: transpose.c ../src/uti.h
mul2m.o: ../src/grb_mul2a.c ../src/uti.h
../src/uti.o: ../src/uti.c ../src/uti.h

# end Makefile

