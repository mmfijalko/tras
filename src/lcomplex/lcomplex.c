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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <math.h>

#include <tras.h>
#include <lcomplex.h>

/*
 * TODO: Theory, Berlekamp-Massey algorithm
 */

/*
 * The parameters for the Linear Complexity Test:
 * M - the length in bits of a block.
 * n - the length of the bit string.
 * K - the number of degrees of freedom, temporary K = 6 hardcoded.
 */

struct lcomplex_ctx {
	uint8_t	*	block;		/* storage for last, incomplete block */
	unsigned int	nblks;		/* number of full block processed */
	unsigned int	nbits;		/* number of bits updated */
	unsigned int *	vfreq;		/* chi-square frequency table for T */
	unsigned int	M;		/* the length of a block in bits */
	unsigned int	K;		/* degrees of freedom */
	unsigned int	u;		/* theoretical mean under H0 */
	double		alpha;		/* significance level */
};

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
lcomplex_init(struct tras_ctx *ctx, void *params)
{
	struct lcomplex_params *p = params;
	struct lcomplex_ctx *c;
	int size;

	if (ctx == NULL || params == NULL)
		return (EINVAL);

	if (p->K < LINEARC_MIN_K || p->K > LINEARC_MAX_K)
		return (EINVAL);
	if (p->M < LINEARC_MIN_M || p->M > LINEARC_MAX_M)
		return (EINVAL);
	if (p->u != 0)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);

	size = sizeof(struct lcomplex_ctx);
	size += (p->M + 7) / 8;
	size += (p->K + 1) * sizeof(unsigned int);

	c = malloc(size);
	if (c == NULL)
		return (ENOMEM);

	c->M = p->M;
	c->K = p->K;
	c->u = p->M / 2.0 + (9 + ((p->M & 0x01) ? 1 : -1)) / 36.0 +
	    (p->M / 3.0 + 2.0/9.0) / pow(2.0, p->M);
	c->alpha = p->alpha;

	c->block = (uint8_t *)(c + 1);
	memset((void *)c->block, 0, (p->M + 7) / 8);

	c->vfreq = (unsigned int *)(c->block + ((p->M + 7) / 8));
	memset((void *)c->vfreq, 0, (p->K + 1) * sizeof(unsigned int));

	ctx->context = c;
	ctx->algo = &lcomplex_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
lcomplex_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct lcomplex_ctx *c = ctx->context;
	unsigned int M, n, nblk, offs, full, i;
	
	/*
	 * TODO: multi-check for conditions to calculate.
	 */

	M = c->M;

	/* number of bits in the block buffer */
	n = c->nbits % M;

	if ((n + nbits) < M) {
		/* Still not full block, concatenate sequence */
		lcomplex_copy_block(c->block, n, data, 0, nbits);
		c->nbits += nbits;
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
	nblk = (nbits - offs) / M;
	n = (nbits - offs) % M;

	/* Process linear complexity for full blocks of data */
	for (i = 0; i < nblk; i++, offs += M)
		lcomplex_update_block(c, offs, data);

	/* Copy not processed part of sequence to the block buffer */
	if (n != 0)
		lcomplex_copy_block(c->block, 0, data, offs, n);

	c->nblks += nblk + full;
	c->nbits += nbits;

	return (0);
}

int
lcomplex_final(struct tras_ctx *ctx)
{
	struct lcomplex_ctx *c = ctx->context;
	double pvalue;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	/* todo: here calculation of statistics */
	pvalue = 0.0;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = 0;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	/*
	 * XXX: condition for input length:
	 * N = floor(n / M)
	 */

	return (0);
}

int
lcomplex_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
lcomplex_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
lcomplex_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo lcomplex_algo = {
	.name =		"Frequency Test",
	.desc =		"Generic Frequency (Monobit) Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		lcomplex_init,
	.update =	lcomplex_update,
	.test =		lcomplex_test,
	.final =	lcomplex_final,
	.restart =	lcomplex_restart,
	.free =		lcomplex_free,
};
