# Compiling/installing the liblp library for ASPTOOLS

# Some absolute path required below
LIBDIR=$(CURDIR)/lib

UTILS=		version.lo symbol.lo atom.lo input.lo output.lo rule.lo
UTILS_SRC=	version.h symbol.h symbol.c atom.h atom.c io.h input.c \
		output.c rule.h rule.c

LIBTOOL=	libtool
CC=		gcc
CC_FLAGS=	-g

all: 		$(UTILS) liblp.la

liblp.la:	$(UTILS)
		$(LIBTOOL) --tag=CC --mode=link \
		gcc $(CC_FLAGS) -o liblp.la $(UTILS) \
		-static -rpath $(LIBDIR)

clean:
		rm -f *.o *.lo liblp.a liblp.la

install:	liblp.la
		$(LIBTOOL) --mode=install cp liblp.la $(LIBDIR)/liblp.la

finish:		liblp.la
		$(LIBTOOL) --mode=finish $(LIBDIR)

%.lo:		%.c
		$(LIBTOOL) --tag=CC --mode=compile gcc $(CC_FLAGS) -c $*.c
