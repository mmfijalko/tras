
CC := cc
CFLAGS= -g 
INC= -I${CURDIR}
INC+=-I${CURDIR}/include
INC+=-I./frequency

SUBDIR= frequency blkfreq lcomplex approxe bmatrix cusum \
	excursion excursionv fourier maurer ntmatch otmatch \
	runs serial

all: hamming8.o
	for dir in ${SUBDIR}; do \
		${MAKE} -C $$dir; \
	done;
#	+${MAKE} -C ${SUBDIR}

%.o: %.c %.h
	${CC} ${CFLAGS} ${INC} -c $< -o $@

clean:
	for dir in ${SUBDIR}; do \
		${MAKE} -C $$dir -f Makefile $@; \
	done
	rm -rf *.o
