
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../chi2/
CFLAGS+=-I${CURDIR}/../

VPATH+=${CURDIR}/../chi2/

LDFLAGS=-lm

excpi: excpi.o
	${CC} $^ ${LDFLAGS} -o excpi

all:  excursion.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
	rm -f excpi
