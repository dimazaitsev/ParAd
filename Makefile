# ParAd: Parallel Adriana, Makefile
# @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

# Externals: GraphBLAS, version >= 1.1.2

GRAPGBLASINCLUDEDIR=/usr/local/include
GRAPGBLASLIBDIR=/usr/local/lib 

# METISLIBDIR=/usr/lib/x86_64-linux-gnu
# -L$(METISLIBDIR) -lmetis 

# Variables

PROJ=test/ParAd
CC=mpicc
MAKE=make
CFLAGS=-I$(GRAPGBLASINCLUDEDIR) -Wno-unused-result
LDFLAGS=-L$(GRAPGBLASLIBDIR) -lgraphblas -fopenmp -lm 
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
ana.o: ana.c deo.h fic.h coy.h fui.h mui.h grb_mul2a.h smb.h uti.h ana.h cor.h agg_clans.h
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
agg_clans.o: agg_clans.h

# end Makefile

