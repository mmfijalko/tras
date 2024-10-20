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
#include <string.h>
#include <math.h>
#include <stddef.h>

#include <stdio.h>

#include <tras.h>
#include <cdefs.h>
#include <chi2.h>
#include <excursion.h>

	#include <stdio.h>

/*
 * Private context for random excursion test.
 */
struct excursion_ctx {
	int		state;	/* current excursion state */
	unsigned int	cycle;	/* current cycle number */
	double *	chi2o;	/* observed frequency table */
	unsigned int *	cfreq;	/* one cycle frequency table */
	unsigned int *	sfreq;	/* state/cycles frequency table	*/
	unsigned int	nbits;	/* number of bits updated */
	double		alpha;	/* significance level for H0 */
};

/*
 * The precalculated frequency table for states beeing exactly k times.
 */
static const double excursion_prob[8][6] = {
	0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000,
	0.50000000, 0.25000000, 0.12500000, 0.06250000, 0.03125000, 0.03125000,
	0.75000000, 0.06250000, 0.04687500, 0.03515625, 0.02636719, 0.07910156,
	0.83333333, 0.02777778, 0.02314815, 0.01929012, 0.01607510, 0.08037551,
	0.87500000, 0.01562500, 0.01367188, 0.01196289, 0.01046753, 0.07327271,
	0.90000000, 0.01000000, 0.00900000, 0.00810000, 0.00729000, 0.06561000,
	0.91666667, 0.00694444, 0.00636574, 0.00583526, 0.00534899, 0.05883890,
	0.92857143, 0.00510204, 0.00473761, 0.00439921, 0.00408498, 0.05310473,
};

/*
 * Map excursion state for frequency table row index.
 */
static int state_map[8] = {-4, -3, -2, -1, 1, 2, 3, 4};

int
excursion_init(struct tras_ctx *ctx, void *params)
{
	struct excursion_params *p = params;
	struct excursion_ctx *c;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	size = sizeof(struct excursion_ctx) + 8 * sizeof(unsigned int) +
	    8 * 6 * sizeof(unsigned int);

	error = tras_init_context(ctx, &excursion_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);
	c = ctx->context;

	c->cfreq = (unsigned int *)(c + 1);
	c->sfreq = (unsigned int *)(c->cfreq + 8);

	c->alpha = p->alpha;

	return (0);
}

static void
excursion_cycle_done(struct excursion_ctx *c)
{
	unsigned int j, k, *sfreq;

	/*
	 * Copy counters to state/cycles table.
	 */
	sfreq = c->sfreq;
	for (j = 0; j < 8; j++, sfreq += 6) {
		for (k = 0; k < 5; k++) {
			if (c->cfreq[j] == k)
				sfreq[k]++;
		}
		if (c->cfreq[j] >= 5)
			sfreq[5]++;
	}
	c->cycle++;

	/*
	 * Reset cycle frequencye table when before new one.
	 */
	memset(c->cfreq, 0, 8 * sizeof(unsigned int));
}

int
excursion_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct excursion_ctx *c;
	unsigned int i, n, m;
	uint8_t *p, mask;
	int j;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = (struct excursion_ctx *)ctx->context;
	p = data;
	n = nbits;

	while (n > 0) {
		m = min(n, 8);
		for (i = 0, mask = 0x80; i < m; i++, mask >>= 1) {
			 /* Up or down. */
			c->state += (*p & mask) ? 1 : -1;

			/* Run end of cycle routine or update state counter */
			if (c->state == 0) {
				excursion_cycle_done(c);
			} else if (c->state >= -4 && c->state <= 4) {
				if (c->state < 0)
					j = c->state + 4;
				else if (c->state > 0)
					j = c->state + 3;
				/* XXX: j could be undefined here */
				c->cfreq[j]++;
			}
		}
		p++;
		n = n - m;
	}
	c->nbits += nbits;

	return (0);
}

int
excursion_final(struct tras_ctx *ctx)
{
	struct excursion_ctx *c;
	double pvalue, pvmin, pvmax;
	struct chi2_params chi2p;
	struct tras_ctx chi2c;
	unsigned j, k, J = 0, fail;
	double exp[6], obs[6];
	int x, error;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nbits < EXCURSION_MIN_BITS)
		return (EALREADY);

	if (c->state != 0)
		excursion_cycle_done(c);

	printf("number of cycle, J = %u\n", c->cycle);
	for (j = 0; j < 8; j++) {
		J = 0;
		for (k = 0; k <=5; k++) {
			printf("%u\t", c->sfreq[j * 6 + k]);
			J += c->sfreq[j * 6 + k];
		}
		printf(" | J = %u\n", J);
	}

	printf("expected number of cycles, J = %u\n", c->cycle);
	for (j = 0; j < 8; j++) {
		x = abs(state_map[j]);
		for (k = 0; k <=5; k++) {
			printf("%u\t", (unsigned int)(c->cycle * excursion_prob[x][k]));
		}
		printf(" | J = %u\n", J);
	}

	J = sqrt(c->nbits) / 200;
	J = max(J, 500);

	if (c->cycle < J)
		printf("hypotesis rejected (%u)\n", J);
	else
		printf("hypotesis accepted (%u)\n", J);

	pvmin = 1.0;
	pvmax = 0.0;

	for (j = 0, fail = 0; j < 8; j++) {
		x = abs(state_map[j]);
		for (k = 0; k <= 5; k++)
			obs[k] = c->cycle * c->sfreq[j * 6 + k];
		for (k = 0; k <= 5; k++)
			exp[k] = c->cycle * excursion_prob[x][k];
		memset(&chi2c, 0, sizeof(chi2c));
		chi2p.K = 6;
		chi2p.df = 5;
		chi2p.exp = exp;
		chi2p.alpha = c->alpha;
		error = chi2_init(&chi2c, (void *)&chi2p);
		if (error != 0)
			return (error);
		error = chi2_test(&chi2c, obs, 6 * sizeof(double) * 8);
		if (error != 0) {
			chi2_free(&chi2c);
			return (error);
		}
		pvalue = chi2c.result.pvalue1;
		pvmin = min(pvalue, pvmin);
		pvmax = max(pvalue, pvmax);
		chi2_free(&chi2c);
	}

	if (pvmin < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = -1;
	ctx->result.pvalue1 = pvmin;
	ctx->result.pvalue2 = pvmax;

	tras_fini_context(ctx, 0);

	return (0);
}

int
excursion_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
excursion_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
excursion_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo excursion_algo = {
	.name =		"Excursion",
	.desc =		"Random Excursion Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		excursion_init,
	.update =	excursion_update,
	.test =		excursion_test,
	.final =	excursion_final,
	.restart =	excursion_restart,
	.free =		excursion_free,
};
