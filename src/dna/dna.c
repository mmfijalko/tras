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
 * The DNA Test.
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
#include <dna.h>
#include <sparse.h>
#include <cdefs.h>

/*
 * The DNA test context
 */
struct dna_ctx {
	unsigned int	nbits;	/* number of bits processed */
	double		alpha;	/* significance level for H0 */
	uint32_t *	wmap;	/* bits map for DNA words */
	unsigned int	letters;/* letters in context */
	uint32_t	word;	/* last 3 words collected */
	unsigned int	sparse;	/* number of missing words */
};

int
dna_init(struct tras_ctx *ctx, void *params)
{
	struct dna_ctx *c;
	struct dna_params *p = params;
	int size;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	size = sizeof(struct dna_ctx) + DNA_WORDS / 8;

	c = malloc(size);
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}
	c->wmap = (uint32_t *)(c + 1);
	memset(c->wmap, 0, DNA_WORDS / 8);

	c->nbits = 0;
	c->alpha = p->alpha;
	c->letters = 0;
	c->sparse = DNA_WORDS;
	ctx->context = c;
	ctx->algo = &dna_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
dna_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct dna_ctx *c;
	uint32_t *strokes, word;
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

	strokes = (uint32_t *)data;
	n = nbits / 32;

	k = min(c->letters, 10);
	k = min(n, 10 - k);

	for (i = 0; i < k; i++) {
		c->word = (c->word << 2) & ~DNA_L_MASK;
		c->word |= (strokes[i] & DNA_LETTER_MASK) &
		    DNA_WORD_MASK;
	}
	c->letters += k;
	if (c->letters < 10) {
		c->nbits += nbits;
		return (0);
	}

	n = n - k;
	n = min(DNA_LETTERS, n);

	strokes += k;
	word = c->word;
	for (i = 0; i < n; i++) {
		word = ((word << 2) | strokes[i] & DNA_LETTER_MASK) &
		    DNA_WORD_MASK;
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
dna_final(struct tras_ctx *ctx)
{
	struct dna_ctx *c;
	double pvalue, s;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	c = ctx->context;
	if (c->nbits < DNA_MIN_BITS)
		return (EALREADY);

	s = (double)c->sparse - 141910.4026047629;
	s = fabs(s) / 337.0 / sqrt((double)2.0);
	pvalue = erfc(s);

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - DNA_BITS;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
dna_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
dna_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
dna_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo dna_algo = {
	.name =		"dna",
	.desc =		"Four Letters C,G,A,T words test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		dna_init,
	.update =	dna_update,
	.test =		dna_test,
	.final =	dna_final,
	.restart =	dna_restart,
	.free =		dna_free,
};

static struct sparse_params sparse_params_dna = {
	.m = 4,
	.k = 10,
	.b = 2,
	.r = 32,
	.wmax = SPARSE_MAX_WORDS,
	.mean = 141910.4026047629,
	.var = 337,0,
	.alpha = 0.01,
};
int
dna_sparse_init(struct tras_ctx *ctx, void *params)
{

	if (params != NULL)
		return (EINVAL);

	return (sparse_init(ctx, &sparse_params_dna));
}

int
dna_sparse_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (sparse_update(ctx, data, nbits));
}

int
dna_sparse_final(struct tras_ctx *ctx)
{

	return (sparse_final(ctx));
}

int
dna_sparse_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (sparse_test(ctx, data, nbits));
}

int
dna_sparse_restart(struct tras_ctx *ctx, void *params)
{

	if (params != NULL)
		return (EINVAL);

	return (sparse_restart(ctx, &sparse_params_dna));
}

int
dna_sparse_free(struct tras_ctx *ctx)
{

	return (sparse_free(ctx));
}

const struct tras_algo dna_sparse_algo = {
	.name =		"dna_sparse",
	.desc =		"Four Letters C,G,A,T words test using sparse.",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		dna_sparse_init,
	.update =	dna_sparse_update,
	.test =		dna_sparse_test,
	.final =	dna_sparse_final,
	.restart =	dna_sparse_restart,
	.free =		dna_sparse_free,
};
