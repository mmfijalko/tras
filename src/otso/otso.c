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
 * The Overlapping-Triples-Sparse-Occupancy test (OTSO).
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
#include <otso.h>

	#include <stdio.h>

/*
 * The OTSO test parameters encoded as sparse parameters.
 */
static const struct sparse_params sparse_otso_params = {
	.m = 64,
	.k = 3,
	.b = 6,
	.r = 32,
	.wmax = SPARSE_MAX_WORDS,
//	.mean = 87.9395,
	.mean = 87.85,
	.var = 9.37,
};

static inline int
otso_set_params(struct sparse_params *sp, void *params)
{

	return (sparse_set_params(sp, &sparse_otso_params, params));
}

int
otso_init(struct tras_ctx *ctx, void *params)
{
	struct sparse_params sp;
	int error;

	error = otso_set_params(&sp, params);
	if (error != 0)
		return (error);

	return (sparse_init(ctx, &sp));
}

int
otso_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (sparse_update(ctx, data, nbits));
}

int
otso_final(struct tras_ctx *ctx)
{

	return (sparse_final(ctx));
}

int
otso_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (sparse_test(ctx, data, nbits));
}

int
otso_restart(struct tras_ctx *ctx, void *params)
{

	return (sparse_generic_restart(ctx, &sparse_otso_params, params));
}

int
otso_free(struct tras_ctx *ctx)
{

	return (sparse_free(ctx));
}

const struct tras_algo otso_algo = {
	.name =		"otso",
	.desc =		"Overlapping-Triples-Sparse-Occupancy Test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		otso_init,
	.update =	otso_update,
	.test =		otso_test,
	.final =	otso_final,
	.restart =	otso_restart,
	.free =		otso_free,
};
