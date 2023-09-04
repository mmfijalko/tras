
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../tras/

all: maurer.o coron.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
