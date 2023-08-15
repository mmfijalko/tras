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

#define	polyn(n)	\
	(((n) + 8) / 8)
#define	bit(p, i)	\
	((p)[(i) >> 3] & (1 << (7 - ((i) & 0x07)))

/*
 * Berlekamp-Massey algorith for the smallest length of LFSR. From "Handbook of
 * Applied Cryptography by A. Menezes, P. van Oorschot and S. Vanstone".
 */
static int
bkamp_massey(void *s, unsigned int n, unsigned int *pL)
{
	unsigned char *p, L, d, N;
	unsigned char *c, *t, *b;
	int m, cn, tn, bn, i, k;

	if (s == NULL || pl == NULL || n == 0)
		return (EINVAL);

	buf = malloc(3 * polyn(n));
	if (buf == NULL)
		return (ENOBUFS);

	m = -1;
	L = 0;
	p = (unsigned char *)s;
	N = 0;

	c[0] = 0x80;
	cn = 0;

	b[0] = 0x80;
	bn = 0;

	tn = 0;

	while (N < n) {
		/* TODO: Compute discrepancy d */
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
				/* B(D) := B(D) * D^(N - m) */
				/* TODO: shift B(D) */
			}

			/*
			 * B(D) shifted or not; C(D) := C(D) + B(D)
			 */
			cn = max(cn, bn);
			k = polyn(cn);
			for (i = 0; i < k; i++)
				c[i] = c[i] ^ b[i];

			if (L <= N / 2) {
				L = N + 1 - L;
				m = N;
				k = polyn(tn);
				for (i = 0; i < k; i++)
					b[i] = t[i];
				bn = tn;
			}
		}
		N++;
	}

	*pN = L;

	return (0);
}
