#
# Building binary matrix (31x31) rank test module Marek Marcin Fijałkowski, 2023
#

CFLAGS=	-g
CFLAGS+=-I${CURDIR}
FAMDIR=${CURDIR}/../
CFLAGS+=-I${FAMDIR}/../include/
CFLAGS+=-I${FAMDIR}/../utils/

VPATH+=${FAMDIR}/../utils/
VPATH+=${FAMDIR}/../tras/

LDFLAGS=-lm

all: brank31.o hamming8.o utils.o tras.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
