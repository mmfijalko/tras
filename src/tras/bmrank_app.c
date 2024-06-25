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
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <time.h>

#define	min(a, b)	(((a) < (b)) ? (a) : (b))

static void
binary_matrix32_show(uint32_t *bmatrix, unsigned int m, unsigned int n)
{
	unsigned int i, j;
	uint32_t row, mask;

	for (i = 0; i < m; i++) {
		row = bmatrix[i];
		mask = 0x80000000;
		for (j = 0; j < n; j++) {
			printf("%c ", (row & mask) ? '1' : '0');
			mask = mask >> 1;
		}
		printf("\n");
	}
}

static unsigned int
binary_matrxi32_rank(uint32_t *bmatrix, unsigned int m, unsigned int n)
{
	unsigned int h, k, j, i;
	uint32_t kmask, rmask, t;

	h = k = 1;

	for (k = 0, h = 0; k < n && h < m; k++) {
		/*
		 * Find the pivot, row index.
		 */
		kmask = 1 << (31 - k); 
		for (i = h; i < m; i++) {
			if (bmatrix[i] & kmask)
				break;
		}
		if (i < m) {
			/*
			 * swap rows with index h and j = pivot.
			 */
			if (i != h) {
				t = bmatrix[h];
				bmatrix[h] = bmatrix[i];
				bmatrix[i] = t;
			}

			/*
			 * Do for all rows below pivot row.
			 */
			for (i = h + 1; i < m; i++) {
				if (bmatrix[i] & kmask) {
					rmask = 1 << (31 - k);
					bmatrix[i] ^= bmatrix[h];
				}
			}
			h++;
		}
	}

	return (h);
}

static int
bmatrix_random_test(void)
{
	uint32_t *matrix;
	time_t t = time(NULL);
	unsigned int n, m, i, rank;

	m = 18;
	n = 15;

	matrix = malloc(m * sizeof(uint32_t));
	if (matrix == NULL) {
		printf("failed to alloc memory\n");
		return (ENOMEM);
	}

	srand((unsigned int)t);

	for (i = 0; i < m; i++)
		matrix[i] = rand();

	printf("original matrix:\n");
	binary_matrix32_show(matrix, m, n);
	printf("\n");

	rank = binary_matrxi32_rank(matrix, m, n);

	printf("raw echelon matrix:\n");
	binary_matrix32_show(matrix, m, n);
	printf("\nwith rank = %d\n", rank);

	free(matrix);

	return (0);
}

static double *
bmatrix_rank_probs(unsigned int m, unsigned int q)
{
        int i, j, r;
        double *p, pr, y;

        r = (int)min(m, q); 

        p = malloc((r + 1) * sizeof(double));
        if (p == NULL)
                return (p);

	printf("m = %u, q = %u\n", m, q);

        for (j = 0; j < r + 1; j++) {
		y = (double)(j * ((int)m + (int)q - j) - (int)(m * q));
                pr = pow(2.0, y);
//		printf("%d : pr = %.32f, y = %.16g\n", j, pr, y);
                for (i = 0; i < (int)j; i++) {
			pr = pr * (1.0 - pow(2.0, i - (int)q));
			pr = pr * (1.0 - pow(2.0, i - (int)m));
                        pr = pr / (1.0 - pow(2.0, i - j));
                }
                p[j] = pr;
        }
        return (p);
}

static int
bmatrix_show_probs(unsigned int m, unsigned int q)
{
	unsigned int i;
	double *p;

	p = bmatrix_rank_probs(m, q);
	if (p == NULL) {
		printf("bmatrix: failed to allocated rank probs memory\n");
		return (ENOMEM);
	}

	for (i = 0; i < min(m, q) + 1; i++) {
		printf("P(r = %d) = %.32f\n", i, p[i]);
	}

	free(p);

	return (0);
}

static int
bmatrix_static_test(void)
{
	uint32_t *matrix;
	unsigned int m, n, rank;

	m = 4;
	n = 4;

	matrix = malloc(m * sizeof(uint32_t));
	if (matrix == NULL) {
		printf("failed to alloc memory\n");
		return (ENOMEM);
	}

	matrix[0] = 0x80000000;
	matrix[1] = 0x40000000;
	matrix[2] = 0x20000000;
	matrix[3] = 0x10000000;

	printf("original matrix:\n");
	binary_matrix32_show(matrix, m, n);
	printf("\n");

	rank = binary_matrxi32_rank(matrix, m, n);

	printf("raw echelon matrix:\n");
	binary_matrix32_show(matrix, m, n);
	printf("\nwith rank = %d\n", rank);

	free(matrix);

	return (0);

}

int main(void)
{
#ifdef __not_yet__
	bmatrix_static_test();
#endif
	bmatrix_show_probs(32, 32);

#ifdef __not_yet__
	bmatrix_random_test();
#endif

	return (0);
}

