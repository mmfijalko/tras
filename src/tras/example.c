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

#include "tras.h"
#include "frequency.h"

int main(void)
{
	struct tras_ctx ctx;
	unsigned int n, size, blks, nbytes;
	void *data;
	int error;

	/* Generate test data */
	size = 1024 * 1024;
	nblks = 1024;
	data = malloc(size);
	if (data == NULL)
		return (ENOMEM);
	for (i = 0; i < size; i++)
		((char *)data)[i] = (char)(i & 0xff);

	/* Initialize test context */
	error = tras_test_init(&ctx, &frequency_algo, NULL);
	if (error != 0) {
		free(data);
		return (error);
	}

	/* Iterate to update test with data */
	n = size;
	while (n > 0) {
		nbytes = min(n, blks);
		error = tras_test_upate(&ctx, data, 8 * nbytes);
		if (error != 0)
			break;
		n = n - nbytes;
	}
	if (error != 0)
		goto exit;

	/* Finalize test to get results */
	error = tras_test_final(ctx);
	if (error != 0)
		goto exit;

	/* TODO: print results */
exit:
	tras_test_free(&ctx);

	free(data);

	return (error);
}
