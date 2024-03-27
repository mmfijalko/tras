
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../utils/
CFLAGS+=-I${CURDIR}/../
CFLAGS+=-I${CURDIR}/../frequency/

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/
VPATH+=${CURDIR}/../frequency/

LDFLAGS=-lm

all: test

test: plot.o test.o utils.o tras.o frequency.o hamming8.o
	${CC} $^ ${LDFLAGS} -o test

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
	rm -rf test
