#
# Building all tests in the subdirectories, Marek Marcin Fijałkowski, 2023
#

CC := cc
CFLAGS= -g 
INC= -I${CURDIR}
INC+=-I${CURDIR}/include
INC+=-I${CURDIR}/utils

SUBDIR= frequency blkfreq lcomplex approxe bmatrix cusum \
	excursion excursionv fourier maurer ntmatch otmatch \
	runs serial bmatrix sphere3d craps c1tsbits plot \
       	mindist opso oqso dna
#SUBDIR+= squeeze c1tssbytes
#SUBDIR+= tras utils

all:
	for dir in ${SUBDIR}; do \
		${MAKE} -C $$dir; \
	done;

%.o: %.c %.h
	${CC} ${CFLAGS} ${INC} -c $< -o $@

clean:
	for dir in ${SUBDIR}; do \
		${MAKE} -C $$dir -f Makefile $@; \
	done
	rm -rf *.o
