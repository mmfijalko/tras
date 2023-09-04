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
#include <stdlib.h>

#include <tras.h>

void
tras_ctx_init(struct tras_ctx *ctx)
{

	ctx->state = TRAS_STATE_NONE;
	ctx->context = NULL;
	ctx->algo = NULL;
}

void
tras_ctx_free(struct tras_ctx *ctx)
{

	/* todo: */
}

int
tras_test_init(struct tras_ctx *ctx, const struct tras_algo *algo, size_t size)
{

	if (ctx == NULL || algo == NULL)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EBUSY);

	ctx->context = malloc(size);
	if (ctx->context == NULL)
		return (ENOMEM);

	ctx->algo = algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
tras_test_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	/* todo: */

	return (0);
}

int
tras_verify_algo(const struct tras_algo *algo)
{

	if (algo == NULL)
		return (ENOSYS);
	if (algo->name == NULL || algo->desc == NULL)
		return (ENOSYS);
	if (algo->init == NULL || algo->update == NULL || algo->final == NULL)
		return (ENOSYS);

	return (0);
}

int
tras_do_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	int error;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state < TRAS_STATE_INIT)
		return (ENXIO);
	if (tras_verify_algo(ctx->algo) != 0) {
		ctx->state = TRAS_STATE_ERROR;
		return (ENOSYS);
	}
	error = ctx->algo->update(ctx, data, nbits);
	if (error != 0)
		return (error);
	error = ctx->algo->final(ctx);
	if (error != 0)
		return (error);

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
tras_test_final(struct tras_ctx *ctx)
{
	int error;

	if (ctx == NULL)
		return (EINVAL);

	if (ctx->algo->final == NULL) {
		ctx->state = TRAS_STATE_ERROR;
		return (ENOSYS);
	}

	error = ctx->algo->final(ctx);
	if (error != 0)
		return (error);

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
tras_test_restart(struct tras_ctx *ctx, void *params)
{

	/* todo: */

	return (0);
}

/*
 * Free test context. It calls the test free method.
 */
int
tras_test_free(struct tras_ctx *ctx)
{

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->algo == NULL)
		return (ENXIO);
	if (ctx->algo->free != NULL)
		return (ctx->algo->free(ctx));

	return (0);
}

int
tras_do_restart(struct tras_ctx *ctx, void *params)
{
	int error;

	error = tras_do_free(ctx);
	if (error != 0)
		return (error);
	return (ctx->algo->init(ctx, params));
}

int
tras_do_free(struct tras_ctx *ctx)
{

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state < TRAS_STATE_INIT)
		return (ENXIO);
	if (ctx->context != NULL) {
		free(ctx->context);
		ctx->context = NULL;
	}
	ctx->state = TRAS_STATE_NONE;

	return (0);
}
