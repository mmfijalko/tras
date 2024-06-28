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

/*
 * Calculate binary matrix rank.
 *
 * Parameters :
 * bmatrix - the binary matrix stored as list of 32 bits words;
 *           each element of the table represents one row; one
 *           word in the table is in big endian representation
 *           and only n bits are valid ([31..(31-n+1)]), the
 *           rest is unused.
 *
 * m - the number of rows of the bmatrix <1, 32>.
 * n - the number of columns of the bmatrix <1, 32>.
 *
 * Returns: the rank of the m x binary matrix.
 */
unsigned int
binary_matrix_rank(uint32_t *bmatrix, unsigned int m, unsigned int n)
{
	unsigned int h, k, j, i;
	uint32_t kmask, t;

	kmask = 0x80000000;

	for (k = 0, h = 0; k < n && h < m; k++) {
		/*
		 * Find the pivot, row index.
		 */
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
				if (bmatrix[i] & kmask)
					bmatrix[i] ^= bmatrix[h];
			}
			h++;
		}
		kmask = kmask >> 1;
	}

	return (h);
}
