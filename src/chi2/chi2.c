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
 * The Pearson chi-square test.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <math.h>

#include <tras.h>
#include <chi2.h>

static double
chi2_statistics(unsigned int *v, double *p, unsigned int nv, unsigned int n)
{
	unsigned int i;
	double s, d;

	for (i = 0, s = 0.0; i < nv; i++) {
		d = v[i] - n * p[i];
		s = s + (d * d) / (n * p[i]);
	}
	return (s);
}

int
chi2_init(struct tras_ctx *ctx, void *params)
{
	struct chi2_params *p = params;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->K < 5)
		return (EINVAL);
	if (p->df < 4)
		return (EINVAL);
	if (p->exp == NULL)
		return (EINVAL);

	tras_init_context(ctx, &chi2_algo, 0, 0);

	ctx->context = p;

	return (0);
}

int
chi2_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct chi2_params *p;
	double *obs, s, d;
	unsigned int i;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	p = ctx->context;

	if (nbits != p->K * sizeof(double) * 8)
		return (EINVAL);

	obs = (double *)data;

	for (i = 0, s = 0.0; i < p->K; i++) {
		if (p->exp[i] == 0.0)
			return (ENXIO);
		d = obs[i] - p->exp[i];
		s = s + d * d / p->exp[i];
	}

	ctx->result.stats1 = s;

	return (0);
}

int
chi2_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
chi2_final(struct tras_ctx *ctx)
{
	struct chi2_params *p;

	TRAS_CHECK_FINAL(ctx);

	p = ctx->context;

	/* TODO:
	 * pvalue1 = igamc(p->df / 2.0, ctx->result.stats1 / 2.0);
	 */

	ctx->result.pvalue1 = 0.0;
	ctx->result.pvalue2 = 0.0;

	/* reset context to not free by tras_fini_context function */
	ctx->context = NULL;

	tras_fini_context(ctx, 0);

	return (0);
}

int
chi2_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
chi2_free(struct tras_ctx *ctx)
{

	/* Intentionally no free */
}

const struct tras_algo chi2_algo = {
	.name =		"TODO:",
	.desc =		"TODO:",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		chi2_init,
	.update =	chi2_update,
	.test =		chi2_test,
	.final =	chi2_final,
	.restart =	chi2_restart,
	.free =		chi2_free,
};
