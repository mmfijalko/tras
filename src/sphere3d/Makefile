
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../
CFLAGS+=-I${CURDIR}/../utils/

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/

all: utils.o tras.o sphere3d.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
