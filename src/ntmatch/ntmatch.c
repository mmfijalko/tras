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

/*
 * Implementation of the Non-overlapping Template Matching Test.
 */

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>

#include <tras.h>
#include <cdefs.h>
#include <ntmatch.h>

	#include <stdio.h>
/*
 * Private context for the test.
 */
struct ntmatch_ctx {
	unsigned int *	w;	/* templates frequency table */
	unsigned int	m;	/* the length of each template */
	unsigned int	M;	/* the length of each substring */
	unsigned int 	N;	/* the number of independent blocks */
	uint32_t	B;	/* the m-bits template to be matched */
	unsigned int 	nbmax;	/* max number of bits to be tested */
	unsigned int	wbits;	/* the number of valid bits in the word */
	uint32_t	word;	/* word saved from last update */
	unsigned int	nbits;	/* number of bits updated */
	double		alpha;	/* the significance level for H0 */
};

static int
ntmatch_check_template(uint32_t B)
{

	/*
	 * TODO: implementation.
	 */

	return (1);
}

int
ntmatch_init(struct tras_ctx *ctx, void *params)
{
	struct ntmatch_params *p = params;
	struct ntmatch_ctx *c;
	int size, error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->m < NTMATCH_MIN_M || p->m > NTMATCH_MAX_M)
		return (EINVAL);
	if (p->M < NTMATCH_MIN_SUBS_LEN || p->N < NTMATCH_MIN_N)
	       return (EINVAL);
	if (!ntmatch_check_template(p->B))
		return (EINVAL);

	size = sizeof(struct ntmatch_ctx) + p->N * sizeof(unsigned int);

	error = tras_init_context(ctx, &ntmatch_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);

	c = ctx->context;
	c->w = (unsigned int *)(c + 1);

	c->m = p->m;
	c->M = p->M;
	c->N = p->N;
	c->B = p->B;
	c->nbmax = c->N * c->M;
	c->alpha = p->alpha;

	return (0);
}

int
ntmatch_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct ntmatch_ctx *c;
	uint8_t *p, bm;
	unsigned int n, o, i, m, k;
	unsigned int b, wbits;
	uint32_t mask, word;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;
	p = (uint8_t *)data;

	/* the number of bits to update */
	n = min(c->nbits, c->nbmax);
	n = min(c->nbmax - n, nbits);

	/* the block number and the offset in the block */
	b = c->nbits / c->M;
	o = c->nbits % c->M;

	/* the number of valid bits in the saved word */
	wbits = c->wbits;

	/* the mask for template and the word from previous update */
	word = (o != 0) ? c->word : 0;
	mask = (1 << c->m) - 1;

	while (n > 0) {
		k = c->M - o;
		k = min(k, n);
		for (i = 0, bm = 0x80; i < k; i++) {
			word = ((word << 1) | ((*p & bm) ? 1:0)) & mask;
			if (wbits >= c->m) {
				/* the next full m-bits word with one new bit */
				if (c->B == word) {
					c->w[b]++;
					word = 0;
					wbits = 0;
				}
			} else {
				/* still constructing m-bits word */
				wbits++;
			}
			bm = bm >> 1;
			if (bm == 0x00) {
				bm = 0x80;
				p++;
			}
		}
		o = o + k;
		if (o == c->M) {
			o = 0;
			wbits = 0;
			word = 0;
			b++;
		}
		n = n - k;
	}
	c->word = word;
	c->wbits = wbits;

	c->nbits += nbits;

	return (0);
}

int
ntmatch_final(struct tras_ctx *ctx)
{
	struct ntmatch_ctx *c;
	double mean, var;
	double chi2, pvalue;
	unsigned int i, k;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nbits < c->nbmax)
		return (EALREADY);

#if 0
	printf("frequency table for ntmatch algorithm:\n");
	for (i = 0; i < c->N; i++) {
		printf("w[%d] = %u\n", i, c->w[i]);
	}
	printf("\n");
#endif

	mean = (double)c->M - (double)c->m + 1.0;
	var = (double)c->M;
	i = c->m;
	while (i > 0) {
		k = min(i, 16);
		mean = mean / (double)(1UL << k);
		i = i - k;
	}

	for (chi2 = 0.0, i = 0; i < c->N; i++)
		chi2 += ((double)c->w[i] - mean) * ((double)c->w[i] - mean);
	chi2 = chi2 / var;

#ifdef __not_yet__
	pvalue = igamc((double)c->N / 2.0, chi2 / 2.0);
#else
	pvalue = 0.0;
#endif
	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits % c->M;
	ctx->result.stats1 = chi2;	/* XXX: temp */
	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

	return (0);
}

int
ntmatch_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
ntmatch_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
ntmatch_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo ntmatch_algo = {
	.name =		"ntmatch",
	.desc =		"Non-Overlapping Template Matching Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		ntmatch_init,
	.update =	ntmatch_update,
	.test =		tras_do_test,
	.final =	ntmatch_final,
	.restart =	tras_do_restart,
	.free =		tras_do_free,
};
