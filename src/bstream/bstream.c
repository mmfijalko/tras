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
 * The Bitstream Test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tras.h>
#include <hamming8.h>
#include <utils.h>
#include <bits.h>
#include <cdefs.h>
#include <sparse.h>
#include <bstream.h>

/*
 * The bistream test is one of the sparse occupancy test variant.
 * Using sparse occupancy test context so the parameters.
 */
static const struct sparse_params bstream_params = {
	.m = 2,
	.k = 20,
	.b = 1,
	.r = 1,
	.wmax = SPARSE_MAX_WORDS,
	.mean = 141909.0,
	.var = 428.0,
	.alpha = 0.01,
};

int
bstream_init(struct tras_ctx *ctx, void *params)
{
	struct bstream_params *p = params;
	struct sparse_params sp;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	memcpy(&sp, &bstream_params, sizeof(struct sparse_params));
	sp.alpha = p->alpha;

	error = sparse_init(ctx, &sp);
	if (error != 0)
		return (error);

	ctx->algo = &bstream_algo;

	return (0);	
}

int
bstream_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct sparse_ctx *c;
	unsigned int n, i, j, k;
	uint32_t word, mask, wpos;
	uint8_t *bytes = (uint8_t *)data;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;

	if (nbits == 0 || c->nbits >= BSTREAM_BITS) {
		c->nbits += nbits;
		return (0);
	}

	k = min(c->letters, BSTREAM_WORDLEN);
	k = min(nbits, BSTREAM_WORDLEN - k);
	for (i = 0; i < k; i++) {
		c->word = ((c->word << 1) & ~0x01) & 0x000fffff;
		c->word |= (bytes[i >> 3] >> (7 - (i & 0x07))) & 0x01;
	}
	c->letters += k;

	if (c->letters < BSTREAM_WORDLEN || c->letters >= BSTREAM_LETTERS) {
		c->nbits += nbits;
		return (0);
	}

	n = nbits - k;
	n = min(n, BSTREAM_LETTERS - c->letters);
	word = c->word;
	for (j = 0, i = k; j < n; j++, i++) {
		mask = (bytes[i >> 3] >> (7 - (i & 0x07))) & 0x01;
		word = ((word << 1) | mask) & 0x000fffff;
		mask = 1 << (word & 0x1f);
		wpos = word >> 5;
		if ((c->wmap[wpos] & mask) == 0) {
			c->wmap[wpos] |= mask;
			c->sparse--;
		}
	}

	c->word = word;
	c->letters += n;
	c->nbits += nbits;

	return (0);
}

int
bstream_final(struct tras_ctx *ctx)
{

	return (sparse_final(ctx));
}

int
bstream_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
bstream_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
bstream_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo bstream_algo = {
	.name =		"bstream",
	.desc =		"The Bitstream Test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		bstream_init,
	.update =	bstream_update,
	.test =		bstream_test,
	.final =	bstream_final,
	.restart =	bstream_restart,
	.free =		bstream_free,
};
