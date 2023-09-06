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
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include <tras.h>
#include <utils.h>
#include <blkfreq.h>

#include <sys/random.h>

int main(void)
{
	struct blkfreq_params params = {
		.m = 21,
		.alpha = 0.01,
	};
	struct tras_ctx ctx;
	int error, i, j, nrd;
	char buffer[2048];

	tras_ctx_init(&ctx);

	getrandom(buffer, sizeof(buffer), 0);

	error = blkfreq_init(&ctx, &params);
	if (error != 0) {
		printf("test: failed to init blkfreq test\n");
		return (error);
	}

	for (j = 0; ; j++) {
		nrd = read(STDIN_FILENO, buffer, sizeof(buffer));
		if (nrd < 0) {
			return (errno);
		}
		if (nrd == 0) {
			return (0);
		}
		if (nrd != sizeof(buffer)) {
			return (ENXIO);
		}

	error = blkfreq_update(&ctx, buffer, 8 * sizeof(buffer));
	if (error != 0) {
		printf("test: failed to update blkfreq test\n");
		blkfreq_free(&ctx);
		return (error);
	}

	error = blkfreq_final(&ctx);
	if (error != 0) {
		printf("test: failed to finalize the blkfreq test\n");
		blkfreq_free(&ctx);
		return (error);
	}

//	printf("pvalue = %g\n", ctx.result.pvalue1);

	error = blkfreq_restart(&ctx, &params);
	if (error != 0) {
		printf("test: failed to restart blkfreq test\n");
		break;
	}
	}

	error = blkfreq_free(&ctx);
	if (error != 0) {
		printf("test: failed to free blkfreq context (%d)\n", error);
		return (error);
	}

	return (0);
}

