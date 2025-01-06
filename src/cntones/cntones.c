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
 * TODO: name of the test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include <tras.h>
#include <utils.h>
#include <cdefs.h>
#include <bits.h>
#include <c1tsbits.h>

	#include <stdio.h>

/*
 * The mapping from bytes to letter through their Hamming weight.
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
 * The helper mapping from two letters word to index map.
 */
static uint8_t w2imap[40] = {
	 0,  1,  2,  3,  4,  0,  0,  0, 
	 5,  6,  7,  8,  9,  0,  0,  0, 
	10, 11, 12, 13, 14,  0,  0,  0, 
	15, 16, 17, 18, 19,  0,  0,  0, 
	20, 21, 22, 23, 24,  0,  0,  0, 
};

/*
 * The probabilities of letters in the monkey test.
 */
static double lprob[5] = {
	0.14453125,		/* [0]: {0, 1, 2} -> A */
	0.21875000,		/* [1]: {3} -> B */
	0.27343750,		/* [2]: {4} -> C */
	0.21875000,		/* [3]: {5} -> D */
	0.14453125,		/* [4]: {6, 7, 8} -> E */
};

/*
 * The V1 - V2 statistics is asymptotically normal with the below parameters.
 */
#define	C1TSBITS_MEAN		2500

#define	C1TSBITS_STDDEV		70.71

/*
 * XXX: temporary definition here to move to an include file.
 */
#define miss(c, cmax)   (((c) < (cmax)) ? (cmax) - (c) : 0)
#define	min(a, b)	(((a) < (b)) ? (a) : (b))

/*
 * The context for the generic count-the-1's test.
 */
struct cntones_ctx {
	uint8_t		last;	/* bits left from previous update */
	uint32_t	word;	/* last word colected from updates */
	unsigned int *	w4freq;	/* four letter words frequencies */
	unsigned int *	w5freq;	/* five letter words frequencies */
	int		algo;	/* algo type and type of byte selection */
	unsigned int	sbit;	/* selected start bit from random word */
	unsigned int	nbits;	/* number of bits processed */
	double		alpha;	/* significance level for H0 */
};

int
cntones_init(struct tras_ctx *ctx, void *params)
{
	struct cntones_params *p = params;
	struct cntones_ctx *c;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->algo != CNTONES_ALGO_BITSTREAM &&
	    p->algo != CNTONES_ALGO_SELBYTES)
		return (EINVAL);
	if (p->sbit > 23)
		return (EINVAL);

	size = sizeof(struct cntones_ctx) + 625 * sizeof(unsigned int) +
	    3125 * sizeof(unsigned int);

	error = tras_init_context(ctx, &cntones_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);
	c = ctx->context;

	c->w4freq = (unsigned int *)(c + 1);
	c->w5freq = (unsigned int *)(c->w4freq + 625);
	c->alpha = p->alpha;

	return (0);
}

#define	__EXTRACT_BYTE_0(p, o)	\
	(((p)[(o) >> 3] << ((o) & 0x07)) & ~lmask8[(o) & 0x07])

#define	__EXTRACT_BYTE_1(p, o)	\
	(((o) & 0x07) ?		\
	(((p)[((o) >> 3) + 1] >> (8-((o) & 0x07))) & lmask8[8-((o) & 0x07)]) \
	: 0)

#define	__EXTRACT_BYTE(p, o)	\
	__EXTRACT_BYTE_0(p, o) | __EXTRACT_BYTE_1(p, o)

#ifdef __not_yet__
#define	__EXTRACT_LETTER(p, o)	\
{}
#endif

static inline uint8_t
cntones_extract_byte(uint8_t *p, unsigned int offs)
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

#ifdef notyet

