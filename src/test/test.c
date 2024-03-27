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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <tras.h>
#include <utils.h>
#include <algos.h>

#include <sys/random.h>

struct test_algo {
	const char		*name;		/* algorithm name for user */
	const struct tras_algo	*algo;		/* tras algorithm descriptor */
	void			*params;	/* tras algorithm params */
	unsigned int		blocksize;	/* block size for algorithm */
};

struct frequency_params frequency_params = {
	.alpha = 0.05,
};

struct sphere3d_params sphere3d_params = {
	.K = SPHERE3D_MIN_TRIPLETS,
	.alpha = 0.05,
};

struct mindist_params mindist_params = {
	.K = MINDIST_MIN_POINTS,
	.alpha = 0.05,
};

struct plot_params plot_params = {
	.idist = PARKING_LOT_IDIST_COORD_MAX,
	.alpha = 0.05,
};

struct squeeze_params squeeze_params = {
	.K = SQUEEZE_MIN_NUMBERS,
	.alpha = 0.05,
};

static const struct test_algo algo_list[] = {
	{ "frequency", &frequency_algo, &frequency_params, 0 },
	{ "sphere3d", &sphere3d_algo, &sphere3d_params, 0 },
	{ "approxe", NULL, NULL },
	{ "blkfreq", NULL, NULL },
	{ "brank31", NULL, NULL },
	{ "brank68", NULL, NULL },
	{ "bstream", NULL, NULL },
	{ "c1tssbytes", NULL, NULL },
	{ "cusum", NULL, NULL },
	{ "excursion", NULL, NULL },
	{ "fourier", NULL, NULL },
	{ "lcomplex", NULL, NULL},
	{ "maurer", NULL, NULL },
	{ "ntmatch", NULL, NULL },
	{ "opso", NULL, NULL },
	{ "otmatch", NULL, NULL },
	{ "plot", &plot_algo, &plot_params },
	{ "runs", NULL, NULL },
	{ "sphere3d", NULL, NULL },
	{ "squeeze", &squeeze_algo, &squeeze_params },
	{ "bkampmassey", NULL, NULL },
	{ "bmatrix", NULL, NULL },
	{ "brank32", NULL, NULL },
	{ "bspace", NULL, NULL },
	{ "c1tsbits", NULL, NULL },
	{ "craps", NULL, NULL },
	{ "dna", NULL, NULL },
	{ "excursionv", NULL, NULL },
	{ "kstest", NULL, NULL },
	{ "mindist", &mindist_algo, &mindist_params },
	{ "operm5", NULL, NULL },
	{ "oqso", NULL, NULL },
	{ "ovlpsum", NULL, NULL }, 
	{ "serial", NULL, NULL },
	{ NULL, NULL, NULL },
};

/*
 * Identifiers for commands.
 */
#define	TEST_CMD_NONE		0
#define	TEST_CMD_HELP		1
#define	TEST_CMD_LIST		2
#define	TEST_CMD_TEST		3

#define	TEST_OPT_STR		"hlt:"

static int test_cmd = TEST_CMD_HELP;

/*
 * Test selected to run.
 */
static const struct test_algo *test_desc = NULL;

static void
test_usage(void)
{
	printf("test: application to run tras statistica test\n");
	printf("synopsis: test [hlt]\n");
	printf("-h        : print usage of the application\n");
	printf("-l        : print list of algorithms\n");
	printf("-t        : run statistical test\n");	
}

static int
test_cmd_list(void)
{

	printf("test: printing list of algorithms\n");

	return (ENOTSUP);
}

static int
test_cmd_test(void)
{
	const struct tras_algo *algo;
	struct tras_ctx ctx;
	int error, id, nrd, size;
	int total;
	char *data, idstr[64];

	tras_ctx_init(&ctx);

	algo = test_desc->algo;

	error = algo->init(&ctx, test_desc->params);
	if (error != 0) {
		printf("test failed to init %s algorithm\n", algo->name);
		return (error);
	}

	size = test_desc->blocksize;
	size = size ? size : 2048;

	data = malloc(size);
	if (data == NULL) {
		algo->free(&ctx);
		return (ENOMEM);
	}

	for (id = 1; ;) {
		nrd = read(STDIN_FILENO, data, size);
		if (nrd < 0) {
			error = errno;
			break;
		}
		if (nrd == 0) {
			error = 0;
			break;
		}
		error = algo->update(&ctx, data, nrd * 8);
		if (error != 0) {
			printf("test: failed to updadate data for %s test\n",
			    algo->name);
			break;
		}
		total += nrd;
		error = algo->final(&ctx);
		if (error != 0 && error != EALREADY) {
			printf("test: failed to final %s test\n",
			    algo->name);
			break;
		}
		if (error == EALREADY)
			continue;

		snprintf(idstr, sizeof(idstr), "%s test #%d", algo->name, id);

		printf("%-28s: pvalue = %.*f%-8s stats = %.*f%-8s %s\n", idstr,
		    8, ctx.result.pvalue1, "\t", 8, ctx.result.pvalue2, "\t",
			(ctx.result.status == TRAS_TEST_PASSED) ? "success" : "failed");
		error = algo->restart(&ctx, test_desc->params);
		if (error != 0) {
			printf("test: failed to restart %s test\n",
			    algo->name);
			break;
		}
		id++;
	}

	algo->free(&ctx);
	free(data);

	return (error);
}

static int
test_select_test(const char *name)
{
	const struct test_algo *d = &algo_list[0];

	while (d->name != NULL) {
		if (strcmp(d->name, name) != 0) {
			d++;
		} else {
			test_desc = d;
			break;
		}
	}
	return ((test_desc == NULL) ? EINVAL : 0);
}

int main(int argc, char *argv[])
{
	int error, c;

	while ((c = getopt(argc, argv, "hlt:")) != -1) {
		switch (c) {
		case 'h':
			test_usage();
			return (0);
		case 'l':
			test_cmd = TEST_CMD_LIST;
			break;
		case 't':
			test_cmd = TEST_CMD_TEST;
			error = test_select_test(optarg);
			if (error != 0) {
				printf("test: invalid algorithm name\n");
				return (EINVAL);
			}
			break;
		default:
			printf("test: invalid options entered\n");
			test_usage();
			return (EINVAL);
		}
	}

	switch (test_cmd) {
	case TEST_CMD_NONE:
	case TEST_CMD_HELP:
		test_usage();
		return (0);
	case TEST_CMD_LIST:
		error = test_cmd_list();
		break;
	case TEST_CMD_TEST:
		error = test_cmd_test();
		break;
	default:
		printf("test: fatal, invalid command\n");
		exit(-1);
	}

	return (error);
}