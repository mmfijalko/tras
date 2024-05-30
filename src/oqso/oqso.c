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
 * todo:
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include <tras.h>
#include <utils.h>
#include <bits.h>

#include <hamming8.h>
#include <oqso.h>

/*
 * todo.
 */
struct oqso_ctx {
	unsigned int	nbits;	/* number of bits processed */
	double		alpha;	/* significance level for H0 */
};

int
oqso_init(struct tras_ctx *ctx, void *params)
{
	struct oqso_params *p = params;
	struct oqso_ctx *c;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	c = malloc(sizeof(struct oqso_ctx));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}

	/* todo: other initializations when defined */

	c->nbits = 0;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &oqso_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
oqso_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	/*
	 * TODO: implementation.
	 */

	return (ENOSYS);
}

int
oqso_final(struct tras_ctx *ctx)
{
	struct oqso_ctx *c;
	double pvalue, sobs;
	int sum;

	TRAS_CHECK_FINAL(ctx);

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
oqso_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
oqso_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
oqso_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo oqso_algo = {
	.name =		"oqso",
	.desc =		"Overlapping-Quadruples-Sparse-Occupancy Test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		oqso_init,
	.update =	oqso_update,
	.test =		oqso_test,
	.final =	oqso_final,
	.restart =	oqso_restart,
	.free =		oqso_free,
};
