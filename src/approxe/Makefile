
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../utils/
CFLAGS+=-I${CURDIR}/../math/

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/
VPATH+=${CURDIR}/../math/


all: approxe.o utils.o tras.o lentz_gamma.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
