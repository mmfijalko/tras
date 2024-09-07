
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../utils/
CFLAGS+=-I${CURDIR}/../cephes/

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/
VPATH+=${CURDIR}/../cephes/

LDFLAGS=-lm

all: utils.o tras.o igamc.o chi2.o chi2_utils.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
