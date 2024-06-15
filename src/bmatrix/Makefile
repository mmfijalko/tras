
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../utils/
CFLAGS+=-I${CURDIR}/../

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/

LDFLAGS=-lm

all: tras.o utils.o bmrank.o bmatrix.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
