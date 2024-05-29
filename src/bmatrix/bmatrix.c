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
#include <stdlib.h>

#include <tras.h>
#include <bmatrix.h>

struct bmatrix_ctx {
	unsigned int	nbits;	/* number of bits processed */
	unsigned int	nmatx;	/* number of matrices processed */
	unsigned int	fm0;	/* number of full rank matrices */
	unsigned int	fm1;	/* number of full rank-1 matrices */
	unsigned int	m;	/* number of rows in matrix */
	unsigned int	q;	/* number of columns in matrix */
	unsigned int	mq;	/* number of bits per matrix */
	double		alpha;	/* significance level for H0 */
};

int
bmatrix_init(struct tras_ctx *ctx, void *params)
{
	struct bmatrix_ctx *c;
	struct bmatrix_params *p = params;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->m < BMATRIX_MIN_M || p->q < BMATRIX_MIN_Q)
		return (EINVAL);
	if (p->m > BMATRIX_MAX_M || p->q > BMATRIX_MAX_Q)
		return (EINVAL);

	c = malloc(sizeof(struct bmatrix_ctx));
	if (c == NULL)
		return (ENOMEM);

	c->nbits = 0;
	c->nmatx = 0;
	c->fm0 = 0;
	c->fm1 = 0;
	c->alpha = p->alpha;
	c->m = p->m;
	c->q = p->q;
	c->mq = c->m * c->q;

	ctx->context = c;
	ctx->algo = &bmatrix_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
bmatrix_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	/*
	 * todo: implementation.
	 */
	return (ENOSYS);
}

int
bmatrix_final(struct tras_ctx *ctx)
{
	struct bmatrix_ctx *c;
	unsigned int n;
	double diffn, expt0, expt1, exptn;
	double fmn, chi2, pvalue;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nmatx < BMATRIX_MIN_MATRICES)
		return (EALREADY);

	/* Calculate chi-square distribution statistics */
	n = c->nmatx;

	expt0 = 0.2888 * c->nmatx;
	expt1 = 0.5776 * c->nmatx;
	exptn = 0.1336 * c->nmatx;
	diffn = (double)(n - c->fm0 + c->fm1);

	chi2 = (c->fm0 - expt0) * (c->fm0 - expt0) / expt0 +
	    (c->fm1 - expt1) * (c->fm1 - expt1) / expt1 +
	    (diffn - exptn) * (diffn - exptn) / exptn;

	/* todo: here calculation of statistics */
	pvalue = 0.0;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits % c->mq;
	ctx->result.stats1 = chi2;
	ctx->result.stats2 = 0.0;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
bmatrix_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
bmatrix_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
bmatrix_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo bmatrix_algo = {
	.name =		"BMatrix",
	.desc =		"Binary Matrix Rank Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		bmatrix_init,
	.update =	bmatrix_update,
	.test =		bmatrix_test,
	.final =	bmatrix_final,
	.restart =	bmatrix_restart,
	.free =		bmatrix_free,
};
