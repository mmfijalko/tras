
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../utils/

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/

LDFLAGS=-lm

all: mindist.o hamming8.o utils.o tras.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
