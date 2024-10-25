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
 * The serial test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tras.h>
#include <cdefs.h>
#include <igamc.h>
#include <serial.h>

#include <stdio.h>

#ifdef SERIAL_DEBUG

#define	serial_printf(msg, ...) do {			\
	printf("%s: " msg, __func__, ##__VA_ARGS__);	\
} while (0)

#else

#define serial_printf(msg, ...)

#endif

/*
 * Context structure for the serial test.
 */
struct serial_ctx {
	uint32_t	first;	/* first m - 1 bits for augmenting */
	uint32_t 	block;	/* not full m bits or last m-1 bits */
	unsigned int *	m0;	/* frequency table for m bits blocks */
	unsigned int *	m1;	/* frequency table for m-1 bits blocks */
	unsigned int *	m2;	/* frequency table for m-2 bits blocks */
	unsigned int	nbits;	/* number of bits processed */
	unsigned int	m;	/* length of block in bits from params */
	double		alpha;	/* significance level from params */
};

/*
 * Calculate minimum number of bits for the serial test with block size.
 * Input size recommendation : m < floor(log_2(n)) - 2
 */
static int
serial_min_bits(struct tras_ctx *ctx)
{
	struct serial_ctx *c = ctx->context;

	return (pow(2, c->m + 2) + 1);
}

/*
 * Initialize the serial test.
 */
int
serial_init(struct tras_ctx *ctx, void *params)
{
	struct serial_params *p = params;
	struct serial_ctx *c;
	unsigned int sm, me;
	int size, error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->m < SERIAL_MIN_M || p->m > SERIAL_MAX_M)
		return (EINVAL);

	/*
	 * Notice: if m == 1 the test is frequency test.
	 */

	sm = 1 << p->m;
	me = sm + sm / 2 + sm / 4;

	size = sizeof(struct serial_ctx) + me * sizeof(unsigned int);

	error = tras_init_context(ctx, &serial_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);
	c = ctx->context;

	c->m0 = (unsigned int *)(c + 1);
	c->m1 = (unsigned int *)(c->m0 + sm);
	c->m2 = (unsigned int *)(c->m1 + sm / 2);

	c->m = p->m;
	c->alpha = p->alpha;

	return (0);
}

/*
 * Update state when first m - 1 bits are collected.
 */
static void
serial_update_bits(struct serial_ctx *c, unsigned int offs, void *data,
    unsigned int nbits)
{
	uint32_t block, m0, m1, m2;
	uint8_t *p, m;
	unsigned int n;

	serial_printf("update bits, offs=%u, nbits=%u\n", offs, nbits);

	m0 = (1 << c->m) - 1;
	m1 = m0 >> 1;
	m2 = m1 >> 1;

	p = (uint8_t *)data + offs / 8;
	m = 0x80 >> (offs & 0x07);
	n = nbits;

	block = c->block;

	while (n > 0) {
		block = (block << 1) | ((*p & m) ? 1 : 0);
		c->m0[block & m0]++;
		c->m1[block & m1]++;
		c->m2[block & m2]++;
		m = m >> 1;
		if (m == 0) {
			p++;
			m = 0x80;
		}
		n--;
	}
	c->block = block;
}

/*
 * Update state of the serial test with subsequent binary sequence.
 */
int
serial_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct serial_ctx *c;
	uint32_t block, m0, m1, m2;
	unsigned int n, k, j, offs, i;
	uint8_t *p, m;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	if (nbits == 0)
		return (0);

	c = ctx->context;
	p = (uint8_t *)data;

	m0 = (1 << c->m) - 1;
	m1 = (1 << (c->m - 1)) - 1;
	m2 = (1 << (c->m - 2)) - 1;

	block = c->block;

	/*
	 * Building first m - 1 bits block and context.
	 */
	k = c->nbits;

	if (k < (c->m - 1)) {
		n = c->m - 1 - c->nbits;
		n = min(n, nbits);
		offs = n;
		while (n > 0) {
			j = min(n, 8);
			for (i = 0, m = 0x80; i < j; i++, m = m >> 1) {
				block = (block << 1) | ((*p & m) ? 1 : 0);
				k++;
				if (k >= (c->m - 1))
					c->m1[block & m1]++;
				if (k >= (c->m - 2))
					c->m2[block & m2]++;
			}
			n = n - j;
			p++;
		}
		/* If collected store first m - 1 bits for augmenting */
		if (k == (c->m - 1))
			c->first = block;
	}

	/*
	 * Ready to iterate with all blocks.
	 */
	serial_update_bits(c, offs, data, nbits - offs);

	c->nbits += nbits;

	return (0);
}

int
serial_update2(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct serial_ctx *c;
	uint32_t block, m0, m1, m2;
	unsigned int n, k, j, offs, i;
	uint8_t *p, m;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	if (nbits == 0)
		return (0);

	serial_printf("processing %u bits\n", nbits);

	c = ctx->context;
	p = (uint8_t *)data;

	m0 = (1 << c->m) - 1;
	m1 = m0 >> 1;
	m2 = m1 >> 1;

	/*
	 * Building first m - 1 bits block and context.
	 */
	k = c->nbits;

	if (k < (c->m - 1)) {
		serial_printf("updating first m - 1 bits\n");
		block = c->block;
		n = c->m - 1 - k;
		n = min(n, nbits);
		offs = n;
		while (n > 0) {
			j = min(n, 8);
			for (i = 0, m = 0x80; i < j; i++, m >>= 1)
				block = (block << 1) | ((*p & m) ? 1 : 0);
			n = n - j;
			k = k + j;
			p++;
		}
		c->block = block;
		if (k == (c->m - 1)) {
			c->first = block;
			c->m1[block & m1]++;
			c->m2[block & m2]++;
			block = block >> 1;
			c->m2[block & m2]++;
			serial_printf("first block = %x\n", c->first);
		}
	} else {
		offs = 0;
	}

	if (nbits > offs) {
		/*
		 * Ready to iterate with all blocks.
		 */
		serial_update_bits(c, offs, data, nbits - offs);
	}

	c->nbits += nbits;

	return (0);
}

