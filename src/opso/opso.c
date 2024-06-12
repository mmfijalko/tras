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
 * The Overlapping Pairs Sparse Occupancy test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tras.h>
#include <cdefs.h>
#include <utils.h>
#include <bits.h>

#include <opso.h>

/*
 * The OPSO test context.
 */
struct opso_ctx {
	unsigned int	nbits;	/* number of bits processed */
	double		alpha;	/* significance level for H0 */
	uint32_t *	wmap;	/* bitmap for opso words */
	uint32_t	word;	/* last letter collected */
	unsigned int	letters;/* number of letters processed */
	unsigned int	sparse;	/* number of missing words */
};

int
opso_init(struct tras_ctx *ctx, void *params)
{
	struct opso_params *p = params;
	struct opso_ctx *c;
	int size, error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	size = sizeof(struct opso_ctx) + OPSO_WORDS / 8;

	error = tras_init_context(ctx, &opso_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);

	c->wmap = (uint32_t *)(c + 1);

	c->alpha = p->alpha;
	c->sparse = OPSO_WORDS;

	return (0);
}

int
opso_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct opso_ctx *c;
	unsigned int n, i, k;
	uint32_t *strokes, word;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	if (nbits & 0x1f)
		return (EINVAL);
	if (nbits == 0)
		return (0);

	c = ctx->context;

	if (c->nbits >= OPSO_MAX_BITS) {
		c->nbits += nbits;
		return (0);
	}

	strokes = (uint32_t *)data;
	n = nbits / 32;

	if (c->letters < 2) {
		k = min(n, 2 - c->letters);
		for (i = 0; i < k; i++) {
			c->word = (c->word << 10) & ~OPSO_LETTER_MASK;
			c->word |= ((strokes[i] >> 22) & OPSO_LETTER_MASK) &
			   OPSO_WORD_MASK;
		}
		c->nbits += k * 32;
		c->letters += k;
		if (c->letters < 2)
			return (0);
		strokes += k;
		n = n - k;
	}

	k = min(OPSO_LETTERS - c->letters, n);
	word = c->word;

	for (i = 0; i < k; i++) {
		word = ((word << 10) | ((strokes[i] >> 22) & 0x000003ff)) &
		    0x000fffff;
		if ((c->wmap[word >> 5] & (1 << (word & 0x1f))) == 0) {
			c->wmap[word >> 5] |= (1 << (word & 0x1f));
			c->sparse--;
		}
	}
	c->word = word;
	c->letters += n;
	c->nbits += n * 32;

	return (0);
}

int
opso_final(struct tras_ctx *ctx)
{
	struct opso_ctx *c;
	double pvalue, s;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;
	if (c->nbits < OPSO_MIN_BITS)
		return (EALREADY);

	s = (double)c->sparse - 141909.3299550069;
	s = fabs(s) / 290.4622634038 / sqrt((double)2.0);
	pvalue = erfc(fabs(s));

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - OPSO_BITS;
	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

	return (0);
}

int
opso_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
opso_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
opso_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo opso_algo = {
	.name =		"opso",
	.desc =		"Overlapping-Pairs-Sparse-Occupancy Test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		opso_init,
	.update =	opso_update,
	.test =		opso_test,
	.final =	opso_final,
	.restart =	opso_restart,
	.free =		opso_free,
};
