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
#include <c1tsbits.h>

/*
 * Bytes to letters map.
 */
static uint8_t b2lmap[256] = {
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 2,
	0, 0, 0, 1, 0, 1, 1, 2, 0, 1, 1, 2, 1, 2, 2, 3,
	0, 0, 0, 1, 0, 1, 1, 2, 0, 1, 1, 2, 1, 2, 2, 3,
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	0, 0, 0, 1, 0, 1, 1, 2, 0, 1, 1, 2, 1, 2, 2, 3,
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 4,
	0, 0, 0, 1, 0, 1, 1, 2, 0, 1, 1, 2, 1, 2, 2, 3,
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 4,
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 4,
	2, 3, 3, 4, 3, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4,
};

/*
 * Letters probabilities.
 */
static double lprob[5] = {
	0.14453125,
	0.21875000,
	0.27343750,
	0.21875000,
	0.14453125,
};

/*
 * The context for the Count-the-1's Test (Stream of Bits)
 */
struct c1tsbits_ctx {
	unsigned int	nbits;	/* number of bits processed */
	uint8_t		last;	/* bits left from previous update */
	unsigned int	freq[5];/* frequencies for nums of 1's */
	double		alpha;	/* significance level for H0 */
};

int
c1tsbits_init(struct tras_ctx *ctx, void *params)
{
	struct c1tsbits_params *p = params;
	struct c1tsbits_ctx *c;
	int i;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	c = malloc(sizeof(struct c1tsbits_ctx));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}

	c->nbits = 0;
	c->last = 0;
	for (i = 0; i < 5; i++)
		c->freq[i] = 0;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &c1tsbits_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

inline static void
c1tsbits_update_freq(struct c1tsbits_ctx *c, uint8_t ones)
{

	if (ones < 3)
		c->freq[0]++;
	else if (ones > 5)
		c->freq[4]++;
	else
		c->freq[ones - 2]++;
}

#define	__EXTRACT_BYTE_0(p, o)	\
	(((p)[(o) >> 3] << ((o) & 0x07)) & ~lmask8[(o) & 0x07])

#define	__EXTRACT_BYTE_1(p, o)	\
	(((o) & 0x07) ?		\
	(((p)[((o) >> 3) + 1] >> (8-((o) & 0x07))) & lmask8[8-((o) & 0x07)]) \
	: 0)

#define	__EXTRACT_BYTE(p, o)	\
	__EXTRACT_BYTE_0(p, o) | __EXTRACT_BYTE_1(p, o)

static inline uint8_t
c1tsbits_extract_byte(uint8_t *p, unsigned int offs)
{
	uint16_t u16;
	unsigned int n;

	n = offs >> 3;

	if ((offs & 0x07) == 0)
		return (p[n]);

	u16 = (((uint16_t)p[n]) << 8) & 0xff00;
       	u16 |= (uint16_t)p[n + 1] & 0x00ff;
	u16 = (u16 >> (8 - (offs & 0x07)));

	return ((uint8_t)(u16 & 0x00ff));
}

int
c1tsbits_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct c1tsbits_ctx *c;
	uint8_t *p, h, b;
	unsigned int i, n, r, offs;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;
	p = (uint8_t *)data;

	n = nbits >> 3;
	r = c->nbits & 0x07;

	if (r > 0) {
		if (nbits < (8 - r)) {
			c->last |= (*p >> r) & lmask8[8 - r];
			c->last &= mmask8[nbits + r];
		} else {
			offs = 8 - r;
			c->last |= (*p >> r) & lmask8[offs];
			h = hamming8[c->last];
			c1tsbits_update_freq(c, h);
			c->last = 0;
			n = nbits - offs;
			r = n & 0x07;
			n = n >> 3;
			for (i = 0; i < n; i++, offs += 8) {
				b = __EXTRACT_BYTE(p, offs);
				h = hamming8[b];
				c1tsbits_update_freq(c, h);
			}
			if (r > 0) {
				n = nbits >> 3;
				c->last = p[n - 1] & mmask8[r];
			}
		}
	} else {
		for (i = 0; i < n; i++, p++) {
			h = hamming8[*p];
			c1tsbits_update_freq(c, h);
		}
		r = nbits & 0x07;
		if (r > 0)
			c->last = *p & mmask8[r];
	}

	/* todo: implementation */

	c->nbits += nbits;

	return (0);
}

int
c1tsbits_final(struct tras_ctx *ctx)
{
	struct c1tsbits_ctx *c;
	double pvalue, sobs;
	int sum;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	(void)c;

	/* todo: implementation */

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
c1tsbits_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
c1tsbits_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
c1tsbits_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo c1tsbits_algo = {
	.name =		"c1tsbits",
	.desc =		"Count-the-1's Test (Stream of Bits)",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		c1tsbits_init,
	.update =	c1tsbits_update,
	.test =		c1tsbits_test,
	.final =	c1tsbits_final,
	.restart =	c1tsbits_restart,
	.free =		c1tsbits_free,
};
