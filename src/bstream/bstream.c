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
 * The Minimum Distance Test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include <tras.h>
#include <hamming8.h>
#include <utils.h>
#include <bits.h>
#include <bstream.h>

/*
 * The minimum distance test context.
 */
struct bstream_ctx {
	unsigned int	nbits;	/* number of bits processed */
	double		alpha;	/* significance level for H0 */
};

int
bstream_init(struct tras_ctx *ctx, void *params)
{
	struct bstream_ctx *c;
	struct bstream_params *p = params;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	c = malloc(sizeof(struct bstream_ctx));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}

	/* todo: other initializations when defined */

	c->nbits = 0;

	ctx->context = c;
	ctx->algo = &bstream_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
bstream_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct bstream_ctx *c;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	(void)c;

	/* todo: implementation */

	c->nbits += nbits;

	return (0);
}

int
bstream_final(struct tras_ctx *ctx)
{
	struct bstream_ctx *c;
	double pvalue, sobs;
	int sum;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	(void)c;

	/* todo: implementation */

	pvalue = 0.0;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits & 0x07;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
bstream_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
bstream_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
bstream_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo bstream_algo = {
	.name =		"bstream",
	.desc =		"The Bitstream Test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		bstream_init,
	.update =	bstream_update,
	.test =		bstream_test,
	.final =	bstream_final,
	.restart =	bstream_restart,
	.free =		bstream_free,
};
