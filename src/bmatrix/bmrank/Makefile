#
# Building binary matrix rank test module Marek Marcin Fijałkowski, 2023
#

CC := cc
CFLAGS=	-g

FAMDIR=${CURDIR}/../
CFLAGS+=-I${FAMDIR}/../include/
CFLAGS+=-I${FAMDIR}/../utils/

VPATH+=${FAMDIR}/../utils/
VPATH+=${FAMDIR}/../tras/

LDFLAGS=-lm

all: tras.o utils.o bmrank.o bmatrix.o

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
