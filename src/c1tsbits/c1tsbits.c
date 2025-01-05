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
 * Two letters word to index map.
 */
static uint8_t w2imap[40] = {
	 0,  1,  2,  3,  4,  0,  0,  0, 
	 5,  6,  7,  8,  9,  0,  0,  0, 
	10, 11, 12, 13, 14,  0,  0,  0, 
	15, 16, 17, 18, 19,  0,  0,  0, 
	20, 21, 22, 23, 24,  0,  0,  0, 
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
 * The context for the Count-the-1's Test (Stream of Bits)
 */
struct c1tsbits_ctx {
	unsigned int	nbits;	/* number of bits processed */
	uint8_t		last;	/* bits left from previous update */
	uint32_t	word;	/* last word colected from updates */
	unsigned int *	w4freq;	/* four letter words frequencies */
	unsigned int *	w5freq;	/* five letter words frequencies */
	double		alpha;	/* significance level for H0 */
};

int
c1tsbits_init(struct tras_ctx *ctx, void *params)
{
	struct c1tsbits_params *p = params;
	struct c1tsbits_ctx *c;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	size = sizeof(struct c1tsbits_ctx) + 625 * sizeof(unsigned int) +
	    3125 * sizeof(unsigned int);

	error = tras_init_context(ctx, &c1tsbits_algo, size, TRAS_F_ZERO);
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

#ifdef notyet

int
c1tsbits_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
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
c1tsbits_update8(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct c1tsbits_ctx *c;
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
c1tsbits_final(struct tras_ctx *ctx)
{
	struct c1tsbits_ctx *c;
	double pvalue, s, v1v2;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;
	if (c->nbits < C1TSBITS_MIN_NBITS)
		return (EALREADY);

	/* todo: implementation, temporary no statistics */
	v1v2 = 0.0;

	s = fabs(v1v2 - C1TSBITS_MEAN) / C1TSBITS_STDDEV / sqrt((double)2.0);
	pvalue = erfc(fabs(s));

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - C1TSBITS_MIN_NBITS;
	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

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
	.update =	c1tsbits_update8,
	.test =		c1tsbits_test,
	.final =	c1tsbits_final,
	.restart =	c1tsbits_restart,
	.free =		c1tsbits_free,
};
