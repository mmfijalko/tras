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

#ifndef __SPARSE_H__
#define	__SPARSE_H__

/*
 * The structure with all parameters for generic sparse occupancy.
 */
struct sparse_params {
	unsigned int	m;	/* number of letters in alphabet */
	unsigned int	k;	/* number of letters in a word */
	unsigned int	b;	/* number of bits for a letter */
	unsigned int	r;	/* number of bits in one rng word */
	unsigned int	boff;	/* offset of bits in one stroke int */
	unsigned int	wmax;	/* max number of words to update */
	double		mean;	/* mean of missing words */
	double		var;	/* variance of normal statistics */
	double		alpha;	/* significance level for H0 */
};

/*
 * Only needed parameters for specific sparse occupancy test.
 */
struct oxso_params {
	unsigned int	boff;	/* offset of bits in one stroke int */
	double		alpha;	/* significance level for H0 */
};

/* The number of possible words, 2 ^ 20 */
#define	SPARSE_WORDS		1048576	

/* Max number of sparse test update */
#define	SPARSE_MAX_WORDS	(2 * SPARSE_WORDS)

/*
 * The Sparse Occupancy test context. 
 */
struct sparse_ctx {
	unsigned int	nbits;	/* number of bits processed */
	double		alpha;	/* significance level for H0 */
	uint8_t *	wmap;	/* bits map for DNA words */
	unsigned int	letters;/* letters in context */
	unsigned int	lmax;	/* max letters to update */
	uint32_t	word;	/* last 3 words collected */
	unsigned int	sparse;	/* number of missing words */
	uint32_t	lmask;	/* precalculated mask for letter */
	uint32_t	wmask;	/* precalculated mask for word */
	struct sparse_params params; /* backup of params */
};

int sparse_set_params(struct sparse_params *sp,
    const struct sparse_params *spin, struct oxso_params *params);
int sparse_generic_restart(struct tras_ctx *ctx,
    const struct sparse_params *spin, void *params);

TRAS_DECLARE_ALGO(sparse);

#endif


