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

#ifndef __C1TSBYTE_H__
#define	__C1TSBYTE_H__

/*
 * Count-the-1's Test (Stream of Specific Bytes) parameters.
 */
struct c1tsbyte_params {
	unsigned int	sbit;	/* start bit for selected byte */
	double		alpha;	/* significance level for H0 */
};

/*
 * Words to process.
 */
#define	C1TSBYTE_WORDS		256000

/*
 * The number of letters needed to get (overlapped) all five letters words.
 */
#define	C1TSBYTE_LETTERS	(C1TSBYTE_WORDS + 4)

/*
 * Each single letter is selected from one generator word.
 */
#define	C1TSBYTE_BYTES		(32 * C1TSBYTE_LETTERS)

/*
 * The number of bits is strictly defined.
 */
#define	C1TSBYTE_MIN_NBITS	(C1TSBYTE_BYTES << 3)

#define	C1TSBYTE_MAX_NBITS	C1TSBYTE_MIN_NBITS

TRAS_DECLARE_ALGO(c1tsbyte);

#endif

