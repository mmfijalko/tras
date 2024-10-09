#
# Building all binary matrices test Marek Marcin Fijałkowski, 2023
#

CFLAGS= -g 
CFLAGS+=-I${CURDIR}

INC= -I${CURDIR}/
INC+=-I${CURDIR}/include
INC+=-I${CURDIR}/utils

SUBDIR=	bmrank brank31 brank32 brank68

all: sub bmatrix.o

sub:
	for dir in ${SUBDIR}; do	\
		${MAKE} -C $$dir;	\
	done;

%.o: %.c %.h
	${CC} ${CFLAGS} ${INC} -c $< -o $@

clean:
	for dir in ${SUBDIR}; do \
		${MAKE} -C $$dir -f Makefile $@;	\
	done
	rm -rf *.o
