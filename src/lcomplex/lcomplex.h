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

#ifndef	__TRAS_LINEAR_COMPLEXITY_H__
#define	__TRAS_LINEAR_COMPLEXITY_H__

/*
 * The parameters for the Linear Complexity Test:
 * M - the length in bits of a block.
 * n - the length of the bit string.
 * K - the number of degrees of freedom, temporary K = 6 hardcoded.
 * alpha - significance level to accept or reject H0.
 */

/*
 * The parameters structure for the Linear Complexity Test.
 */
struct lcomplex_params {
	unsigned int	M;		/* the length in bits of a block */
	unsigned int	K;		/* the number of degrees of freedom */
	double		u;		/* theoretical mean under H0 */
	double		alpha;		/* significance level for H0 */
};

/*
 * Input Restrictions:
 *
 * n >= 10^6
 *
 * 500 <= M <= 5000
 *
 * N : number of independent blocks n = MN, N >= 200 for chi-square result 
 *     to be valid.
 *
 * Question: are the parameters restriction values tied with hardcoded degree
 *           of freedom K = 6 ?
 */

#define	LINEARC_MIN_BITS	1000000
#define	LINEARC_MAX_BITS	0

#define	LINEARC_MIN_M		500
#define	LINEARC_MAX_M		5000

#define	LINEARC_MIN_K		6
#define	LINEARC_MAX_K		6

TRAS_DECLARE_ALGO(lcomplex);

#endif

