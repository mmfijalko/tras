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
#include <errno.h>
#include <stddef.h>

#include <tras.h>
#include <approxe.h>

struct approxe_ctx {
	unsigned int 	nbits;	/* number of bits processed */
	uint8_t	*	first;	/* first m-1 appended bits */
	unsigned int *	freq0;	/* block value frequencies for m */
	unsigned int *	freq1;	/* block value frequencies for m + 1 */
	unsigned int	m;	/* bits for each block */
	double		alpha;	/* significance level for H0 */
};

int
approxe_init(struct tras_ctx *ctx, void *params)
{
	struct approxe_ctx *c;
	struct approxe_params *p = params;
	unsigned int n;

	if (ctx == NULL || params == NULL) {
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (p->m < APPROXE_MIN_M || p->m > APPROXE_MAX_M)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	n = (unsigned int)(pow(2.0, p->m));

	c = malloc(sizeof(struct approxe_ctx) + (p->m - 1 + 7) / 8 +
	   (n + 2 * n) * sizeof(unsigned int));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}
	c->freq0 = (unsigned int *)(c + 1);
	c->freq1 = (unsigned int *)(c->freq0 + n);
	c->first = (uint8_t *)(c->freq1 + n * 2);

	for (i = 0; i < n; i++)
		c->freq0[i] = 0;
	for (i = 0; i < 2 * n; i++)
		c->freq1[i] = 0;

	c->nbits = 0;
	c->m = p->m;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &approxe_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
approxe_update(struct tras_ctx *ctx, void *data, unsigned int bits)
{
	struct approxe_ctx *c;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	/* todo */

	return (ENOSYS);
}

int
approxe_final(struct tras_ctx *ctx)
{
	struct approxe_ctx *c;
	double pvalue, phim0, phim1;
	unsigned int i, n, *freq;

	n = (unsigned int)(pow(2.0, c->m));

	freq = malloc(2 * sizeof(double) * n);
	if (freq == NULL)
		return (ENOMEM);

	/* Calculate relative frequencies for m */ 
	for (i = 0; i < n; i++) {
		freq[i] = ((double)c->freqm0[i]) / n;
	}
	/* Calculate phi value for m */
	for (i = 0, phim0 = 0.0; i < n; i++) {
		phim0 += freq[i] * log(freq[i]);
	}
	/* Calculate relative frequencies for m + 1 */
	for (i = 0, n = n * 2, phim1 = 0.0; i < n; i++) {
		freq[i] = ((double)c->freqm1[i]) / n;
	}
	/* Calculate phi value for m + 1 */
	for (i = 0, phim1 = 0.0; i < n; i++) {
		phim1 += freq[i] * log(freq[i]);
	}
	return (0);
}

int
approxe_test(struct tras_ctx *ctx, void *data, unsigned int bits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
approxe_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

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
