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

/*
 * NOTICE: Berlekamp Massey algorithm.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define	polyn(n)	\
	(((n) + 8) / 8)

#define	bit(p, i)	\
	((p)[(i) >> 3] & (1 << (7 - ((i) & 0x07))))

#define	max(a, b)	(((a) > (b)) ? (a) : (b))

/*
 * Berlekamp-Massey algorith for the smallest length of LFSR. From "Handbook of
 * Applied Cryptography by A. Menezes, P. van Oorschot and S. Vanstone".
 */
static int
bkamp_massey(void *s, unsigned int n, unsigned int *pL)
{
	unsigned char *p, L, d, N;
	unsigned char *c, *t, *b, *g, *buf;
	int m, cn, tn, bn, gn, i, k, j;
	unsigned char b0, b1;

	if (s == NULL || pL == NULL || n == 0)
		return (EINVAL);

	buf = malloc(4 * polyn(n));
	if (buf == NULL)
		return (ENOBUFS);
	c = buf;
	t = c + polyn(n);
	b = t + polyn(n);
	g = b + polyn(n);

	m = -1;
	L = 0;
	p = (unsigned char *)s;
	N = 0;

	c[0] = 0x80;
	cn = 0;

	b[0] = 0x80;
	bn = 0;

	t[0] = 0;
	tn = 0;

	d = 0;

	while (N < n) {
		printf("N = %d : \n", N);

		/*
		 * Compute discrepancy
		 */
		for (i = 1, d = 0; i <= L; i++) {
			if (bit(c, i) && bit(p, N - i))
				d++;
		}
		if (bit(p, N))
			d++;

		if (d & 0x01) {
			/* T(D) := C(D) */
			k = polyn(cn);
			for (i = 0; i < k; i++)
				t[i] = c[i];
			tn = cn;

			/*
			 * C(D) := C(D) + B(D) * D^(N - m)
			 */
			if (N > m) {
				/* G(D) := B(D) * D^(N - m) */
				k = N - m;
				gn = bn + k;
				printf("N > m, k = %d, gn = %d\n", k, gn);
				if (k & 0x07) {
					j = (k + 7) / 8;
					for (i = 0; i < j; i++)
						g[i] = 0;
					k = k & 0x07;
					for (i = 0; i < polyn(bn); i++) {
						b0 = b[i] >> k;
						b1 = b[i] << (8 - k);
						g[j + i - 1] |= b0;
						g[j + i] = b1;
//						printf("b0 = %02x, b1 = %02x, g[%d] = %02x, g[%d] = %02x\n",
//						    b0, b1, j + i - 1, g[j + i - 1], j + i, g[j + i]);
					}
				} else {
					k = k / 8;
					for (i = 0; i < k; i++)
						g[i] = 0;
					for (i = 0; i < polyn(bn); i++)
						g[i + k] = b[i];
				}
				cn = max(cn, gn);
				k = polyn(cn);
				for (i = 0; i < k; i++)
					c[i] = c[i] ^ g[i];
			} else {
				/*
				 * B(D) not shifted; C(D) := C(D) + B(D)
				 */
				cn = max(cn, bn);
				k = polyn(cn);
				for (i = 0; i < k; i++)
					c[i] = c[i] ^ b[i];
			}

			if (L <= N / 2) {
				L = N + 1 - L;
				m = N;
				k = polyn(tn);
				for (i = 0; i < k; i++)
					b[i] = t[i];
				bn = tn;
			}
		}
		printf("\tsN = %d\n", bit(p, N) ? 1 : 0);
		printf("\td = %d\n", d & 0x01);
		if (t[0] == 0)
			printf("\tT(D) = -\n");
		else {
			printf("\tT(D) = ");
			for (i = 0; i < polyn(tn); i++)
				printf("%02x ", t[i]);
			printf("\n");
		}

		printf("\tC(D) = ");
		for (i = 0; i < polyn(cn); i++)
			printf("%02x ", c[i]);
		printf("\n");

		printf("\tL = %d, m = %d\n", L, m);

		printf("\tB(D) = ");
		for (i = 0; i < polyn(bn); i++)
			printf("%02x ", b[i]);
		printf("\n");
		printf("\tN = %d\n", N + 1);

		N++;
	}

	*pL = L;

	free(buf);

	return (0);
}

int
main(void)
{
	unsigned char s[128];
	unsigned int L;
	int error;

#if 0
	s[0] = 0x37;
	s[1] = 0x00;

	error = bkamp_massey((void *)s, 9, &L);
	if (error != 0) {
		printf("bkamp_meassey failed (%d)\n", error);
		return (error);
	}

	printf("L = %d\n", L);

	s[0] = 0x92; /* 1, 0, 0, 1, 0, 0, 1, 1, */
       	s[1] = 0xc4; /* 1, 1, 0, 0, 0, 1, 0, 0, */
       	s[2] = 0xe0; /*	1, 1, 1, 0. */
	
	error = bkamp_massey((void *)s, 20, &L);
	if (error != 0) {
		printf("bkamp_meassey failed (%d)\n", error);
		return (error);
	}

	printf("L = %d\n", L);

	s[0] = 0xe8; /* 1,1,1,0,1,0,0,0, */
	s[1] = 0xa6; /* 1,0,1,0,0,1,1,0, */
	s[2] = 0x3b; /* 0,0,1,1,1,0,1,1, */
	s[3] = 0x00; /* 0,0 */
	error = bkamp_massey((void *)s, 26, &L);
	if (error != 0) {
		printf("bkamp_meassey failed (%d)\n", error);
		return (error);
	}

	printf("L = %d\n", L);
#endif

	s[0] = 0x85; /* 1000 0101 */
	s[1] = 0x76; /* 0111 0110 */
	s[2] = 0x3e; /* 0011 1110 */
	s[3] = 0x68; /* 0110 100 */

	error = bkamp_massey((void *)s, 31, &L);
	if (error != 0) {
		printf("bkamp_meassey failed (%d)\n", error);
		return (error);
	}

	printf("L = %d\n", L);

	s[0] = 0x78;

	error = bkamp_massey((void *)s, 6, &L);
	if (error != 0) {
		printf("bkamp_meassey failed (%d)\n", error);
		return (error);
	}

	printf("L = %d\n", L);

	s[0] = 0xe8; // 1,1,1,0, 1,0,0,0,
	s[1] = 0xa6; // 1,0,1,0, 0,1,1,0,
	s[2] = 0x00; // 0,0

	error = bkamp_massey((void *)s, 18, &L);
	if (error != 0) {
		printf("bkamp_meassey failed (%d)\n", error);
		return (error);
	}

	printf("L = %d\n", L);

	return (0);
}
