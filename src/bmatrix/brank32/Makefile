
CFLAGS=	-g
CFLAGS+=-I${CURDIR}

FAMDIR=${CURDIR}/../
CFLAGS+=-I${FAMDIR}/../include/
CFLAGS+=-I${FAMDIR}/../utils/
CFLAGS+=-I${FAMDIR}/bmrank/

VPATH+=${FAMDIR}/../utils/
VPATH+=${FAMDIR}/../tras/
VPATH+=${FAMDIR}/bmrank/

LDFLAGS=-lm

all: brank32.o hamming8.o utils.o tras.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
