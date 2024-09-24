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
 * The Squeeze Test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tras.h>
#include <cdefs.h>
#include <squeeze.h>

/*
 * The context structure for the squeeze test.
 */
struct squeeze_ctx {
	unsigned int	nint;	/* number of integers processed */
	unsigned int *	freqt;	/* frequency table for iterations */
	unsigned int	nfreq;	/* number of slots for chi-square */
	unsigned int	klast;	/* squeezed value from last update */
	unsigned int	ilast;	/* the iterations from last update */
	unsigned int	nword;	/* the number of words updated */
	unsigned int	K;	/* max number of integers */
	unsigned int	nbits;	/* number of bits processed */
	double		alpha;	/* significance level for H0 */
};

/*
 * Initialize the squeeze test.
 */
int
squeeze_init(struct tras_ctx *ctx, void *params)
{
	struct squeeze_params *p = params;
	struct squeeze_ctx *c;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->K < SQUEEZE_MIN_NUMBERS || p->K > SQUEEZE_MAX_NUMBERS)
		return (EINVAL);

	size = sizeof(struct squeeze_ctx) + SQUEEZE_CHI_SQUARE_SLOTS *
	    sizeof(unsigned int);

	error = tras_init_context(ctx, &squeeze_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);
	c = ctx->context;

	c->freqt = (unsigned int *)(c + 1);
	c->nfreq = SQUEEZE_CHI_SQUARE_SLOTS;
	c->klast = 2147483647;
	c->K = p->K;
	c->alpha = p->alpha;

	return (0);
}

inline static double
squeeze_uint_to_floatU01(uint32_t u32)
{

	/* XXX: this is wrong function that need to be corrected
	 * because since u32 is uniformly distributed the
	 * return value is not.
	 */
	return ((double)u32 / pow(2.0, 32));
}

/*
 * Update state of the squeeze test with subsequent binary sequence.
 */
int
squeeze_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct squeeze_ctx *c;
	unsigned int i, n, j, k;
	double u;
	uint32_t *p;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	if (nbits == 0 || (nbits & 0x1f))
		return (EINVAL);

	c = ctx->context;
	p = (uint32_t *)data;	/* TODO: consider endianism */

	/* Restore k and i from last update */
	k = c->klast;
	i = c->ilast;

	/* Get maximum number of squeezes and words from data */
	n = nbits / 32;
	j = min(c->nint, c->K);
	j = c->K - j;

	while (n > 0 && j > 0) {
		while (n > 0 && i < 48 && k != 1) {
			u = squeeze_uint_to_floatU01(*p);
			k = (unsigned int)ceil(u * k);
			i++;
			u++;
			n--;
			p++;
		}
		if (i >= 48 || k == 1) {
			i = (i < 6) ? 0 : i - 6;
			c->freqt[i]++;
			c->nint++;
			k = 2147483647;
			i = 0;
			j--;
		}
	}
	c->nword += nbits / 32 - n;
	c->klast = k;
	c->ilast = i;
	c->nbits += nbits;

	return (0);
}

/*
 * Finalize the squeeze test and determine its result.
 */
int
squeeze_final(struct tras_ctx *ctx)
{
	struct squeeze_ctx *c;
	double pvalue, mean;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nint < c->K)
		return (EALREADY);

	pvalue = 0.0;

	/*
	 * Determine and store results.
	 */
	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - c->nword * sizeof(uint32_t) * 8;
	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

	return (0);
}

/*
 * The squeeze test for update and finalize in one call.
 */
int
squeeze_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

/*
 * Restart the initialized squeeze test.
 */
int
squeeze_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

/*
 * Uninitialize the squeeze test and free its allocated resources.
 */
int
squeeze_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

/*
 * The squeeze test algorithm description for tras software.
 */
const struct tras_algo squeeze_algo = {
	.name =		"squeeze",
	.desc =		"Squeeze Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		squeeze_init,
	.update =	squeeze_update,
	.test =		squeeze_test,
	.final =	squeeze_final,
	.restart =	squeeze_restart,
	.free =		squeeze_free,
};
