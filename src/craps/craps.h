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

#ifndef __CRAPS_H__
#define	__CRAPS_H__

/*
 * The craps test's 1st level parameters.
 */
struct craps_params {
	unsigned int	K;	/* number of games to complete test */
	unsigned int	throws;	/* maximum number of throws, security */
	double		alpha1;	/* significance level for wins test */
	double		alpha2;	/* significance level for throws test */
};

/* The minimum number of games for final */
#define	CRAPS_MIN_GAMES		200000

/*
 * There is no simple rule to determine maximum number of bits.
 * The test can be infinte because we must toss the dice until
 * win or loss. So, the test has the throws parameter to specifiy
 * maximum number of tosses, when it should be broken.
 */

/* Number of bits to finalize the DNA test */
#define	CRAPS_BITS	(DNA_LETTERS1 * 32)

#define	CRAPS_MIN_BITS	CRAPS_BITS	/* minimun number of bits for test */
#define	CRAPS_MAX_BITS	CRAPS_BITS	/* maximum number of bits for test */

TRAS_DECLARE_ALGO(craps);

#endif


