/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2023 Marek Marcin Fijałkowski
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <stdio.h>

typedef double (*seqfun_t)(double a, double x, int n);

static double
lentz_algo(double a, double x, seqfun_t afun, seqfun_t bfun, double epsilon,
    int *error)
{
	double C, D, f, delta, af, bf, dm;
	int n;

printf("lentz's algorithm #1:\n");
printf("---------------------\n");

	if (afun == NULL || bfun == NULL | epsilon == 0.0) {
		if (error != NULL)
			*error = EDOM;
		return (NAN);
	}

	/* Initialization */
	dm = epsilon;
	af = afun(a, x, 1);
	bf = bfun(a, x, 1);
	if (bf == 0) {
		if (error != NULL)
			*error = ERANGE;
		return (NAN);
	}
	f = af / bf;
	C = af / dm;
	D = 1.0 / bf;
	n = 2;

printf("a[1] = %g, b[1] = %g, epsilon = %g 1.0 + epsilon %g\n", af, bf, dm, dm + 1.0);
printf("f[1] = %g, C[1] = %g, D[1] = %g, dm = %g\n", f, C, D, dm);

	/* Iteration */
	do {
printf("step number = %d\n", n);
		af = afun(a, x, n);
		bf = bfun(a, x, n);
printf("a[%d] = %g, b[%d] = %g\n", n, af, n, bf);
		D = D * af + bf;
		if (D == 0.0)
			D = dm;
		C = bf + af / C;
		if (C == 0.0)
			C = dm;
printf("D[%d] = %g, C[%d] = %g\n", n, D, n, C);
		D = 1.0 / D;
printf("D = 1 / D = %g\n", D);
		delta = C * D;
		f = f * delta;
		delta = fabs(delta) - 1.0;
		n++;
printf("delta = %.16g, 1.0 + epsilon = %.16g\n", delta, 1.0 + epsilon);
	} while (delta >= epsilon);

	if (error != NULL)
		error = 0;

	return (f);
}

static double
lentz2_algo(double a, double x, seqfun_t afun, seqfun_t bfun, double epsilon,
    int *error)
{
	double C, D, f, delta, af, bf, dm;
	int n;

printf("lentz's algorithm #2:\n");
printf("---------------------\n");

	if (afun == NULL || bfun == NULL | epsilon == 0.0) {
		if (error != NULL)
			*error = EDOM;
		return (NAN);
	}

	/* Initialization */
	dm = epsilon;
	af = afun(a, x, 0);
	bf = bfun(a, x, 0);
	f = (bf == 0.0) ? dm : bf;
	C = f;
	D = 0;
	n = 1;

printf("a[0] = %g, b[0] = %g, epsilon = %g\n", af, bf, dm);
printf("f[0] = %g, C[0] = %g, D[0] = %g, dm = %g\n", f, C, D, dm);

	/* Iteration */
	do {
printf("step number = %d\n", n);
		af = afun(a, x, n);
		bf = bfun(a, x, n);
		D = D * af + bf;
		C = bf + af / C;
printf("a[%d] = %g, b[%d] = %g C[%d] = %g, D[%d] = %g\n", n, af, n, bf, n, C, n, D);
		if (D == 0.0)
			D = dm;
		if (C == 0.0)
			C = dm;
		D = 1.0 / D;
printf("D = 1 / D = %g\n", D);
		delta = C * D;
		f = f * delta;
		n++;
printf("f = %g, delta = %.30G\n", f, delta);
	} while (fabs(delta - 1.0) >= epsilon);

	if (error != NULL)
		error = 0;

	return (f);
}

static double
test_afun(double a, double x, int n)
{

	return ((n == 0) ? 0.0 : ((double)n * (a - (double)n)));
}

static double
test_bfun(double a, double x, int n)
{

	return ((n == 0) ? 0.0 : (x - a + 2 * (double)n + 1.0));
}

int main(int argc, char *argv[])
{
	double l1, l2, a, x, g, epsilon, tg;
	int error = 0;

	a = atof(argv[1]);
	x = atof(argv[2]);

	epsilon = 1e-20;

	printf("calculating continued fraction with the lentz's algo for a = %g, x = %g\n", a, x);

	l1 = lentz_algo(a, x, test_afun, test_bfun, epsilon, &error);
	if (l1 == NAN || error != 0) {
		printf("failed to calculate lentz formula for a = %g, x = %g\n",
		    a, x);
		return (error);
	}

	l2 = lentz2_algo(a, x, test_afun, test_bfun, epsilon, &error);
	if (l2 == NAN || error != 0) {
		printf("failed to calculate lentz formula for a = %g, x = %g\n",
		    a, x);
		return (error);
	}

	printf("lentz1 value = %g\n", l1);
	printf("lentz2 value = %g\n", l2);

	g = pow(x, a) * pow(M_E, -x);
	g = g / (x - a + 1.0 + l1);
	printf("calulated G1(%g, %g) = %.20g\n", a, x, g);

	g = pow(x, a) * pow(M_E, -x);
	printf("prefix tail of G2 = %g\n", g);
	printf("denominator = %g\n", x - a + 1.0 + l2);
	g = g / (x - a + 1.0 + l2);
	printf("calulated G2(%g, %g) = %.20g\n", a, x, g);

	return (0);
}
