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

#ifndef __LONGRUNS_H__
#define	__LONGRUNS_H__

/*
 * Parameters for the Longes Run of Ones in a Block test.
 */
struct longruns_params {
	unsigned int	M;	/* the length of each block */
	unsigned int	N;	/* the number of blocks */
	double		alpha;	/* the significance level for H0 */
int		version;/* the update version */
};

/*
 * The list of block size parameters supported by the algorithm.
 * The parameters (K and probabilities) are provided by NIST.
 */
#define	LONGRUNS_M0		8
#define	LONGRUNS_M1		128
#define	LONGRUNS_M2		512
#define	LONGRUNS_M3		1000
#define	LONGRUNS_M4		10000

/*
 * Minimum number of bits for the longruns test.
 */
#define	LONGRUNS_MIN_BITS	100

/*
 * No maximum number of bits for the runs test defined.
 */
#define	LONGRUNS_MAX_BITS	0

TRAS_DECLARE_ALGO(longruns);

#endif
