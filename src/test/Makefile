CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../utils/
CFLAGS+=-I${CURDIR}/../frequency/
CFLAGS+=-I${CURDIR}/../sphere3d/
CFLAGS+=-I${CURDIR}/../mindist/
CFLAGS+=-I${CURDIR}/../plot/
CFLAGS+=-I${CURDIR}/../squeeze/
CFLAGS+=-I${CURDIR}/../approxe/

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/
VPATH+=${CURDIR}/../frequency/
VPATH+=${CURDIR}/../sphere3d/
VPATH+=${CURDIR}/../mindist/
VPATH+=${CURDIR}/../plot
VPATH+=${CURDIR}/../squeeze/
VPATH+=${CURDIR}/../approxe/

LDFLAGS=-lm

all: test

test: test.o frequency.o hamming8.o utils.o tras.o sphere3d.o mindist.o plot.o \
    squeeze.o approxe.o
	${CC} $^ ${LDFLAGS} -o test

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
	rm -f test
