
CFLAGS=	-g
FAMDIR=${CURDIR}/../

CFLAGS+=-I${CURDIR}
CFLAGS+=-I${FAMDIR}
CFLAGS+=-I${FAMDIR}/../include/
CFLAGS+=-I${FAMDIR}/../utils/

VPATH+=${FAMDIR}
VPATH+=${FAMDIR}/../utils/
VPATH+=${FAMDIR}/../tras/

LDFLAGS=-lm

all: c1tsbyte.o utils.o tras.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
	rm -f test
