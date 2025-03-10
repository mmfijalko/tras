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

#ifndef __BSPACE_H__
#define	__BSPACE_H__

/*
 * Common parameters for single birtday spacing test and for the real
 * first level birthdays spacing test with multiple K values (the number
 * of pairs of sequencial birtdays with the spacing of more than one day).
 */
struct bspace_params {
	unsigned long long	m;	/* number of birthdays */
	unsigned long long	n;	/* number of days in a year */
	unsigned int		b;	/* bits offset in generator word */
	unsigned int		q;	/* number of bits per day */
	unsigned int		jn;	/* number of j's stats for chi2 */
	double			alpha;	/* significance level for H0 */
};

/*
 * Practical restrictions for the test.
 */
#define	BSPACE_MIN_Q		8	/* minimum number of birthdays 2^8 */
#define	BSPACE_MAX_Q		32	/* maximum number of birthdays 2^32 */

#define	BSPACE_MIN_M		(1ULL << BSPACE_MIN_Q)
#define	BSPACE_MAX_M		(1ULL << BSPACE_MAX_Q)

/*
 * Maximum and minimu numbers of days in a year. The minimu is large enough
 * to compare results to the Poison distributes with mean m^3/(4n).
 */
#define	BSPACE_MIN_N		(1ULL << 18)	/* enough to make sense */
#define	BSPACE_MAX_N		(1ULL << 32)	/* full 32 bits of words */

/*
 * Bit offset range for getting birthday from one generator word.
 */
#define	BSPACE_MIN_BIT_OFFSET	0
#define	BSPACE_MAX_BIT_OFFSET	7

TRAS_DECLARE_ALGO(sbs);

/*
 * The min and max number of samples of j or sometimes called K, the number
 * of pairs of sequential birthdays with the spacing of more than one day.
 */
#define	BSPACE_MIN_JN		200		/* min number of j samples */
#define	BSPACE_MAX_JN		500		/* max number of j samples */

TRAS_DECLARE_ALGO(bspace);

// TRAS_DECLARE_ALGO(bspace2lev);

#endif
