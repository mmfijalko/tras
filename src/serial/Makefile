
CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../cephes/
CFLAGS+=-I${CURDIR}/../

CFLAGS+=-DSERIAL_DEBUG

VPATH+=${CURDIR}/../cephes/

all: igamc.o serial.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
