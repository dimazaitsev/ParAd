# ParAd: Parallel Adriana - compatibility with Tina, Makefile
# @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

# Rules

PROJ1=fromTina
PROJ2=printTina
CC=gcc
CFLAGS=-I../src
LDFLAGS=-O3  

SRC1 = fromTina.c readnet.c ../src/uti.o

SRC2 = printTina.c readnet.c writeinv.c chi.c ../src/uti.o

.c.o:
	@rm -f $@
	$(CC) $(CFLAGS) -c $*.c

OBJ1 = $(SRC1:.c=.o) 

OBJ2 = $(SRC2:.c=.o) 

CLEANFILES = $(PROJ1) $(PROJ2) $(OBJ1) $(OBJ2)

all: $(PROJ1) $(PROJ2)

$(PROJ1): $(OBJ1)
	@rm -f $@
	$(CC) $(CFLAGS) -o $@ $(OBJ1) $(LDFLAGS)
	
$(PROJ2): $(OBJ2)
	@rm -f $@
	$(CC) $(CFLAGS) -o $@ $(OBJ2) $(LDFLAGS)

clean:
	rm -f $(CLEANFILES)
	
# Dependencies (.c and .h files)

fromTina.o: fromTina.c readnet.h ../src/uti.h
printTina.o: printTina.c readnet.h writeinv.h chi.h ../src/uti.h
readnet.o: readnet.c ../src/uti.h
writeinv.o: writeinv.c ../src/uti.h
chi.o: chi.c ../src/uti.c

# end Makefile

