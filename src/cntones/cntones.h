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

#ifndef __CNTONES_H__
#define	__CNTONES_H__

/*
 * The generic count-the-1's test parameters
 */
struct c1tsbits_params {
	int		algo;	/* algo type and type of byte selection */
	unsigned int	sbit;	/* selected start bit from random word */
	double		alpha;	/* significance level for H0 */
};

/*
 * The Count-the-1's version of the test if its input is stream of bits.
 */
#define	CNTONES_ALGO_BITSTREAM	1

/*
 * The Count-the-1's version of the test in which bytes are selected.
 */
#define	CNTONES_ALGO_SELBYTES	2

/*
 * Words to process for the count-the-1's tests.
 */
#define	CNTONES_WORDS		256000

/*
 * The number of letters needed to get (overlapped) all five letters words.
 */
#define	CNTONES_LETTERS		(CNTONES_WORDS + 4)

/*
 * The stream is treated as sequence of bytes and letters.
 */
#define	C1TSBITS_BYTES		C1TSBITS_LETTERS

/*
 * The number of bits is strictly defined.
 */
#define	C1TSBITS_MIN_NBITS	(C1TSBITS_BYTES << 3)

#define	C1TSBITS_MAX_NBITS	C1TSBITS_MIN_NBITS

TRAS_DECLARE_ALGO(c1tsbits);

#endif


