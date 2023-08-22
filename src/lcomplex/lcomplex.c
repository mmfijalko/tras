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

#include <math.h>

#include "tras.h"

/*
 * TODO: implementation for the Linear Complexity Test.
 *
 * TODO: Theory, Berlekamp-Massey algorithm
 */

/*
 * The parameters for the Linear Complexity Test:
 * M - the length in bits of a block.
 * n - the length of the bit string.
 * K - the number of degrees of freedom, temporary K = 6 hardcoded.
 */

struct lcomplex_state {
	uint8_t	*	block;		/* storage for last, incomplete block */
	unsigned int	nblks;		/* number of full block processed */
	unsigned int	nbits;		/* number of bits updated */
	unsigned int *	vfreq;		/* chi-square frequency table for T */
};

struct lcomplex_params {
	unsigned int	M;		/* the length in bits of a block */
	unsigned int	K;		/* the number of degrees of freedom */
	double		u;		/* theoretical mean under H0 */
};

struct lcomplex_ctx {
	struct lcomplex_state *	s;	/* the linear complex test state */
	struct lcomplex_params *p;	/* the linear complex test params */
};

int lcomplex_update(void *ctx, void *data, unsigned int bits);
int lcomplex_final(void *ctx);

/*
 * XXX: very temporary definition for Proof of Concept.
 */

#define	tras_get_context(ctx)	(ctx)

int
lcomplex_init(void *ctx)
{
	struct lcomplex_ctx *c;

	c = malloc(sizeof(struct lcomplex_ctx) + sizeof(struct lcomplex_state) +
	    sizeof(struct lcomplex_params);
	if (c == NULL)
		return (ENOMEM);
	c->s = (struct lcomplex_state *)(c + 1);
	memset(c->s, 0, sizeof(struct lcomplex_state));
	c->p = (struct lcomplex_params *)(c->s + 1);
	c->p->M = 0;
	c->p->K = 0;
	c->p->u = 0.0;

	return (0);
}

/*
 * Chi-square sections probabilities for K = 6 as NIST calculated.
 */
static const double lcomplex_chi_prob[6 + 1] = {
	0.010417,	/* Pi#0 */
	0.031250,	/* Pi#1 */
	0.125000,	/* Pi#2 */
	0.500000,	/* Pi#3 */
	0.250000,	/* Pi#4 */
	0.062500,	/* Pi#5 */
	0.020833,	/* Pi#6 */
};

static double
chi2_statistics(unsigned int *v, double *p, unsigned int nv, unsigned int n)
{
	unsigned int i;
	double s, d;

	for (i = 0, s = 0.0; i < nv; i++) {
		d = v[i] - n * p[i];
		s = s + (d * d) / (n * p[i]);
	}
	return (s);
}

int
lcomplex_set_params(void *ctx, void *params)
{
	struct lcomplex_params *p = params;
	struct lcomplex_ctx *c;
	double u;

	c = tras_get_context(ctx);

	if (p->K < LINEARC_MIN_K || p->K > LINEARC_MAX_K)
		return (EINVAL);
	if (p->M < LINEARC_MIN_M || p->M > LINEARC_MAX_M)
		return (EINVAL);
	if (c->s->nbits != 0)
		return (ENXIO);

	c->s->block = malloc((p->M + 7) / 8);
	if (c->s->block == NULL)
		return (ENOMEM);
	memset((void *)c->s->block, 0, (p->M + 7) / 8);
	c->s->vfreq = malloc((p->K + 1) * sizeof(unsigned int));
	if (c->s->vfreq == NULL) {
		free(c->s->block);
		return (ENOMEM);
	}
	memset((void *)c->s->vfreq, 0, (p->K + 1) * sizeof(unsigned int));

	c->p->M = p->M;
	c->p->K = p->K;
	c->p->u = p->M / 2.0 + (9 + ((p->M & 0x01) ? 1 : -1)) / 36.0 +
	    (p->M / 3.0 + 2.0/9.0) / pow(2.0, p->M);

	return (0);
}

int
lcomplex_test(void *ctx, void *data, unsigned int bits)
{
	int error;

	error = lcomplex_update(ctx, data, bits);
	if (error != 0)
		return (error);

	error = lcomplex_final(ctx);
	if (error != 0)
		return (error);

	return (0);
}

static unsigned int
lcomplex_linearc(void *data, unsigned int boff, unsigned int bits)
{

	/*
	 * TODO: calculate linear complexity, prototype function.
	 */

	return (0);
}

static void
lcomplex_copy_block(void *dst, unsigned int doff, void *src, unsigned int soff,
    unsigned int nbits)
{

	/* TODO: Copy sequence of bits */
}

static void
lcomplex_update_block(struct lcomplex_ctx *c, int offs, void *data)
{

	/* TODO: single block update for linear complexity */
}

int
lcomplex_update(void *ctx, void *data, unsigned int bits)
{
	struct lcomplex_ctx *c = tras_get_context(ctx);
	struct lcomplex_params *p;
	unsigned int M, n, nblk, offs, full;
	
	/*
	 * TODO: multi-check for conditions to calculate.
	 */

	p = c->p;
	M = p->M;

	/* number of bits in the block buffer */
	n = c->nbits % M;

	if ((n + bits) < M) {
		/* Still not full block, concatenate sequence */
		lcomplex_copy_block(c->block, n, data, 0, bits);
		c->nbits += bits;
		return (0);
	}

	if (n != 0) {
		/* Some bits in the buffer and able to collect full block */
		lcomplex_copy_block(c->block, n, data, 0, M - n);
		lcomplex_update_block(c, 0, c->block);
		offs = M - n;
		full = 1;
	} else {
		/* There was nothing in the block buffer, just use data */
		offs = 0;
		full = 0;
	}
	nblk = (bits - offs) / M;
	n = (bits - offs) % M;

	/* Process linear complexity for full blocks of data */
	for (i = 0; i < nblk; i++, offs += M)
		lcomplex_update_block(c, offs, data);

	/* Copy not processed part of sequence to the block buffer */
	if (n != 0)
		lcomplex_copy_block(c->block, 0, data, offs, n);

	c->nblks += nblk + full;
	c->nbits += bits;

	return (0);
}

int
lcomplex_final(void *ctx)
{
	struct lcomplex_ctx *c = tras_get_context(ctx);
	struct lcomplex_state *s;

	/*
	 * XXX: condition for input length:
	 * N = floor(n / M)
	 */
	s = c->s;

	return (0);
}

int
lcomplex_free(void *ctx)
{
	struct lcomplex_ctx *c = tras_get_context(ctx);

	if (c != NULL)
		free(c);

	return (0);
}
