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
sparse_max_nbits(struct sparse_params *p)
{

	return (((p->wmax + p->k - 1) * p->r));
}

inline static unsigned int
sparse_min_nbits(struct sparse_params *p)
{

	return (sparse_max_nbits(p));
}

static void
sparse_init_context(struct sparse_ctx *c, void *params)
{
	struct sparse_params *p = &c->params;

	if (params != NULL)
		memcpy(p, params, sizeof(struct sparse_params));
	c->wmap = (uint8_t *)(c + 1);
	c->nbits = 0;
	c->alpha = p->alpha;
	c->letters = 0;
	c->lmax = p->wmax + p->k - 1;
	c->word = 0;
	c->sparse = (unsigned int)pow(p->m, p->k);
	c->lmask = (1 << p->b) - 1;
	c->wmask = (1 << (p->k * p->b)) - 1;
	memset(c->wmap, 0, c->sparse / 8);
}

int
sparse_init(struct tras_ctx *ctx, void *params)
{
	struct sparse_params *p = params;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	error = sparse_verify_params(ctx, p);
	if (error != 0)
		return (error);

	error = tras_init_context(ctx, &sparse_algo, sizeof(struct sparse_ctx) +
	    (size_t)pow(p->m, p->k) / 8, TRAS_F_ZERO);
	if (error != 0)
		return (error);

	sparse_init_context(ctx->context, params);

	return (0);
}

static uint32_t lmask[32] = {
	0x00000000, 0x00000001, 0x00000003, 0x00000007,
	0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
	0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
	0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
	/* ... */
};

static uint32_t hmask[32] = {
};

#define	SPARSE_WORD_LETTER(w, b, o)				\
	((w) >> (o)) & lmask[(b) & 0x1f] 			\
	(w) << (32 - (((o) + (b)) & 0x1f))


/* Add one stroke to the word w */
#define	SPARSE_WORD_SET(c, p, w, s) do {			\
	(w) = ((w) << (p)->b) & (c)->wmask;			\
	(w) = (w) | (((s) >> (32 - (p)->b)) & (c)->lmask);	\
} while (0)

/* Update words map and sparse counter with a word */
#define SPARSE_WMAP_SET(c, w) do {				\
	uint32_t mask, wpos;					\
	mask = 1 << ((w) & 0x07);				\
	wpos = (w) >> 3;					\
	if (((c)->wmap[wpos] & mask) == 0) {			\
		(c)->wmap[wpos] |= mask;			\
		(c)->sparse--;					\
	}							\
} while (0)

#define	miss(c, cmax)	(((c) < (cmax)) ? (cmax) - (c) : 0)

/* Simple shift right with window */
#define	SPARSE_SHRW(s, o, b)	\
	(((s) >> (o)) & ((1 << b) - 1))
/* Simple shift left with window */
#define	SPARSE_SHLW(s, o, b)	\
	(((s) << (o)) & ((1 << b) - 1))

#define	SPARSE_ROTR32(s, o)	\
	()
#define	SPARS_ROTL32(s, o)	\
	()
#define	SPARSE_ROTR32W(s, o, b)	\
	()
#define	SPARSE_ROTL32W(s, o, b)	\
	()

int
sparse_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct sparse_params *p;
	struct sparse_ctx *c;
	uint32_t *strokes, word;
	unsigned int n, i, k;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	if (nbits & 0x1f)
		return (EINVAL);

	c = ctx->context;
	p = &c->params;

	k = miss(c->letters, c->lmax);
	n = nbits / 32;
	n = min(k, n);
	if (n > 0) {
		strokes = (uint32_t *)data;
		word = c->word;
		if (c->letters < p->k) {
			k = min(n, p->k - c->letters);
			for (i = 0; i < k; i++)
				SPARSE_WORD_SET(c, p, word, strokes[i]);
			c->letters += k;
			if (c->letters < p->k) {
				c->word = word;
				c->nbits += nbits;
				return (0);
			}
			SPARSE_WMAP_SET(c, word);
			n = n - k;
			strokes += k;
		}
		for (i = 0; i < n; i++) {
			SPARSE_WORD_SET(c, p, word, strokes[i]);
			SPARSE_WMAP_SET(c, word);
		}
		c->word = word;
		c->letters += n;
	}
	c->nbits += nbits;

	return (0);
}

int
sparse_final(struct tras_ctx *ctx)
{
	struct sparse_ctx *c;
	struct sparse_params *p;
	double pvalue, s;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;
	p = &c->params;

	if (c->nbits < sparse_min_nbits(p))
		return (EALREADY);

	s = (double)c->sparse - p->mean;
	s = fabs(s) / p->var / sqrt((double)2.0);
	pvalue = erfc(fabs(s));

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - sparse_max_nbits(p);
	ctx->result.stats1 = (double)c->sparse;
	ctx->result.stats2 = s;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	tras_fini_context(ctx, 0);

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

	if (ctx == NULL)
		return (EINVAL);

	if (params != NULL)
		return (tras_do_restart(ctx, params));
	if (!TRAS_IS_INITED(ctx))
		return (ENXIO);

	sparse_init_context(ctx->context, NULL);

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

int
sparse_set_params(struct sparse_params *sp, const struct sparse_params *spin,
    struct oxso_params *params)
{
	struct oxso_params *p = (struct oxso_params *)params;

	TRAS_CHECK_PARA(p, p->alpha);

	memcpy(sp, spin, sizeof(struct sparse_params));

	sp->alpha = p->alpha;
	sp->boff = p->boff;

	return (0);
}

int
sparse_generic_restart(struct tras_ctx *ctx, const struct sparse_params *spin,
    void *params)
{
	struct sparse_params sp;
	void *p = params;
	int error;

	if (params != NULL) {
		error = sparse_set_params(&sp, spin, params);
		if (error != 0)
			return (error);
		p = (void *)&sp;
	}

	return (sparse_restart(ctx, p));
}
