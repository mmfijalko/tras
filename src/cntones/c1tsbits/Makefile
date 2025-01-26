
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
FAMDIR=${CURDIR}/../

CFLAGS+=-I${FAMDIR}
CFLAGS+=-I${FAMDIR}/../include/
CFLAGS+=-I${FAMDIR}/../utils/

VPATH+=${FAMDIR}
VPATH+=${FAMDIR}/../utils/
VPATH+=${FAMDIR}/../tras/

LDFLAGS=-lm

all: c1tsbits.o hamming8.o utils.o tras.o cntones.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
	rm -f test
