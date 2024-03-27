
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../utils/
CFLAGS+=-I${CURDIR}/../frequency/
CFLAGS+=-I${CURDIR}/../

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/
VPATH+=${CURDIR}/../frequency/

LDFLAGS=-lm

all: test

test: frequency.o hamming8.o utils.o tras.o runs.o test.o
	${CC} $^ ${LDFLAGS} -o $@

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
	rm -f genruns
