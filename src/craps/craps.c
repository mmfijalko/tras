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
 * The Count-the-1's Test (Stream of Bits)
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include <tras.h>
#include <hamming8.h>
#include <utils.h>
#include <bits.h>
#include <craps.h>

/*
 * The context for the Craps Test.
 */
struct craps_ctx {
	unsigned int	nbits;	/* number of bits processed */
	unsigned int	freq[21];/* throws to win frequencies */
	double		alpha;	/* significance level for H0 */
	unsigned int	K;	/* number of repetition the game */
	unsigned int	thrs;	/* number of throws */
	unsigned int	wins;	/* number of wins */
	unsigned int 	games;	/* number of games */
	int		next;	/* game not finished */
	unsigned int	toss;	/* last toss dice sum */
};

/*
 * Convert uniformly on interval [0, 1) 32-bits random value.
 */
static double
craps_uniform01(uint32_t r32)
{

	/*
	 * XXX: temporary procedure, simple, not uniformly disributing
	 * value r32 to double in [0, 1) interval. Some values are more
	 * likely than others
	 */

	return ((double)r32 / (double)pow(2.0, 32));
}

int
craps_init(struct tras_ctx *ctx, void *params)
{
	struct craps_ctx *c;
	struct craps_params *p = params;
	int i;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	c = malloc(sizeof(struct craps_ctx));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}

	c->nbits = 0;
	for (i = 0; i < 21; i++)
		c->freq[i] = 0;
	c->thrs = 0;
	c->wins = 0;
	c->next = 0;
	c->games = 0;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &craps_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
craps_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct craps_ctx *c;
	unsigned int r, i, n;
	double u01;
	unsigned int toss, thrs, dice;
	uint32_t *p32;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	if (c->games >= c->K) {
		c->nbits += nbits;
		return (0);
	}

	/* rest from previous update as two 32-bits numbers (two dices) */
	r = c->nbits & 0x3f;
	if (r > 0 || (nbits & 0x3f)) {
		/* Not supported scenario, yet */
		return (ENOSYS);
	}

	p32 = (uint32_t *)data;
	thrs = c->thrs;
	n = nbits >> 6;
	for (i = 0; i < n; i++) {
		u01 = craps_uniform01(p32[i / 2]);
		dice = (unsigned int)(u01 * 6);
		u01 = craps_uniform01(p32[i / 2 + 1]);
		dice += (unsigned int)(u01 * 6);
		if (c->next) {
			toss = c->toss;
			thrs = (thrs < 20) ? ++thrs : 20;
			if (dice == 5 || (dice == toss)) {
				c->next = 0;
				c->freq[thrs]++;
				c->games++;
				thrs = 0;
				if (dice == toss)
					c->wins++;
			}
		} else if (dice == 7 || dice == 9) {
			c->freq[thrs]++;
			c->next = 0;
			c->wins++;
		} else if (dice == 0 || dice == 1 || dice == 10) {
			c->freq[thrs]++;
			c->next = 0;
		} else {
			c->next = 1;
			c->toss = dice;
		}
	}

	c->nbits += nbits;

	return (0);
}

int
craps_final(struct tras_ctx *ctx)
{
	struct craps_ctx *c;
	double pvalue, sobs;
	double mean, var, p;
	int sum;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	(void)c;

	/* todo: implementation */

	p = 244.0 / 495.0;
	mean = p * c->K;
	var = p * (1.0 - p) * c->K;

	pvalue = 0.0;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits & 0x07;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
craps_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
craps_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
craps_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo craps_algo = {
	.name =		"craps",
	.desc =		"The Craps Test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		craps_init,
	.update =	craps_update,
	.test =		craps_test,
	.final =	craps_final,
	.restart =	craps_restart,
	.free =		craps_free,
};
