# Basic utilities related to Smodels/DIMACS formats

TOOLS=		len lplist lpstrip

INC=		../liblp
LIB=		../liblp/lib
BIN=		../bin

CC=		gcc
CCFLAGS=	-g -I$(INC)

LDFLAGS=	-static -L$(LIB) -llp

all: 		$(TOOLS)

len:		len.o
		$(CC) len.o -o len $(LDFLAGS)

lplist:		lplist.o
		$(CC) lplist.o -o lplist $(LDFLAGS)

lpreify:	lpreify.o
		$(CC) lpreify.o -o lpreify $(LDFLAGS)

lpstrip:	lpstrip.o
		$(CC) lpstrip.o -o lpstrip $(LDFLAGS)

clean:
		rm -f *.o
		rm -f $(TOOLS)

install:	$(TOOLS)
		for t in $(TOOLS);\
		do cp -f ./$$t $(BIN)/$$t;\
		   strip $(BIN)/$$t; done

%.o:            %.c
		$(CC) $(CCFLAGS) -c $*.c
