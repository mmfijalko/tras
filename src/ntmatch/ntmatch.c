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
#include <utils/hamming8.h>
#include <ntmatch.h>

/*
 * Private context for the test.
 */
struct ntmatch_ctx {
	unsigned int	nbits;
	double		alpha;
};

int
ntmatch_init(struct tras_ctx *ctx, void *params)
{
	struct ntmatch_ctx *c;
	struct ntmatch_params *p = params;
	size_t size;

	if (ctx == NULL || params != NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	c = malloc(sizeof(struct ntmatch_ctx));
	if (c == NULL)
		return (ENOMEM);

	/* TODO: check parameters and allocated tables */

	c->nbits = 0;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &ntmatch_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
ntmatch_update(struct tras_ctx *ctx, void *data, unsigned int bits)
{

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	/* todo: */
	return (0);
}

int
ntmatch_final(struct tras_ctx *ctx)
{
	struct ntmatch_ctx *c;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	/* todo: implementation */

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
