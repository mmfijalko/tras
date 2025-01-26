
INC= -I${CURDIR}/
INC+=-I${CURDIR}/include
INC+=-I${CURDIR}/utils

SUBDIR= c1tsbits c1tsbyte

all: sub

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
