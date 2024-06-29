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
#include <string.h>
#include <math.h>

#include <tras.h>
#include <utils.h>
#include <bits.h>
#include <bmrank.h>
#include <brank68.h>

int
brank68_init(struct tras_ctx *ctx, void *params)
{
	struct brank68_params *p = params;
	struct bmrank_params bmrankp;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	size = sizeof(struct tras_ctx);

	error = tras_init_context(ctx, &brank68_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);

	bmrankp.uniform = 1;
	bmrankp.m = 6;
	bmrankp.q = 8;
	bmrankp.nr = 2;
	bmrankp.s0 = p->byte * 8;
	bmrankp.N = 100000;
	bmrankp.alpha = p->alpha;

	error = bmrank_init((struct tras_ctx *)ctx->context, &bmrankp);
	if (error != 0) {
		tras_fini_context(ctx, 0);
		return (error);
	}

	return (0);
}

int
brank68_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	return (bmrank_update((struct tras_ctx *)ctx->context, data, nbits));
}

int
brank68_final(struct tras_ctx *ctx)
{
	struct tras_ctx *c;
	int error;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	error = bmrank_final(c);
	if (error != 0)
		return (error);

	memcpy(&ctx->result, &c->result, sizeof(ctx->result));

	tras_fini_context(ctx, 0);

	return (0);
}

int
brank68_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
brank68_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
brank68_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo brank68_algo = {
	.name =		"brank68",
	.desc =		"The Binary Rank Test for 6 x 8 Matrices",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		brank68_init,
	.update =	brank68_update,
	.test =		brank68_test,
	.final =	brank68_final,
	.restart =	brank68_restart,
	.free =		brank68_free,
};
