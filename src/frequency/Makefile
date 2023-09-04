
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../utils/

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/

LDFLAGS=-lm

all: test

test: frequency.o test.o hamming8.o utils.o tras.o
	${CC} $^ ${LDFLAGS} -o test

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
	rm -f test