int
cntones_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct c1tsbits_ctx *c;
	uint8_t *p, h, b;
	unsigned int i, n, r, offs;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;
	p = (uint8_t *)data;

	r = c->nbits & 0x07;

	if (nbits > 0) {
		offs = 8 - r;
		if (r > 0) {
			c->last |= (*p >> r) & lmask8[offs];
			if (nbits < offs) {
				c->last &= mmask8[nbits + r];
			} else {
				h = hamming8[c->last];
				c1tsbits_update_freq(c, h);
				c->last = 0;
				n = nbits - offs;
				r = n & 0x07;
				n = n >> 3;
				for (i = 0; i < n; i++, p++) {
					b = __EXTRACT_BYTE(p, offs);
					h = hamming8[b];
					c1tsbits_update_freq(c, h);
				}
			}
		} else {
			n = nbits >> 3;
			for (i = 0; i < n; i++, p++) {
				h = hamming8[*p];
				c1tsbits_update_freq(c, h);
			}
			r = nbits & 0x07;
		}
		if (r > 0)
			c->last = *p & mmask8[r];
		break;
	}
	c->nbits += nbits;

	return (0);
}
#endif

#define	C1TSBITS_WORDMASK	0x00007fff

	#include <stdio.h>

int
cntones_update_bitstream(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct cntones_ctx *c;
	unsigned int j, n, id4, id5;
	uint32_t word;
	uint8_t *p;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	if (nbits & 0x07)
		return (EINVAL);

	c = ctx->context;
	p = (uint8_t *)data;

	j = c->nbits >> 3;
	n = miss(j, C1TSBITS_BYTES);
	n = min(n, (nbits >> 3));

	word = c->word;

	/* Get first four letters */
	while (j < 4 && n > 0) {
		word = (word << 3) | b2lmap[*p++];
		n--; j++;
	}
	/* Iterate with five and four letters words */
	while (n > 0) {
		word = ((word << 3) | b2lmap[*p++]) & C1TSBITS_WORDMASK;
		/* 6 bits map is used to calculate four letter word position */
		id4 = 25 * w2imap[(word >> 6) & 0x3f] + w2imap[word & 0x3f];
		/* get five letters word position */
		id5 = 625 * ((word >> 12) & 0x07) + id4;
		/* update frequencies */
		c->w4freq[id4]++;
		c->w5freq[id5]++;
		n--;
	}
	c->word = word;
	c->nbits += nbits;

	return (0);
}

int
cntones_final(struct tras_ctx *ctx)
{
	struct cntones_ctx *c;
	double pvalue, s, d, v2, v1, *exp;
	int i, j, w, l;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nbits < C1TSBITS_MIN_NBITS)
		return (EALREADY);

	/*
	 * XXX: when the chi-square algo implementation is completed
	 * we will use it to get the results - pvalue, result and stats.
	 */

	exp = malloc(3125 * sizeof(double));
	if (exp == NULL)
		return (ENOMEM);

	/* Get expected value for five letter words */
	for (i = 0; i < 3125; i++) {
		exp[i] = C1TSBITS_WORDS;
		w = i;
		for (j = 0; j < 5; j++) {
			l = w % 5;
			exp[i] = exp[i] * lprob[l];
			w = w / 5;
		}
	}
	for (i = 0, v2 = 0.0; i < 3125; i++) {
		d = (double)c->w5freq[i]  - exp[i];
		v2 += d * d / exp[i];
	}

	/* Get expected value for four letter words */
	for (i = 0; i < 625; i++) {
		exp[i] = C1TSBITS_WORDS;
		w = i;
		for (j = 0; j < 4; j++) {
			l = w % 5;
			exp[i] = exp[i] * lprob[l];
			w = w / 5;
		}
	}
	for (i = 0, v1 = 0.0; i < 625; i++) {
		d = (double)c->w4freq[i] - exp[i];
		v1 += d * d / exp[i];
	}

	s = fabs(v2 - v1 - CNTONES_MEAN) / CNTONES_STDDEV;
	s = s / sqrt(2.0);
	pvalue = erfc(fabs(s));

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

#ifdef notyet
	if (c->algo == CNTONES_ALGO_BITSTREAM)
		ctx->result.discard = c->nbits - C1TSBITS_MIN_NBITS;
	else if (c->algo == CNTONES_ALGO_SELBYTES)
		ctx->result.discard = c->nbits - C1TSBYTE_MIN_NBITS;
#endif

	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

	return (0);
}

int
cntones_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
cntones_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
cntones_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo cntones_algo = {
	.name =		"cntones",
	.desc =		"Generic Count-the-1's Test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		cntones_init,
	.update =	cntones_update8,
	.test =		cntones_test,
	.final =	cntones_final,
	.restart =	cntones_restart,
	.free =		cntones_free,
};
