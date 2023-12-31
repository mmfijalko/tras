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
#include <errno.h>
#include <stddef.h>

#include <tras.h>
#include <utils.h>
#include <frequency.h>
#include <blkfreq.h>

#define	min(a, b)	(((a) < (b)) ? (a) : (b))

struct blkfreq_ctx {
	unsigned int	sum;	/* partial sum of ones */
	unsigned int	nbits;	/* total number of bits updated */
	unsigned int	nblks;	/* full blocked updated */
	double		stats;	/* statistics for updated blocks */
	unsigned int	m;	/* block length in bits */
	double		alpha;	/* significance level from params */
};

static int
blkfreq_allow_final(struct blkfreq_ctx *c)
{

	if (c->nbits < BLKFREQ_MIN_N)
		return (0);
	if (c->nbits < c->m * 100)
		return (0);

	return (1);
}

int
blkfreq_init(struct tras_ctx *ctx, void *params)
{
	struct blkfreq_params *p = params;
	struct blkfreq_ctx *c;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);
	if (p->m < BLKFREQ_MIN_M)
		return (EINVAL);

	c = malloc(sizeof(struct blkfreq_ctx));
	if (c == NULL)
		return (ENOMEM);

	c->sum = 0;
	c->nbits = 0;
	c->nblks = 0;
	c->stats = 0.0;

	c->m = p->m;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &blkfreq_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
blkfreq_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct blkfreq_ctx *c;
	unsigned int i, k, n, nb, offs;
	double pii;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;
	n = c->nbits;

	c->nbits += nbits;

//	if (c->nblks >= (BLKFREQ_MAX_BLOCKS - 1))
//		return (0);

	n = n % c->m;
	if (n > 0) {
		k = min(nbits, c->m - n);
		c->sum += frequency_sum1(data, k);
		n = nbits - k;
		if (n == 0)
			return (0);
		offs = k;
		c->nblks++;
	} else {
		offs = 0;
		n = nbits;
	}

	nb = n / c->m;
	for (i = 0; i < nb; i++) {
//		if (c->nblks >= (BLKFREQ_MAX_BLOCKS - 1))
//			return (0);
		c->sum = frequency_sum1_offs(data, offs, c->m);
		pii = (double)c->sum / c->m - 0.5;
		c->stats += pii * pii;
		offs += c->m;
		c->nblks++;
	}
	n = n - nb * c->m;

	if (n != 0)
		c->sum = frequency_sum1_offs(data, offs, n);
	else
		c->sum = 0;

	return (0);
}

int
blkfreq_final(struct tras_ctx *ctx)
{
	struct blkfreq_ctx *c;
	double pvalue;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	if (!blkfreq_allow_final(c))
		return (EALREADY);

	c->stats = 4 * c->m * c->stats;

	/* todo: calculate P-value */
	pvalue = 0.0;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits % c->m;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0.0;

	return (0);
}

int
blkfreq_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
blkfreq_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
blkfreq_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo blkfreq_algo = {
	.name =		"Blkfreq",
	.desc =		"Frequency Test within a Block",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		blkfreq_init,
	.update =	blkfreq_update,
	.test =		blkfreq_test,
	.final =	blkfreq_final,
	.restart =	blkfreq_restart,
	.free =		blkfreq_free,
};
