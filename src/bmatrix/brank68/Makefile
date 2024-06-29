
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
FAMDIR=${CURDIR}/../

CFLAGS+=-I${FAMDIR}
CFLAGS+=-I${FAMDIR}/../include/
CFLAGS+=-I${FAMDIR}/../utils/
CFLAGS+=-I${FAMDIR}/bmrank/

VPATH+=${FAMDIR}
VPATH+=${FAMDIR}/../utils/
VPATH+=${FAMDIR}/../tras/
VPATH+=${FAMDIR}/bmrank/

LDFLAGS=-lm

all: bmatrix.o bmrank.o utils.o tras.o brank68.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