/*
 * Finalize the serial test and determine its result.
 */
int
serial_final(struct tras_ctx *ctx)
{
	struct serial_ctx *c;
	unsigned int sv0, sv1, sv2;
	unsigned int sm, m, n, i, k;
	double psim0, psim1, psim2;
	double dpsim1, dpsim2;
	double pvalue1, pvalue2;
	uint8_t b;

unsigned int sf, total, mmin, mmax;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;
	
	if (c->nbits < serial_min_bits(ctx))
		return (EALREADY);

	n = c->m - 1;
	c->first = c->first << (8 - (n & 0x07));
	while (n > 0) {
		k = (n + 7) / 8 * 8;
		b = (uint8_t)((c->first >> k) & 0xff);
		k = min(n, 8);
		serial_update_bits(c, 0, &b, k);
		n = n - k;
	}

//////////////////////////////////////////////////////////////

	sm = 1 << c->m;

	mmax = 0;
	mmin = c->nbits;
	serial_printf("table for m0\n");
	for (i = 0, total = 0; i < sm; i++) {
		serial_printf("m0[%u] = %u, \n", i, c->m0[i]);
		if (c->m0[i] < mmin)
			mmin = c->m0[i];
		if (c->m0[i] > mmax)
			mmax = c->m0[i];
		total += c->m0[i];
	}
	serial_printf("total for m0=%u\n", total);
	serial_printf("mmin = %u, mmax = %u\n", mmin, mmax);

	serial_printf("table for m1\n");
	mmax = 0;
	mmin = c->nbits;
	for (i = 0, total = 0; i < sm / 2; i++) {
		serial_printf("m1[%u] = %u, \n", i, c->m1[i]);
		if (c->m1[i] < mmin)
			mmin = c->m1[i];
		if (c->m1[i] > mmax)
			mmax = c->m1[i];
		total += c->m1[i];
	}
	serial_printf("total for m1=%u\n", total);
	serial_printf("mmin = %u, mmax = %u\n", mmin, mmax);

	serial_printf("table for m2\n");
	mmax = 0;
	mmin = c->nbits;
	for (i = 0, total = 0; i < sm / 4; i++) {
		serial_printf("m2[%u] = %u, \n", i, c->m2[i]);
		if (c->m2[i] < mmin)
			mmin = c->m2[i];
		if (c->m2[i] > mmax)
			mmax = c->m2[i];
		total += c->m2[i];
	}
	serial_printf("total for m2=%u\n", total);
	serial_printf("mmin = %u, mmax = %u\n", mmin, mmax);
//////////////////////////////////////////////////////////////

	n = c->nbits;
	m = c->m;

	sm = (1 << m);
	for (i = 0, sv0 = 0; i < sm; i++)
		sv0 += c->m0[i] * c->m0[i];
	serial_printf("sv0 = %u\n", sv0);
	psim0 = (double)sv0 / n * sm - n;
	serial_printf("psim0 = %f\n", psim0);

	sm = sm / 2;
	for (i = 0, sv1 = 0; i < sm; i++)
		sv1 += c->m1[i] * c->m1[i];
	serial_printf("sv1 = %u\n", sv1);
	psim1 = (double)sv1 / n * sm - n;
	serial_printf("psim1 = %f\n", psim1);

	sm = sm / 2;
	for (i = 0, sv2 = 0; i < sm; i++)
		sv2 += c->m2[i] * c->m2[i];
	serial_printf("sv2 = %u\n", sv2);
	psim2 = ((m - 2) <= 0) ? 0.0 : (double)sv2 / n * sm - n;
	serial_printf("psim2 = %f\n", psim2);

	/* Calculate first test statistics delta psi square for m */
	dpsim1 = psim0 - psim1;

	/* Calculate second test statistics square psi square for m */
	dpsim2 = psim0 - 2 * psim1 + psim2;

	/* todo: Calculate first p-value for the first statistics */
	pvalue1 = igamc(pow(2, m - 2), dpsim1);
	serial_printf("getting imagc(%f, %f) = %f\n", pow(2, m - 2), dpsim1, pvalue1);

	/* todo: Calculate second p-value for the second statistics */
	pvalue2 = igamc(pow(2, m - 3), dpsim2);
	serial_printf("getting imagc(%f, %f) = %f\n", pow(2, m - 3), dpsim2, pvalue2);

	/* Determine and store results */
	if (pvalue1 < c->alpha || pvalue2 < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = n % c->m;
	ctx->result.stats1 = dpsim1;
	ctx->result.stats2 = dpsim2;
	ctx->result.pvalue1 = pvalue1;
	ctx->result.pvalue2 = pvalue2;

	tras_fini_context(ctx, 0);

	return (0);
}

/*
 * The serial test update with binary sequence and finalize it.
 */
int
serial_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

/*
 * Restart the initialized serial test with new parameters.
 */
int
serial_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

/*
 * Uninitialize the serial test and free its allocated resources.
 */
int
serial_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

/*
 * The general serial test algorithm description for tras software.
 */
const struct tras_algo serial_algo = {
	.name =		"Serial",
	.desc =		"Serial Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		serial_init,
	.update =	serial_update2,
	.test =		serial_test,
	.final =	serial_final,
	.restart =	serial_restart,
	.free =		serial_free,
};
