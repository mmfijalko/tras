/*-
 *
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
 * The Sparse Occupancy test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tras.h>
#include <hamming8.h>
#include <utils.h>
#include <bits.h>
#include <sparse.h>
#include <cdefs.h>

#include <stdio.h>

static int
sparse_verify_params(struct tras_ctx *ctx, struct sparse_params *p)
{

	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (p->k == 0 || p->b == 0)
		return (EINVAL);
	if (p->k * p->b > 32)
		return (EINVAL);
	if (p->m == 0 || ((1 << p->b) != p->m))
		return (EINVAL);
	if (p->mean <= 0.0 || p->var <= 0.0)
		return (EINVAL);
	if (p->wmax != SPARSE_MAX_WORDS)
		return (EINVAL);
	if (p->r == 0 || p->r > 32)
		return (EINVAL);
	return (0);
}

inline static unsigned int
sparse_max_nbits(struct sparse_ctx *c, struct sparse_params *p)
{

	return (((p->wmax + p->k - 1) * p->r));
}

inline static unsigned int
sparse_min_nbits(struct sparse_ctx *c, struct sparse_params *p)
{

	return (sparse_max_nbits(c, p));
}

int
sparse_init(struct tras_ctx *ctx, void *params)
{
	struct sparse_ctx *c;
	struct sparse_params *p = params;
	unsigned int nwords;
	int error;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	error = sparse_verify_params(ctx, p);
	if (error != 0)
		return (error);

	nwords = (unsigned int)pow((double)p->m, (double)p->k);

	c = malloc(sizeof(struct sparse_ctx) + nwords / 8);
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}
	c->wmap = (uint32_t *)(c + 1);
	memset(c->wmap, 0, nwords / 8);
	memcpy(&c->params, params, sizeof(struct sparse_params));

	c->nbits = 0;
	c->alpha = p->alpha;
	c->letters = 0;
	c->lmax = p->wmax + p->k - 1;
	c->sparse = nwords;
	c->word = 0;
	ctx->context = c;
	ctx->algo = &sparse_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
sparse_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct sparse_params *p;
	struct sparse_ctx *c;
	uint32_t *strokes, word, lmask, wmask;
	unsigned int n, i, k;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	if (nbits & 0x1f)
		return (EINVAL);
	if (nbits == 0)
		return (0);

	c = ctx->context;
	p = &c->params;

	if (c->letters >= c->lmax) {
		c->nbits += nbits;
		return (0);
	}

	strokes = (uint32_t *)data;
	n = nbits / 32;

	k = min(c->letters, p->k);
	k = min(n, p->k - k);

	lmask = (1 << p->b) - 1;
	wmask = (1 << (p->k * p->b)) - 1;

	for (i = 0; i < k; i++) {
		c->word = ((c->word << p->b) & ~lmask) & wmask;
		c->word |= ((strokes[i] >> (32 - p->b)) & lmask);
	}
	c->letters += k;
	if (c->letters < p->k) {
		c->nbits += nbits;
		return (0);
	}

	n = n - k;
	n = min(n, c->lmax - c->letters);

	strokes += k;
	word = c->word;
	for (i = 0; i < n; i++) {
		word = ((word << p->b) | ((strokes[i] >> (32 - p->b)) & lmask)) &
		   wmask;
		if ((c->wmap[word >> 5] & (1 << (word & 0x1f))) == 0) {
			c->wmap[word >> 5] |= (1 << (word & 0x1f));
			c->sparse--;
		}
	}
	c->word = word;
	c->letters += n;
	c->nbits += nbits;

	return (0);
}

int
sparse_final(struct tras_ctx *ctx)
{
	struct sparse_ctx *c;
	struct sparse_params *p;
	double pvalue, s;

	if (ctx == NULL || ctx->context == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;
	p = &c->params;

	if (c->nbits < sparse_min_nbits(c, p))
		return (EALREADY);

	s = (double)c->sparse - p->mean;
	s = fabs(s) / p->var / sqrt((double)2.0);
	pvalue = erfc(fabs(s));

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - sparse_max_nbits(c, p);
	ctx->result.stats1 = (double)c->sparse;
	ctx->result.stats2 = s;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
sparse_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
sparse_restart(struct tras_ctx *ctx, void *params)
{
	struct sparse_ctx *c;
	struct sparse_params *p;
	unsigned int nwords;

	if (ctx == NULL)
		return (EINVAL);

	if (params != NULL)
		return (tras_do_restart(ctx, params));

	if (ctx->state != TRAS_STATE_FINAL)
		return (ENXIO);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;
	p = &c->params;

	nwords = (unsigned int)pow((double)p->m, (double)p->k);

	c->nbits = 0;
	c->letters = 0;
	c->sparse = nwords;
	memset(c->wmap, 0, nwords / 8);

	return (0);
}

int
sparse_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo sparse_algo = {
	.name =		"sparse",
	.desc =		"Generic Sparse Occupancy test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		sparse_init,
	.update =	sparse_update,
	.test =		sparse_test,
	.final =	sparse_final,
	.restart =	sparse_restart,
	.free =		sparse_free,
};
