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
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>

#include <tras.h>
#include <excursionv.h>

/*
 * Private context for random excursion variant test.
 */
struct excursionv_ctx {
	double	alpha;		/* significance level */
};

int
excursionv_init(struct tras_ctx *ctx, void *params)
{
	struct excursionv_ctx *c;
	struct excursionv_params *p = params;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	c = malloc(sizeof(struct excursionv_ctx));
	if (c == NULL)
		return (ENOMEM);

	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &excursionv_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
excursionv_update(struct tras_ctx *ctx, void *data, unsigned int bits)
{

	/* todo: */
	return (0);
}

int
excursionv_final(struct tras_ctx *ctx)
{
	struct excursionv_ctx *c;
	double pvalue;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	/* todo: check min bits */

	c = ctx->context;

	/* todo: here calculation of statistics */
	pvalue = 0.0;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = 0;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
excursionv_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
excursionv_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
excursionv_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo excursionv_algo = {
	.name =		"ExcursionV",
	.desc =		"Random Excursion Variant Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		excursionv_init,
	.update =	excursionv_update,
	.test =		excursionv_test,
	.final =	excursionv_final,
	.restart =	excursionv_restart,
	.free =		excursionv_free,
};
