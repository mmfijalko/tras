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
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>

static const uint8_t runs8[256] = {
	0, 1, 2, 1, 2, 3, 2, 1, 2, 3, 4, 3, 2, 3, 2, 1,
	2, 3, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 3, 2, 1,
	2, 3, 4, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 4, 3,
	2, 3, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 3, 2, 1,
	2, 3, 4, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 4, 3,
	4, 5, 6, 5, 6, 7, 6, 5, 4, 5, 6, 5, 4, 5, 4, 3,
	2, 3, 4, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 4, 3,
	2, 3, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 3, 2, 1,
	1, 2, 3, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 3, 2,
	3, 4, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 4, 3, 2,
	3, 4, 5, 4, 5, 6, 5, 4, 5, 6, 7, 6, 5, 6, 5, 4,
	3, 4, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 4, 3, 2,
	1, 2, 3, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 3, 2,
	3, 4, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 4, 3, 2,
	1, 2, 3, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 3, 2,
	1, 2, 3, 2, 3, 4, 3, 2, 1, 2, 3, 2, 1, 2, 1, 0,
};

#define	__BIT(p, i)	(((p)[(i) / 8] >> (7 - ((i) & 0x07))) & 0x01)

/*
 * Slow bit per bit algorithm to calculate number of runs.
 */
static unsigned int
runs_runs_count1(void *data, unsigned int nbits)
{
	uint8_t *p = (uint8_t *)data;
	unsigned int runs, i;

	if (nbits == 0 || nbits == 1)
		return (0);

	for (runs = 0, i = 0; i < nbits - 1; i++) {
		if (__BIT(p, i) != __BIT(p, i + 1))
			runs++;
	}

	return (runs);
}

static unsigned int
runs_runs_count2(void *data, unsigned int nbits)
{
	uint8_t *p = data, u8, t;
	unsigned int n, i, runs;

	n = nbits >> 3;
	t = *p & 0x80;
	for (runs = 0, i = 0; i < n; i++, p++) {
		if ((*p & 0x80) ^ t)
			runs++;
		runs += runs8[*p];
		t = (*p << 7) & 0x80;
	}

	n = nbits & 0x07;
	if (n > 0) {
		if ((*p & 0x80) ^ t)
			runs++;
		t = *p;
		for (i = 0; i < n - 1; i++) {
			if ((t & 0x80) ^ ((t << 1) & 0x80))
				runs++;
			t = t << 1;
		}
	}
	return (runs);
}

int
main(void)
{
	uint8_t u8, runs;
	int i, j, new;

	printf("static const uint8_t runs[256] = {");

	for (i = 0, u8 = 0; i < 256; i++, u8++) {
		if ((i % 16) == 0)
			printf("\n\t");
		for (runs = 0, j = 7; j > 0; j--) {
			if (((u8 >> (j - 1)) ^ (u8 >> j)) & 0x01)
				runs++;
		}
		printf("%d,%s", runs,
		    ((i != 0) && (((i + 1) % 16) == 0)) ? "" : " ");
	}
	printf("\n};\n");
	printf("\n");

	printf("static const uint8_t runs[256] = {");
	for (i = 0, u8 = 0; i < 256; i++, u8++) {
		if ((i % 16) == 0)
			printf("\n\t");
		runs = runs_runs_count2(&u8, 8);
		printf("%d,%s", runs,
		    ((i != 0) && (((i + 1) % 16) == 0)) ? "" : " ");
	}
	printf("\n};\n");
	printf("\n");

	printf("static const uint8_t runs[256] = {");
	for (i = 0, u8 = 0; i < 256; i++, u8++) {
		if ((i % 16) == 0)
			printf("\n\t");
		runs = runs_runs_count1(&u8, 8);
		printf("%d,%s", runs,
		    ((i != 0) && (((i + 1) % 16) == 0)) ? "" : " ");
	}
	printf("\n};\n");

	return (0);
}
