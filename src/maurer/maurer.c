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
#include <string.h>
#include <math.h>

#include <tras.h>
#include <cdefs.h>
#include <const.h>
#include <universal.h>
#include <maurer.h>

double
maurer_coef(struct universal_ctx *c)
{

	return (0.7 - 0.8 / c->L + (4.0 + 32.0 / c->L) * 
	    pow(c->K, -3.0 / c->L) / 15.0);
}

int
maurer_init(struct tras_ctx *ctx, void *params)
{
	struct universal_params p;

	TRAS_CHECK_PARAM(params);

	memcpy(&p, params, sizeof(struct universal_params));
	p.coeff = maurer_coef;

	return (universal_init_algo(ctx, &p, &universal_algo));
}

int
maurer_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (universal_update(ctx, data, nbits));
}

int
maurer_final(struct tras_ctx *ctx)
{

	return (universal_final(ctx));
}

int
maurer_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (universal_test(ctx, data, nbits));
}

int
maurer_restart(struct tras_ctx *ctx, void *params)
{

	return (universal_restart(ctx, params));
}

int
maurer_free(struct tras_ctx *ctx)
{

	return (universal_free(ctx));
}

const struct tras_algo maurer_algo = {
	.name =		"Universal",
	.desc =		"Maurer's Universal Statistical Test",
	.id =		UNIVERSAL_ID_MAURER,
	.version = 	{ 0, 1, 1 },
	.init =		maurer_init,
	.update =	maurer_update,
	.test =		maurer_test,
	.final =	maurer_final,
	.restart =	maurer_restart,
	.free =		maurer_free,
};
