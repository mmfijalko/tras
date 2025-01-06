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
 * Count-the-1's Test (Stream of Specific Bytes).
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
#include <c1tsbyte.h>

	#include <stdio.h>

int
c1tsbyte_init(struct tras_ctx *ctx, void *params)
{
	struct c1tsbyte_params *p = params;
	struct c1tsbyte_ctx *c;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	size = sizeof(struct c1tsbits_ctx) + 625 * sizeof(unsigned int) +
	    3125 * sizeof(unsigned int);

	error = tras_init_context(ctx, &c1tsbyte_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);
	c = ctx->context;

	c->w4freq = (unsigned int *)(c + 1);
	c->w5freq = (unsigned int *)(c->w4freq + 625);
	c->alpha = p->alpha;

	return (0);
}

int
c1tsbyte_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (ENOSYS);

}

int
c1tsbyte_final(struct tras_ctx *ctx)
{

	return (ENOSYS);
}

int
c1tsbyte_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
c1tsbyte_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
c1tsbyte_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo c1tsbyte_algo = {
	.name =		"c1tsbyte",
	.desc =		"Count-the-1's Test (Stream of Specific Bytes)",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		c1tsbyte_init,
	.update =	c1tsbyte_update,
	.test =		c1tsbyte_test,
	.final =	c1tsbyte_final,
	.restart =	c1tsbyte_restart,
	.free =		c1tsbyte_free,
};
