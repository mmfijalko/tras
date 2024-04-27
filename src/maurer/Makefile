
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../tras/
CFLAGS+=-I${CURDIR}/../utils/

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/

LDFLAGS=-lm

all: maurer.o coron.o utils.o tras.o
	
%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
