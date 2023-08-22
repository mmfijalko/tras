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

struct blkfreq_ctx {
	uint8_t *	block;	/* buffer for not full block */
	unsigned int	nbits;	/* total number of bits updated */
	unsigned int	nblks;	/* full blocked updated */
	double		stats;	/* statistics for updated blocks */
};

static int
blkfreq_allow_final(struct blkfreq_ctx *c)
{

	if (c->nbits < BLKFREQ_MIN_N)
		return (0);

	return (1);
}

#define	blkfreq_min_nbits(M)	BLKFREQ_MIN_N
#define	blkfreq_max_nbits(M)	((M) * 100 - 1)

typedef int (*tras_test_init_t)(void *);
typedef int (*tras_test_test_t)(void *);
typedef int (*tras_test_update_t)(void *);
typedef int (*tras_test_final_t)(void *);
typedef int (*tras_test_reset_t)(void *);
typedef int (*tras_test_free_t)(void *);

static int
blkfreq_allow_update(struct blkfreq_ctx *c)
{
}

int
blkfreq_init(void *ctx)
{

	return (0);
}

int
blkfreq_set_params(void *ctx, void *param)
{

	return (0);
}

int
blkfreq_test(void *ctx, void *data, unsigned int bits)
{

	return (0);
}

int
blkfreq_update(void *ctx, void *data, unsigned int bits)
{

	return (0);
}

int
blkfreq_final(void *ctx)
{

	return (0);
}

int
blkfreq_reset(void *ctx)
{

	return (0);
}

int
blkfreq_free(void *ctx)
{

	return (0);
}