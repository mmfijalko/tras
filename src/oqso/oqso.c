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
 * The overlapping-quadruples-sparse-occupancy test (OQSO).
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
#include <sparse.h>
#include <oqso.h>

/*
 * The OQSO test parameters encoded as sparse parameters.
 */
struct sparse_params sparse_oqso_params = {
	.m = 32,
	.k = 4,
	.b = 5,
	.r = 32,
	.wmax = SPARSE_MAX_WORDS,
	.mean = 141909.6005321316,
	.var = 294.6558723658,
};

static inline int
oqso_set_params(struct sparse_params *sp, void *params)
{

	return (sparse_set_params(sp, &sparse_oqso_params, params));
}

int
oqso_init(struct tras_ctx *ctx, void *params)
{
	struct sparse_params sp;
	int error;

	error = oqso_set_params(&sp, params);
	if (error != 0)
		return (error);

	return (sparse_init(ctx, &sp));
}

int
oqso_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (sparse_update(ctx, data, nbits));
}

int
oqso_final(struct tras_ctx *ctx)
{

	return (sparse_final(ctx));
}

int
oqso_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (sparse_test(ctx, data, nbits));
}

int
oqso_restart(struct tras_ctx *ctx, void *params)
{
	return (sparse_generic_restart(ctx, &sparse_oqso_params, params));
}

int
oqso_free(struct tras_ctx *ctx)
{

	return (sparse_free(ctx));
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
