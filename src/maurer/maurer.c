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
#include <hamming8.h>
#include <maurer.h>

/*
 * TODO: content, Maurer (Universal) test implmentation.
 *
 * TODO: Coron improvement of the Maurer test.
 */

int
maurer_init(struct tras_ctx *ctx, void *params)
{

	if (ctx == NULL || params != NULL)
		return (EINVAL);

	tras_ctx_init(ctx);

	ctx->algo = &maurer_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
maurer_update(struct tras_ctx *ctx, void *data, unsigned int bits)
{

	/* todo: */
	return (0);
}

int
maurer_final(struct tras_ctx *ctx)
{

	return (0);
}

int
maurer_test(struct tras_ctx *ctx, void *data, unsigned int bits)
{
	int error;

	error = maurer_update(ctx, data, bits);
	if (error != 0)
		return (error);

	error = maurer_final(ctx);
	if (error != 0)
		return (error);

	return (0);
}

int
maurer_restart(struct tras_ctx *ctx, void *params)
{

	return (0);
}

int
maurer_free(struct tras_ctx *ctx)
{

	return (0);
}

const struct tras_algo maurer_algo = {
	.name =		"Universal",
	.desc =		"Maurer's Universal Statistical Test",
	.version = 	{ 0, 1, 1 },
	.init =		maurer_init,
	.update =	maurer_update,
	.test =		maurer_test,
	.final =	maurer_final,
	.restart =	maurer_restart,
	.free =		maurer_free,
};
