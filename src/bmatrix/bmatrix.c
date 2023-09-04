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
 * Test with two parameters:
 * - M - number of rows in each matrix.
 * - Q - number of columns in each matrix.
 *
 * For parameters other than 32 x 32 the approximation need to
 * be recalculated as NIST algorithm descrition says.
 *
 * For particular M, Q the minimum 38 matrices need to be crated, so
 * minimum number of bits is n >= 38MQ. If n is not multiple of MQ
 * the n % (MQ) bits will be discarded.
 *
 * Number of blocks (matrices) N = lower(n / MQ)
 *
 * Rank of the binary matrices needs to be calculated.
 *
 * TODO: need fast algorithm to determine the binary matrix rand.
 *
 * Question: should number of rows and columns be equal (M == N) ?
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>

#include <tras.h>
#include <bmatrix.h>

int
bmatrix_init(struct tras_ctx *ctx, void *params)
{

	if (ctx == NULL || params != NULL)
		return (EINVAL);

	tras_ctx_init(ctx);

	ctx->algo = &bmatrix_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
bmatrix_update(struct tras_ctx *ctx, void *data, unsigned int bits)
{

	/* todo: */
	return (0);
}

int
bmatrix_final(struct tras_ctx *ctx)
{

	return (0);
}

int
bmatrix_test(struct tras_ctx *ctx, void *data, unsigned int bits)
{
	int error;

	error = bmatrix_update(ctx, data, bits);
	if (error != 0)
		return (error);

	error = bmatrix_final(ctx);
	if (error != 0)
		return (error);

	return (0);
}

int
bmatrix_restart(struct tras_ctx *ctx, void *params)
{

	return (0);
}

int
bmatrix_free(struct tras_ctx *ctx)
{

	return (0);
}

const struct tras_algo bmatrix_algo = {
	.name =		"BMatrix",
	.desc =		"Binary Matrix Rank Test",
	.version = 	{ 0, 1, 1 },
	.init =		bmatrix_init,
	.update =	bmatrix_update,
	.test =		bmatrix_test,
	.final =	bmatrix_final,
	.restart =	bmatrix_restart,
	.free =		bmatrix_free,
};
