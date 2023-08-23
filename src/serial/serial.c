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
 * TODO: implementation of the Serial Test.
 */

/*
 * Input size recomendation:
 *
 * m < floor(log2(n)) - 2
 *
 */

struct serial_params {
	unsigned int	m;	/* length of overlapping bit block */
};

struct serial_ctx {
	unsigned char *	block;	/* not full m bits or last m-1 bits */
	unsigned int *	m0;	/* frequency table for m bits blocks */
	unsigned int *	m1;	/* frequency table for m-1 bits blocks */
	unsigned int *	m2;	/* frequency table for m-2 bits blocks */
	unsigned int	nbits;	/* number of bits processed */
};

int
serial_init(void *ctx)
{

	return (0);
}

int
serial_set_params(void *ctx, struct serial_params *p)
{
	struct serial_ctx *c;
	unsigned int sb, sm;

	if (m < SERIAL_MIN_M || m > SERIAL_MAX_M)
		return (EINVAL);

	sb = (p->m + 7) / 8;
	sm = (1 << p->m);

	c = malloc(sizeof(struct serial_ctx) + sb + (sm + sm / 2 + sm / 4) *
	    sizeof(unsigned int));
	if (c == NULL)
		return (ENOMEM);

	c->block = (unsigned char *)(c + 1);
	c->m0 = (unsigned int *)(c->block + sb);
	c->m1 = (unsigned int *)(c->m0 + sm);
	c->m2 = (unsigned int *)(c->m1 + sm / 2);

	return (0);
}

int
serial_update(void *ctx, void *data, unsigned int nbits)
{

	return (0);
}

int
serial_test(void *ctx, void *data, unsigned int nbits)
{
	int error;

	error = serial_update(ctx, data, nbit);
	if (error != 0)
		return (error);

	error = serial_final(ctx);
	if (error != 0)
		return (error);

	return (0);
}

int
serial_final(void *ctx)
{
	struct serial_ctx *c;
	unsigned int i;
	unsigned int sv0, sv1, sv2;
	unsigned int sm, m, n;
	double psim0, psim1, psim2;
	double pvalue1, pvalue2;

	/*
	 * TODO: check condition to finalize the test.
	 */
	n = c->nbits;

	sm = (1 << m);
	for (i = 0, sv0 = 0; i < sm; i++)
		sv0 += c->m0[i] * c->m0[i];
	psim0 = (double)sv0 / n * sm - n;

	sm = sm / 2;
	for (i = 0, sv1 = 0, i < sm; i++)
		sv1 += c->m1[i] * c->m1[i];
	psim1 = (double)sv1 / n * sm - n;

	sm = sm / 2;
	for (i = 0, sv2 = 0, i < sm; i++)
		sv2 += c->m1[i] * c->m1[i];
	psim2 = (double)sv2 / n * sm - n;

	/* Calculate first test statistics delta psi square for m */
	dpsim1 = psim0 - psim1;

	/* Calculate second test statistics square psi square for m */
	dpsim2 = psim0 - 2 * psim1 + psim2;

	/* Calculate first p-value for the first statistics */
	pvalue1 = 0.0;

	/* Calculate second p-value for the second statistics */
	pvalue2 = 0.0;

	/* Todo: finalize the test. */

	return (0);
}

