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
 * todo: name
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tras.h>
#include <cdefs.h>
#include <const.h>
#include <approxe.h>
#include <lentz_gamma.h>

#include <stdio.h>

struct approxe_ctx {
	unsigned int 	nbits;	/* number of bits processed */
	uint32_t	first;	/* first m-1 appended bits */
	uint32_t	block;	/* last not full block */
	unsigned int *	freq0;	/* block value frequencies for m */
	unsigned int *	freq1;	/* block value frequencies for m + 1 */
	unsigned int	m;	/* bits for each block */
	double		alpha;	/* significance level for H0 */
};

int
approxe_init(struct tras_ctx *ctx, void *params)
{
	struct approxe_params *p = params;
	struct approxe_ctx *c;
	unsigned int i, n, error;
	size_t size;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->m < APPROXE_MIN_M || p->m > APPROXE_MAX_M)
		return (EINVAL);

	n = (unsigned int)pow(2.0, p->m);

	size = sizeof(struct approxe_ctx) + (p->m - 1 + 7) / 8 +
	   (n + 2 * n) * sizeof(unsigned int);

	error = tras_init_context(ctx, &approxe_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);

	c = ctx->context;
	c->freq0 = (unsigned int *)(c + 1);
	c->freq1 = (unsigned int *)(c->freq0 + n);

	c->m = p->m;
	c->alpha = p->alpha;

	return (0);
}

inline static uint32_t
approxe_get_sequence(int offs, int nbits, uint8_t *data)
{
	uint32_t seq = 0;
	uint8_t mask, b, *d;
	int n;

	d = data + offs / 8;

	while (nbits > 0) {
		b = *d++ & (0xff >> (offs & 0x07));
		n = 8 - (offs & 0x7);
		if (nbits < n) {
			b = b >> (n - nbits);
			n = nbits;
		}
		seq = (seq << n) | (uint32_t)b;
		offs += n;
		nbits -= n;
	}
	return (seq);
}

#define	EXTRACT_BIT(d, o)	\
	(((d)[(o) >> 3] >> (7 - ((o) & 0x07))) & 0x01)

static void
approxe_update_sequence(uint8_t *p, unsigned int offs, unsigned int nbits,
    unsigned int m, uint32_t block, unsigned int *freq)
{
	uint32_t mask;

	mask = (1 << m) - 1;

	while (nbits > 0) {
		block = (block << 1) & mask;
		block = block | EXTRACT_BIT(p, offs);
		freq[block]++;
		nbits--;
		offs++;
	}
}

int
approxe_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct approxe_ctx *c;
	unsigned int n, offs;
	uint32_t block;
	uint8_t *p;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	if (nbits == 0)
		return (0);

	c = ctx->context;
	p = (uint8_t *)data;

	offs = 0;
	if (c->nbits < c->m) {
		n = min(nbits, c->m - c->nbits);
		block = approxe_get_sequence(0, n, data);
		block = (c->first << n) | block;
		c->first = block;
		c->block = block;
		if ((c->nbits + n) < c->m) {
			c->nbits += nbits;
			return (0);
		}
		/* Collected m bits sequence, update for m */
		c->freq0[block]++;
		offs = n;
	}

	n = nbits - offs;

	/*
	 * update frequency table for sequence with m bits length.
	 */
	approxe_update_sequence(p, offs, n, c->m, c->block, c->freq0);

	/*
	 * update frequency table for sequence with m + 1 bits length.
	 */
	approxe_update_sequence(p, offs, n, c->m + 1, c->block, c->freq1);

	n = min(c->m, c->nbits);
	block = approxe_get_sequence(c->nbits - n, n, data);
	c->block = (c->block < n) | block;

	c->nbits += nbits;

	return (0);
}

int
approxe_final(struct tras_ctx *ctx)
{
	struct approxe_ctx *c;
	double pvalue, phim0, phim1, stats, *freq;
	unsigned int i, n, k;
	uint8_t d[4];
	int error;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nbits == 0 || (c->m >= (log2(c->nbits) - 5)))
		return (EALREADY);

	d[0] = (c->first >> 24) & 0xff;
	d[1] = (c->first >> 16) & 0xff;
	d[2] = (c->first >>  8) & 0xff;
	d[3] = (c->first >>  0) & 0xff;

	n = 32 - c->m;

	approxe_update_sequence(d, n, c->m - 1, c->m, c->block, c->freq0);
	approxe_update_sequence(d, n, c->m - 1, c->m + 1, c->block, c->freq1);

	k = 1 << c->m;
	n = c->nbits;

	freq = malloc(2 * sizeof(double) * k);
	if (freq == NULL)
		return (ENOMEM);

	/* Calculate relative frequencies for m */ 
	for (i = 0; i < k; i++) {
		freq[i] = ((double)c->freq0[i]) / (double)n;
	}
	/* Calculate phi value for m */
	for (i = 0, phim0 = 0.0; i < k; i++) {
		if (c->freq0[i] != 0)
			phim0 += freq[i] * log(freq[i]);
	}
	/* Calculate relative frequencies for m + 1 */
	for (i = 0, k = k * 2, phim1 = 0.0; i < k; i++) {
		freq[i] = ((double)c->freq1[i]) / (double)n;
	}
	/* Calculate phi value for m + 1 */
	for (i = 0, phim1 = 0.0; i < k; i++) {
		if (c->freq1[i] != 0)
			phim1 += freq[i] * log(freq[i]);
	}

printf("%s: phim0 = %f\n", __func__, phim0);
printf("%s: phim1 = %f\n", __func__, phim1);
	stats = (double)n * (log(2.0) - (phim0 - phim1));

printf("appen = %f, chi = %f\n", (phim0 - phim1), stats);
printf("chi / 2 = %f\n", stats / 2.0);

	/*
	 * todo: finalization is not finished since we don't have
	 * igammac implementation yet.
	 * pvalue = igamc(2 ^ (m - 1), chi2 / 2);
	 */
	pvalue = lentz2_gamma((double)(1 << (c->m - 1)), stats / 2.0, 10e-32, &error);
printf("getting lentz gamma (%g, %g) = %.16g\n",
    (double)(1 << (c->m - 1)), stats / 2.0, pvalue);
	pvalue = pvalue / tgamma((double)(1 << (c->m - 1)));

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = 0;
	ctx->result.stats1 = stats;
	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

	return (0);
}

/*
 * Test the data in continous memory region and finalize.
 */
int
approxe_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

/*
 * Restart the test with previous or new parameters.
 */
int
approxe_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

/*
 * Deallocate all resources for the tras context.
 */
int
approxe_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo approxe_algo = {
	.name =		"Approxe",
	.desc =		"Approximate Entropy Test",
	.version = 	{ 0, 1, 1 },
	.init =		approxe_init,
	.update =	approxe_update,
	.test =		approxe_test,
	.final =	approxe_final,
	.restart =	approxe_restart,
	.free =		approxe_free,
};
