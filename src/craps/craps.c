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
 * The craps test.
 *
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

	#include <stdio.h>

/*
 * The context for the Craps Test.
 */
struct craps_ctx {
	unsigned int	freq[21];/* throws to win frequencies */
	unsigned int	K;	/* number of repetition the game */
	unsigned int	thrs;	/* number of throws */
	unsigned int	wins;	/* number of wins */
	unsigned int 	games;	/* number of games */
	int		next;	/* game not finished */
	unsigned int	toss;	/* last toss dice sum */
	double		alpha1;	/* significance level for wins test */
	double		alpha2;	/* significance level for throws freq test */
	unsigned int	throws;	/* security, maximum number of throws */
	unsigned int	nbits;	/* number of bits updated */
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

	TRAS_CHECK_INIT(ctx);

	TRAS_CHECK_PARA(p, p->alpha1);
	TRAS_CHECK_PARA(p, p->alpha2);

	c = malloc(sizeof(struct craps_ctx));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}

	for (i = 0; i < 21; i++)
		c->freq[i] = 0;

	c->K = p->K;
	c->thrs = 0;
	c->wins = 0;
	c->next = 0;
	c->toss = 0;
	c->games = 0;
	c->throws = p->throws;

	c->nbits = 0;
	c->alpha1 = p->alpha1;
	c->alpha2 = p->alpha2;

	ctx->context = c;
	ctx->algo = &craps_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

#define	offs_to32(s, o)	(((uint32_t *)(s))[(o) >> 5])

#define	seq_get32(s, o)	((offs_to32(s, o) << ((o) & 0x1f)) |	\
	(((o) & 0x1f) ? offs_to32(s, (o) + 32) >> (32 - ((o) & 0x1f)) : 0))

static unsigned int
craps_toss(void *data, unsigned int offs)
{
	unsigned int dice;
	uint32_t die1, die2;
	double u01;

	if (offs & 0x1f) {
		die1 = seq_get32(data, offs);
		die2 = seq_get32(data, offs + 32);
	} else {
		die1 = offs_to32(data, offs);
		die2 = offs_to32(data, offs + 32);
	}

	u01 = craps_uniform01(die1);
	dice = (unsigned int)(u01 * 6);
	u01 = craps_uniform01(die2);
	dice += (unsigned int)(u01 * 6);

	return (dice);
}

int
craps_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct craps_ctx *c;
	unsigned int thrs, dice, next, toss;
	unsigned int r, i, n, offs, throws;
	double u01;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;

	if (c->games >= c->K) {
		c->nbits += nbits;
		return (0);
	}

	/*
	 * Support only updates with multiple of 64 bits.
	 */
	r = c->nbits & 0x3f;
	if (r > 0 || (nbits & 0x3f))
		return (ENOSYS);

	/*
	 * We use numbers of spots on the face of a die from 0 to 5 to
	 * avoid adding one to random number. So, the result of of a toss
	 * of two dice can vary from 0 to 10. (5 and 9 are for win;
	 * 0, 1, 10 are for loss; continue when 5).
	 */
	throws = c->nbits >> 6;

	n = nbits >> 6;
	thrs = c->thrs;
	next = c->next;
	toss = c->toss;
	for (i = 0, offs = 0; i < n; i++, offs += 64) {
		if (c->games >= c->K)
			break;
		if (throws >= c->throws)
			break;
		dice = craps_toss(data, offs);
		throws++;
		if (next) {
			thrs = (thrs < 20) ? ++thrs : 20;
			if (dice != 5 && (dice != toss))
				continue;
			c->freq[thrs]++;
			if (dice == toss)
				c->wins++;
		} else if (dice == 5 || dice == 9) {
			c->freq[0]++;
			c->wins++;
		} else if (dice == 0 || dice == 1 || dice == 10) {
			c->freq[0]++;
		} else {
			next = 1;
			toss = dice;
			continue;
		}
		next = thrs = 0;
		c->games++;
	}
	c->thrs = thrs;
	c->next = next;
	c->toss = toss;

	c->nbits += nbits;

	return (0);
}

int
craps_final(struct tras_ctx *ctx)
{
	struct craps_ctx *c;
	double pvalue1, pvalue2, sobs;
	double mean, stdev, p, s;
	int i, sum;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->games < CRAPS_MIN_GAMES)
		return (EALREADY);
	if ((c->nbits >> 6) >= c->throws)
		return (ENXIO);

printf("%s: games = %u, wins = %u, ratio = %g, finished %s\n", __func__, c->games, c->wins,
    (double)c->wins / (double)c->games,   c->next ? "yes" : "no");

	p = 244.0 / 495.0;
	mean = p * c->K;
	stdev = mean * (1.0 - p);

	s = ((double)c->wins - mean);
	s = fabs(s) / sqrt((double)2.0) / stdev;
	pvalue1 = erfc(s);
	/* TODO: temporary */
	pvalue2 = pvalue1;

	if (pvalue1 < c->alpha1 || pvalue2 < c->alpha2)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits & 0x07;
	ctx->result.stats1 = s;
	ctx->result.stats2 = (double)c->wins;
	ctx->result.pvalue1 = pvalue1;
	ctx->result.pvalue2 = pvalue2;

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
