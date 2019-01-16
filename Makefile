# ParAd: Parallel Adriana 1.1.2, Makefile
# @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

# Externals: GraphBLAS, version >= 1.1.2

GRAPHBLASINCLUDEDIR=/usr/local/include
GRAPHBLASLIBDIR=/usr/local/lib 

# Variables

PROJ=test/ParAd
CC=mpicc
MAKE=make
CFLAGS=-O3 -I$(GRAPHBLASINCLUDEDIR) -Wno-unused-result
LDFLAGS=-O3 -L$(GRAPHBLASLIBDIR) -lgraphblas -fopenmp -lm 
# For static linking comment the previous line and uncomment the next line 
#LDFLAGS=-O3 -static -L$(GRAPGBLASLIBDIR) -lgraphblas -fopenmp -lm

CLEANFILES= $(PROJ) src/*.o test/output/* test/*__*

SRC=$(wildcard src/*.c)

# Rules

.c.o:
	@rm -f $@
	$(CC) $(CFLAGS) -c $*.c -o $@

OBJ = $(SRC:.c=.o) 

all: $(PROJ)
	$(MAKE) -C Tina_io
	$(MAKE) -C utils

$(PROJ): $(OBJ)
	@rm -f $@
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)
	
clean:
	rm -f $(CLEANFILES)
	$(MAKE) -C Tina_io clean
	$(MAKE) -C utils clean
	
# Dependencies (.c and .h files)

Adriana.o: Adriana.c uti.h ana.h TouOMP.h call4ti2.h
ana.o: ana.c deo.h fic.h coy.h fui.h mui.h grb_mul2a.h smb.h uti.h ana.h cor.h
deo.o: deo.c uti.h
TouOMP.o: TouOMP.c uti.h
call4ti2.o: call4ti2.c uti.h
cor.o: cor.c cor.h
coy.o: coy.c uti.h
fui.o: fui.c uti.h
mui.o: mui.c uti.h
grb_mul2a.o: grb_mul2a.c uti.h
uti.o: uti.c uti.h
smb.o: smb.c uti.h

# end Makefile

