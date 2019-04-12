# ParAd: Parallel Adriana 

PROJ=ParAd
CC=mpicc
GRAPGBLASINCLUDEDIR=/usr/local/include
CFLAGS=-O3 -I$(GRAPGBLASINCLUDEDIR) -Wno-unused-result
GRAPGBLASLIBDIR=/usr/local/lib 
LDFLAGS=-O3 -L$(GRAPGBLASLIBDIR) -lgraphblas -fopenmp -lm 

SRC = Adriana.c ana.c deo.c TouOMP.c call4ti2.c cor.c coy.c fui.c mui.c grb_mul2a.c fic.c uti.c smb.c

.c.o:
	@rm -f $@
	$(CC) $(CFLAGS) -c $*.c

OBJ = $(SRC:.c=.o) 

CLEANFILES = $(PROG) $(OBJ)

all: $(PROJ)

$(PROJ): $(OBJ)
	@rm -f $@
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)
        mv $@ ../bin
	
clean:
	rm -f $(CLEANFILES)
	
# Dependencies (.c and .h files)

Adriana.o: Adriana.c uti.h ana.h TouOMP.h call4ti2.h
ana.o: ana.c deo.h fic.h coy.h fui.h mui.h grb_mul2a.h smb.h uti.h ana.h cor.h
deo.o: deo.c uti.h
TouOMP.o: TouOMP.c uti.h
callti2.o: call4ti2.c uti.h
cor.o: cor.c cor.h
coy.o: coy.c uti.h
fui.o: fui.c uti.h
mui.o: mui.c uti.h
grb_mul2a.o: grb_mul2a.c
uti.o: uti.c uti.h
smb.o: smb.c uti.h

# end Makefile

