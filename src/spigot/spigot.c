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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

static int
e_spigot(unsigned int n)
{
	int i, j, k, q, x, nines, predigit;
	unsigned long *a;
	int count;

	a = malloc((n + 1) * sizeof(unsigned long));
	if (a == NULL) {
		printf("failed to alloc memory\n");
		return (ENOMEM);
	}

	for (j = 1; j <= n; j++)
		a[j] = 1;

	printf("10");
	count = 2;
	for (j = 0; j < n; j++) {
		q = 0;
		for (i = n; i > 0; i--) {
			x = (a[i] << 1) + q;
			a[i] = x % i;
			q = x / i;
		}
		a[1] = q & 0x01;
		q = q >> 1;
		if (j == 22) {
			printf("\n");
			count = 0;
		} else if (count == 25) {
			printf("\n");
			count = 0;
		}
		printf("%d", (int)a[1]);
		count++;
	}
	free(a);

	return (0);
}

static void
usage(void)
{

	printf("spigot, the application to calculate the binary\n");
	printf("    expansion of the Euler's number\n");
	printf("syntax: spigot [hI] -n bits [-bf] file\n");
	printf("-h        : print this help\n");
}

int
main(int argc, char *argv[])
{
#if 0
	int c;

	while ((c = getopt(argc, argv, "hIn:b:f:")) != -1) {
		switch (c) {
		case 'h':
			usage();
			return (0);
		case 'I':
			break;
		case 'n':
			break;
		case 'b':
			break;
		case 'f':
			break;
		default:
			printf("spigot: invalid options\n");
			usage();
			return (EINVAL);
		}
	}
#endif

	e_spigot(1004880 - 2);

	return (0);
}
