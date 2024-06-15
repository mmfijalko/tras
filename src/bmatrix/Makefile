#
# Building all binary matrices test Marek Marcin Fijałkowski, 2023
#

CC := cc
CFLAGS= -g 
INC= -I${CURDIR}
INC+=-I${CURDIR}/include
INC+=-I${CURDIR}/utils

SUBDIR=	brank31 brank32 brank68

all:
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
