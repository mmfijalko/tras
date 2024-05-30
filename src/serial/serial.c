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
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <tras.h>
#include <serial.h>

/*
 * Context structure for serial test.
 */
struct serial_ctx {
	uint32_t 	block;	/* not full m bits or last m-1 bits */
	unsigned int *	m0;	/* frequency table for m bits blocks */
	unsigned int *	m1;	/* frequency table for m-1 bits blocks */
	unsigned int *	m2;	/* frequency table for m-2 bits blocks */
	unsigned int	nbits;	/* number of bits processed */
	unsigned int	m;	/* length of block in bits from params */
	double		alpha;	/* significance level from params */
};

/*
 * Allocate and init context for serial test.
 */
static int
serial_alloc_context(struct tras_ctx *ctx, struct serial_params *p)
{
	struct serial_ctx *c;
	unsigned int sm, me;

	ctx->context = NULL;

	sm = 1 << p->m;
	me = sm + sm / 2 + sm / 4;

	c = malloc(sizeof(struct serial_ctx) + me * sizeof(unsigned int));
	if (c == NULL)
		return (ENOMEM);

	c->m0 = (unsigned int *)(c + 1);
	c->m1 = (unsigned int *)(c->m0 + sm);
	c->m2 = (unsigned int *)(c->m1 + sm / 2);

	bzero(c->m0, me * sizeof(unsigned int));

	c->m = p->m;
	c->alpha = p->alpha;
	c->nbits = 0;

	ctx->context = c;

	return (0);
}

/*
 * Calculate minimum number of bits for the serial test with block size.
 * Input size recommendation : m < floor(log_2(n)) - 2
 */
static int
serial_min_bits(struct tras_ctx *ctx)
{
	struct serial_ctx *c = ctx->context;

	/* todo: */

	return (0);
}

/*
 * Initialize the serial test.
 */
int
serial_init(struct tras_ctx *ctx, void *params)
{
	struct serial_params *p = params;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->m < SERIAL_MIN_M || p->m > SERIAL_MAX_M)
		return (EINVAL);

	/*
	 * Notice: if m == 1 the test is frequency test.
	 */

	error = serial_alloc_context(ctx, p);
	if (error != 0) {
		ctx->state = TRAS_STATE_NONE;
		return (error);
	}

	ctx->algo = &serial_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

/*
 * Update state of the serial test with subsequent binary sequence.
 */
int
serial_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	TRAS_CHECK_UPDATE(ctx, data, nbits);


	/*
	 * TODO: implementation.
	 */

	return (ENOSYS);
}

/*
 * Finalize the serial test and determine its result.
 */
int
serial_final(struct tras_ctx *ctx)
{
	struct serial_ctx *c;
	unsigned int sv0, sv1, sv2;
	unsigned int sm, m, n, i;
	double psim0, psim1, psim2;
	double dpsim1, dpsim2;
	double pvalue1, pvalue2;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;
	
	if (c->nbits < serial_min_bits(ctx))
		return (EALREADY);

	/*
	 * TODO: check condition to finalize the test.
	 */

	n = c->nbits;
	m = c->m;

	sm = (1 << m);
	for (i = 0, sv0 = 0; i < sm; i++)
		sv0 += c->m0[i] * c->m0[i];
	psim0 = (double)sv0 / n * sm - n;

	sm = sm / 2;
	for (i = 0, sv1 = 0; i < sm; i++)
		sv1 += c->m1[i] * c->m1[i];
	psim1 = (double)sv1 / n * sm - n;

	sm = sm / 2;
	for (i = 0, sv2 = 0; i < sm; i++)
		sv2 += c->m2[i] * c->m2[i];
	psim2 = ((m - 2) <= 0) ? 0.0 : (double)sv2 / n * sm - n;

	/* Calculate first test statistics delta psi square for m */
	dpsim1 = psim0 - psim1;

	/* Calculate second test statistics square psi square for m */
	dpsim2 = psim0 - 2 * psim1 + psim2;

	/* todo: Calculate first p-value for the first statistics */
	pvalue1 = 0.0;

	/* todo: Calculate second p-value for the second statistics */
	pvalue2 = 0.0;

	/* Todo: finalize the test. */

	/* Determine and store results */
	if (pvalue1 < c->alpha || pvalue2 < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = n % c->m;
	ctx->result.pvalue1 = pvalue1;
	ctx->result.pvalue2 = pvalue2;

	ctx->state = TRAS_STATE_FINAL;

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
	.update =	serial_update,
	.test =		serial_test,
	.final =	serial_final,
	.restart =	serial_restart,
	.free =		serial_free,
};
