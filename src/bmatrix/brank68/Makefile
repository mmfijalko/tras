
CFLAGS=	-g
CFLAGS+=-I${CURDIR}

FAMDIR=${CURDIR}/../
CFLAGS+=-I${FAMDIR}/../include/
CFLAGS+=-I${FAMDIR}/../utils/

VPATH+=${FAMDIR}/../utils/
VPATH+=${FAMDIR}/../tras/

LDFLAGS=-lm

all: brank68.o hamming8.o utils.o tras.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
