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

#ifndef __TRAS_MAURER_H__
#define	__TRAS_MAURER_H__

/*
 * The private context for Universal Statistical test (Maurer, Coron).
 */
struct universal_ctx {
	uint32_t	block;		/* to store not full block */
	unsigned int	nbits;		/* number of bits processed */
	double		alpha;		/* significance level */
	unsigned int	L;		/* the length of each block */
	unsigned int	Q;		/* the number of init block */
	unsigned int 	K;		/* number of L-blocks processed */
	unsigned int *	lblks;		/* last occurence of L-blocks */
	double		stats;		/* statistic sum of log distance */
};

#define	UNIVERSAL_ID_MAURER	0
#define	UNIVERSAL_ID_CORON	1

struct universal_params {
	unsigned int		L;
	unsigned int		Q;
	double			alpha;
};

#define	UNIVERSAL_MIN_L		6
#define	UNIVERSAL_MAX_L		16

TRAS_DECLARE_ALGO(maurer);

int maurer_init_algo(struct tras_ctx *, void *, const struct tras_algo *);

#endif
