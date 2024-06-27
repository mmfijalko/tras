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
#include <math.h>

#include <tras.h>
#include <cdefs.h>
#include <bmatrix.h>
#include <bmrank.h>

#ifdef TRAS_DEBUG

static void
binary_matrix32_show(uint32_t *bmatrix, unsigned int m, unsigned int n)
{
	unsigned int i, j;
	uint32_t row, mask;

	for (i = 0; i < m; i++) {
		row = bmatrix[i];
		mask = 0x80000000;
		for (j = 0; j < n; j++) {
			printf("%c ", (row & mask) ? '1' : '0');
			mask = mask >> 1;
		}
		printf("\n");
	}
}

#endif /* TRAS_DEBUG */

struct bmrank_ctx {
	unsigned int	nmatx;	/* number of matrices processed */
	unsigned int	fm0;	/* number of full rank matrices */
	unsigned int	fm1;	/* number of full rank-1 matrices */
	unsigned int	m;	/* number of rows in matrix */
	unsigned int	q;	/* number of columns in matrix */
	unsigned int	mq;	/* number of bits per matrix */
	unsigned int	N;	/* number of matrices to process */
	uint32_t *	bmtx;	/* the binary matrix from input */
	double *	rprob;	/* the probabilities of ranks */
	unsigned int *	rfreq;	/* the ranks frequencies */
	unsigned int	nr;	/* */
	unsigned int	s0;	/* */
	int		uniform;/* */
	unsigned int	nbits;	/* number of bits processed */
	double		alpha;	/* significance level for H0 */
};

/*
 * Generate the list of probabilities for binary matrices ranks
 * from r = 0 up to r = m, where m = min(M, Q). The size of the
 * matrices are M x Q.
 */
static void
bmrank_rank_probs(double *p, unsigned int m, unsigned int q)
{
        int i, j, r;
        double pr, ci;

        r = (int)min(m, q); 

        for (j = 0; j <= r; j++) {
		/* constant index */
		ci = (double)(j * ((int)m + (int)q - j) - (int)(m * q));
                pr = pow(2.0, ci);
                for (i = 0; i < (int)j; i++) {
			pr = pr * (1.0 - pow(2.0, i - (int)q));
			pr = pr * (1.0 - pow(2.0, i - (int)m));
                        pr = pr / (1.0 - pow(2.0, i - j));
                }
                p[j] = pr;
        }
}

static int
bmrank_check_params(struct tras_ctx *ctx, struct bmrank_params *p)
{

	TRAS_CHECK_PARA(p, p->alpha);

	if (p->m < BMRANK_MIN_M || p->q < BMRANK_MIN_Q)
		return (EINVAL);
	if (p->m > BMRANK_MAX_M || p->q > BMRANK_MAX_Q)
		return (EINVAL);
	if (p->s0 + p->q > BMRANK_MAX_Q)
		return (EINVAL);
	if (p->nr > min(p->m, p->q))
		return (EINVAL);
	if (p->N == 0)
		return (EINVAL);

	return (0);
}

