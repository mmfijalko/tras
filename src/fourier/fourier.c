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
 * The Discrette Fourier Test (Spectral) is non parameter.
 *
 * TODO: Discrette Fourier Transform (DFT) algorithm.
 *
 * Minimum number of bits n >= 1000.
 *
 * No maximum number of bits.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <endian.h>
#include <math.h>

#include <tras.h>
#include <fourier.h>

/*
 * The Discrete Fourier Transform Test context.
 */
struct fourier_ctx {
	unsigned int	nbits;	/* the number of bits processed */
	double		alpha;	/* the significance level for H0 */
};

int
fourier_init(struct tras_ctx *ctx, void *params)
{
	struct fourier_params *p = params;
	struct fourier_ctx *c;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	error = tras_init_context(ctx, &fourier_algo,
	    sizeof(struct fourier_ctx), TRAS_F_ZERO);
	if (error != 0)
		return (error);

	c = ctx->context;

	c->alpha = p->alpha;

	return (0);
}

int
fourier_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct fourier_ctx *c;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;

	/*
	 * TODO: implementation.
	 */
	return (ENOSYS);
}

int
fourier_final(struct tras_ctx *ctx)
{
	struct fourier_ctx *c;
	unsigned int n;
	double t, n0, n1, d;
	double pvalue;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;
	if (c->nbits < FOURIER_MIN_BITS)
		return (EALREADY);

	/* todo: apply DFT to the sequence of bits; S=DFT(X) */

	/* todo: calculate modulus M = modulus(S') = |S'|
	 * S' is a substring of first n/2 elements in S */

	/* todo: compute T = sqrt(log(1/0.05) *n) */

	n = c->nbits;
	t = sqrt(log(1.0 / 0.05) * (double)n);
	d = (n1 - n0) / sqrt(0.95 * (double)n * 0.5 / 4);

	n0 = 0.95 * (double)n / 2.0;
	/* todo: compute n1 - number of peaks in M less than T */

	pvalue = erfc(abs(d) / sqrt((double)2.0));

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = 0;
	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

	return (0);
}

int
fourier_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
fourier_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
fourier_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo fourier_algo = {
	.name =		"Fourier",
	.desc =		"Discrete Fourier Transform (Spectral) Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		fourier_init,
	.update =	fourier_update,
	.test =		fourier_test,
	.final =	fourier_final,
	.restart =	fourier_restart,
	.free =		fourier_free,
};
