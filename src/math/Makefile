
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../utils/

VPATH+=${CURDIR}/../utils/

LDFLAGS=-lm

all: lentz.o
	${CC} $^ ${LDFLAGS} -o lentz

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
	rm -f lentz
