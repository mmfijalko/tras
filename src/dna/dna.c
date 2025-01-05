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
 * The DNA Test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tras.h>
#include <utils.h>
#include <cdefs.h>
#include <bits.h>

#include <sparse.h>
#include <dna.h>

/*
 * The DNA test parameters encoded as sparse parameters.
 */
static const struct sparse_params sparse_dna_params = {
	.m = 4,
	.k = 10,
	.b = 2,
	.r = 32,
	.wmax = SPARSE_MAX_WORDS,
	.mean = 141910.4026047629,
	.var = 337,0,
};

int
dna_init(struct tras_ctx *ctx, void *params)
{
	struct sparse_params sp;
	int error;

	error = sparse_set_params(&sp, &sparse_dna_params, params);
	if (error != 0)
		return (error);

	return (sparse_init(ctx, &sp));
}

int
dna_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (sparse_update(ctx, data, nbits));
}

int
dna_final(struct tras_ctx *ctx)
{

	return (sparse_final(ctx));
}

int
dna_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (sparse_test(ctx, data, nbits));
}

int
dna_restart(struct tras_ctx *ctx, void *params)
{

	return (sparse_generic_restart(ctx, &sparse_dna_params, params));
}

int
dna_free(struct tras_ctx *ctx)
{

	return (sparse_free(ctx));
}

const struct tras_algo dna_algo = {
	.name =		"dna",
	.desc =		"Four Letters C,G,A,T words test using sparse.",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		dna_init,
	.update =	dna_update,
	.test =		dna_test,
	.final =	dna_final,
	.restart =	dna_restart,
	.free =		dna_free,
};
