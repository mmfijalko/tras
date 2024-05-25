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
lentz2_algo(double a, double x, seqfun_t afun, seqfun_t bfun, double epsilon,
    int *error)
{
	double C, D, f, delta, af, bf, dm;
	int n;

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

	/* Iteration */
	do {
		af = afun(a, x, n);
		bf = bfun(a, x, n);
		D = D * af + bf;
		C = bf + af / C;
		if (D == 0.0)
			D = dm;
		if (C == 0.0)
			C = dm;
		D = 1.0 / D;
		delta = C * D;
		f = f * delta;
		n++;
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

double
lentz2_gamma(double a, double x, double epsilon, int *error)
{

	return (lentz2_algo(a, x, test_afun, test_bfun, epsilon, error));
}