int
bmrank_init(struct tras_ctx *ctx, void *params)
{
	struct bmrank_params *p = params;
	struct bmrank_ctx *c;
	unsigned int maxr;
	int size, error;

	TRAS_CHECK_INIT(ctx);

	error = bmrank_check_params(ctx, p);
	if (error != 0)
		return (error);

	maxr = min(p->m, p->q);

	size = sizeof(struct bmrank_ctx);
	size += p->m * sizeof(uint32_t);
	size += (maxr + 1) * sizeof(double);
	size += (maxr + 1) * sizeof(unsigned int);

	error = tras_init_context(ctx, &bmrank_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (0);
	c = ctx->context;

	c->bmtx = (uint32_t *)(c + 1);
	c->rprob = (double *)(c->bmtx + p->m);
	c->rfreq = (unsigned int *)(c->rprob + maxr + 1);

	c->m = p->m;
	c->q = p->q;
	c->mq = p->m * p->q;
	c->N = p->N;
	c->s0 = p->s0;
	c->nr = p->nr;
	c->uniform = p->uniform;

	c->alpha = p->alpha;

	return (0);
}

#define	__ISBIT(p, i)	((p)[(i) / 8] & (1 << ((i) & 0x07)))

static int
bmrank_update_bybits(struct bmrank_ctx *c, void *data, unsigned int nbits)
{
	unsigned int n, i, offs, r, k, rank;
	uint32_t wmask;
	uint8_t *p;

	p = (uint8_t *)data;

	n = c->nbits % c->mq;
	r = n / c->q;
	k = n % c->q;

	wmask = 0x00000001 << (31 - k);

	for (i = 0; i < nbits; i++) {
		c->bmtx[r] &= ~wmask;
		c->bmtx[r] |= __ISBIT(p, i) ? wmask : 0;
		k++;
		if (k < c->q) {
			wmask = wmask >> 1;
			continue;
		}

		/* Full row at least */
		k = 0;
		r = r + 1;
		wmask = 0x80000000;

		/* Full matrix */
		if (r < c->m) {
			wmask = 0x80000000;
			continue;
		}
		rank = binary_matrix_rank(c->bmtx, c->m, c->q);
		if (rank == min(c->m, c->q))
			c->fm0++;
		else if (rank == (min(c->m, c->q) - 1))
			c->fm1++;
		c->nmatx++;
		r = 0;
	}
	c->nbits += nbits;

	return (0);
}

static int
bmrank_update_bybits2(struct bmrank_ctx *c, uint8_t *p, unsigned int nbits)
{
	unsigned int i, j, k, n, r;
	uint32_t mask;

	/* Get the current row and column in the partial matrix */
	n = c->nbits % c->mq;
	r = n / c->q;
	k = n % c->q;

	/* How many bits can the loop below update */
	n = min(c->N * c->mq, c->nbits);
	n = c->N * c->mq - n;
	n = min(n, nbits);

	/* Set initial mask for current column */
	mask = 0x80000000 >> k;

	j = k;
	i = 0;
	while (n > 0) {
		/* At most k bits for the current matrix */
		k = (c->m - r) * c->q - k;
		k = min(k, n);
	
		/* Fill the matrix with k bits starting from i-th bit */
		for (k = i + k; i < k; i++) {
			c->bmtx[r] |= __ISBIT(p, i) ? mask : 0;
			if (++j >= c->q) {
				mask = 0x80000000;
				r++;
				j = 0;
			}
		}
		/* Calculate the full binary matrix rank and store it */
		if (r >= c->m) {
			r = binary_matrix_rank(c->bmtx, c->m, c->q);
			c->rfreq[r];
			c->nmatx++;
		}
		/* If the matrix is not fully filled the loop will end anyway */
		n = n - k;
		r = k = j = 0;
	}
	c->nbits += nbits;

	return (0);
}

static int
bmrank_update_byword(struct bmrank_ctx *c, uint8_t *p, unsigned int nbits)
{

	return (ENOSYS);
}

int
bmrank_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct bmrank_ctx *c;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;

	if (c->uniform)
		return (bmrank_update_byword(ctx->context, data, nbits));
	else
		return (bmrank_update_bybits(ctx->context, data, nbits));
}

int
bmrank_final(struct tras_ctx *ctx)
{
	struct bmrank_ctx *c;
	double expt, chi2, pvalue;
	unsigned int m, fmn;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nmatx < BMRANK_MIN_MATRICES)
		return (EALREADY);

	/*
	 * Generate the probabilities table for the matrices ranks.
	 */
	bmrank_rank_probs(c->rprob, c->m, c->q);

	/*
	 * Calculate chi-square distribution statistics.
	 */
	m = min(c->m, c->q);
	c->rprob[m - 2] = 1.0 - c->rprob[m - 1] - c->rprob[m];

	fmn = c->nmatx - c->fm0 - c->fm1;
	expt = c->rprob[m - 2] * c->nmatx;
	chi2 = ((double)fmn - expt) * ((double)fmn - expt) / expt;

	expt = c->rprob[m - 1] * c->nmatx;
	chi2 += ((double)c->fm1 - expt) * ((double)c->fm1 - expt) / expt;

	expt = c->rprob[m] * c->nmatx;
	chi2 += ((double)c->fm0 - expt) * ((double)c->fm0 - expt) / expt;

	/*
	 * Since p-value = igamc(1, chi2(obs) / 2) it is equal:
	 * pvalue = e ^ (-chi2(obs) / 2);
	 */
	pvalue = exp(-chi2 / 2.0);

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits % c->mq;
	ctx->result.stats1 = chi2;
	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

	return (0);
}

int
bmrank_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
bmrank_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
bmrank_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo bmrank_algo = {
	.name =		"BMatrix",
	.desc =		"Binary Matrix Rank Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		bmrank_init,
	.update =	bmrank_update,
	.test =		bmrank_test,
	.final =	bmrank_final,
	.restart =	bmrank_restart,
	.free =		bmrank_free,
};
