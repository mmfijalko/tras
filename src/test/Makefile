CFLAGS=	-g
CFLAGS+=-I${CURDIR}
CFLAGS+=-I${CURDIR}/../include/
CFLAGS+=-I${CURDIR}/../utils/
CFLAGS+=-I${CURDIR}/../math/
CFLAGS+=-I${CURDIR}/../frequency/
CFLAGS+=-I${CURDIR}/../approxe/
CFLAGS+=-I${CURDIR}/../sphere3d/
CFLAGS+=-I${CURDIR}/../mindist/
CFLAGS+=-I${CURDIR}/../plot/
CFLAGS+=-I${CURDIR}/../squeeze/
CFLAGS+=-I${CURDIR}/../sparse/
CFLAGS+=-I${CURDIR}/../opso/
CFLAGS+=-I${CURDIR}/../otso/
CFLAGS+=-I${CURDIR}/../oqso/
CFLAGS+=-I${CURDIR}/../dna/
CFLAGS+=-I${CURDIR}/../bstream/
CFLAGS+=-I${CURDIR}/../blkfreq/
CFLAGS+=-I${CURDIR}/../runs/
CFLAGS+=-I${CURDIR}/../longruns/
CFLAGS+=-I${CURDIR}/../cusum/
CFLAGS+=-I${CURDIR}/../excursion/
CFLAGS+=-I${CURDIR}/../excursionv/
CFLAGS+=-I${CURDIR}/../maurer/
CFLAGS+=-I${CURDIR}/../coron/
CFLAGS+=-I${CURDIR}/../bspace/
CFLAGS+=-I${CURDIR}/../craps/
CFLAGS+=-I${CURDIR}/../cephes/
CFLAGS+=-I${CURDIR}/../chi2/
CFLAGS+=-I${CURDIR}/../bmatrix/
CFLAGS+=-I${CURDIR}/../bmatrix/bmrank/
CFLAGS+=-I${CURDIR}/../bmatrix/brank31/
CFLAGS+=-I${CURDIR}/../bmatrix/brank68/

VPATH+=${CURDIR}/../utils/
VPATH+=${CURDIR}/../tras/
VPATH+=${CURDIR}/../math/
VPATH+=${CURDIR}/../frequency/
VPATH+=${CURDIR}/../sphere3d/
VPATH+=${CURDIR}/../mindist/
VPATH+=${CURDIR}/../plot/
VPATH+=${CURDIR}/../squeeze/
VPATH+=${CURDIR}/../approxe/
VPATH+=${CURDIR}/../sparse/
VPATH+=${CURDIR}/../opso/
VPATH+=${CURDIR}/../otso/
VPATH+=${CURDIR}/../oqso/
VPATH+=${CURDIR}/../dna/
VPATH+=${CURDIR}/../bstream/
VPATH+=${CURDIR}/../blkfreq/
VPATH+=${CURDIR}/../runs/
VPATH+=${CURDIR}/../longruns/
VPATH+=${CURDIR}/../cusum/
VPATH+=${CURDIR}/../excursion/
VPATH+=${CURDIR}/../excursionv/
VPATH+=${CURDIR}/../maurer/
VPATH+=${CURDIR}/../coron/
VPATH+=${CURDIR}/../bspace/
VPATH+=${CURDIR}/../craps/
VPATH+=${CURDIR}/../cephes/
VPATH+=${CURDIR}/../chi2/
VPATH+=${CURDIR}/../bmatrix/
VPATH+=${CURDIR}/../bmatrix/bmrank/
VPATH+=${CURDIR}/../bmatrix/brank31/
VPATH+=${CURDIR}/../bmatrix/brank68/

LDFLAGS=-lm

all: test

test: hamming8.o utils.o tras.o igamc.o chi2.o chi2_utils.o frequency.o runs.o \
      blkfreq.o sphere3d.o mindist.o plot.o squeeze.o approxe.o sparse.o \
      opso.o otso.o oqso.o dna.o bstream.o cusum.o excursionv.o excursion.o universal.o \
      maurer.o coron.o longruns.o bspace.o craps.o lentz_gamma.o test.o
	${CC} $^ ${LDFLAGS} -o test

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.o
	rm -f test
