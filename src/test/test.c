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
#include <limits.h>
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
	.alpha = 0.01,
};

struct approxe_params approxe_params = {
	.m = 3,
	.alpha = 0.05,
};

struct runs_params runs_params = {
	.alpha = 0.01,
};

struct blkfreq_params blkfreq_params = {
	.m = 64,
	.alpha = 0.01,
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

struct dna_params dna_params = {
	.alpha = 0.01,
};

struct dna_params opso_params = {
	.alpha = 0.01,
};

struct dna_params otso_params = {
	.alpha = 0.01,
};

struct bstream_params bstream_params = {
	.alpha = 0.01,
};

struct sparse_params sparse_params_opso = {
	.m = 1024,
	.k = 2,
	.b = 10,
	.r = 32,
	.wmax = SPARSE_MAX_WORDS,
	.mean = 141909.3299550069,
	.var = 290.4622634038,
	.alpha = 0.01,
};

struct sparse_params sparse_params_otso = {
	.m = 64,
	.k = 3,
	.b = 6,
	.r = 32,
	.wmax = SPARSE_MAX_WORDS,
//	.mean = 87.9395,
	.mean = 87.85,
	.var = 9.37,
	.alpha = 0.01,
};

struct sparse_params sparse_params_oqso = {
	.m = 32,
	.k = 4,
	.b = 5,
	.r = 32,
	.wmax = SPARSE_MAX_WORDS,
	.mean = 141909.6005321316,
	.var = 294.6558723658,
	.alpha = 0.01,
};

struct sparse_params sparse_params_dna = {
	.m = 4,
	.k = 10,
	.b = 2,
	.r = 32,
	.wmax = SPARSE_MAX_WORDS,
	.mean = 141910.4026047629,
	.var = 337,0,
	.alpha = 0.01,
};

struct universal_params maurer_params = {
	.alpha = 0.01,
	.L = 6,
	.Q = 10 * (1 << 6),
};

struct universal_params coron_params = {
	.alpha = 0.01,
	.L = 6,
	.Q = 10 * (1 << 6),
};

struct cusum_params cusum_params_fw = {
	.mode = CUSUM_MODE_FORWARD,
	.alpha = 0.01,
};

struct cusum_params cusum_params_bw = {
	.mode = CUSUM_MODE_BACKWARD,
	.alpha = 0.01,
};

struct excursion_params excursion_params = {
	.alpha = 0.01,
};

struct excursionv_params excursionv_params = {
	.alpha = 0.01,
};

struct bspace_params bspace_params = {
	.b = 0,			/* for test purpose only */
	.m = 512,		/* 2 ^ 9 */
	.q = 24,		/* 24 least significance bits */
	.n = 16 * 1024 * 1024,	/* 2 ^ 24 */
	.nk = 0,		/* ??? */
	.alpha = 0.01,		/* significance level */
};

struct craps_params craps_params = {
	.K = 200000,
	.throws = 200000 * 100,
	.alpha1 = 0.01,
	.alpha2 = 0.01,
};

struct longruns_params longruns_params = {
	.M = 128,
	.N = 64,
	.alpha = 0.01,
.version = 1,
};

static const struct test_algo algo_list[] = {
	{ "frequency", &frequency_algo, &frequency_params, 0 },
	{ "sphere3d", &sphere3d_algo, &sphere3d_params, 0 },
	{ "approxe", &approxe_algo, &approxe_params },
	{ "blkfreq", &blkfreq_algo, &blkfreq_params },
	{ "brank31", NULL, NULL },
	{ "brank68", NULL, NULL },
	{ "bstream", &bstream_algo, &bstream_params },
	{ "c1tssbytes", NULL, NULL },
	{ "cusum", &cusum_algo, &cusum_params_fw },
	{ "cusumfw", &cusum_algo, &cusum_params_fw },
	{ "cusumbw", &cusum_algo, &cusum_params_bw },
	{ "fourier", NULL, NULL },
	{ "lcomplex", NULL, NULL},
	{ "maurer", &maurer_algo, &maurer_params },
	{ "coron", &coron_algo, &coron_params },
	{ "ntmatch", NULL, NULL },
	{ "otmatch", NULL, NULL },
	{ "plot", &plot_algo, &plot_params },
	{ "runs", &runs_algo, &runs_params },
	{ "longruns", &longruns_algo, &longruns_params },
	{ "sphere3d", NULL, NULL },
	{ "squeeze", &squeeze_algo, &squeeze_params },
	{ "bkampmassey", NULL, NULL },
	{ "bmatrix", NULL, NULL },
	{ "brank32", NULL, NULL },
	{ "bspace", &bspace_algo, &bspace_params },
	{ "c1tsbits", NULL, NULL },
	{ "craps", &craps_algo, &craps_params },

	{ "sparse_opso", &sparse_algo, &sparse_params_opso },
	{ "sparse_otso", &sparse_algo, &sparse_params_otso },
	{ "sparse_oqso", &sparse_algo, &sparse_params_oqso },
	{ "sparse_dna", &sparse_algo, &sparse_params_dna },

	{ "opso", &opso_algo, &opso_params },
	{ "otso", &otso_algo, &otso_params },
	{ "oqso", &oqso_algo, &oqso_params },
	{ "dna", &dna_algo, &dna_params },

	{ "dna_sparse", &dna_sparse_algo, NULL },	/* ??? */

	{ "excursion", &excursion_algo, &excursion_params },
	{ "excursionv", &excursionv_algo, &excursionv_params },
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
 * Maximum number of bytes to process.
 */
static unsigned int test_total = 0;

/*
 * Maximum number of bits for one single test.
 */
static unsigned int test_maxnbits = 0;

/*
 * Test selected to run.
 */
static const struct test_algo *test_desc = NULL;

#define	min(a, b)	(((a) < (b)) ? (a) : (b))

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
test_atoi(const char *str, long *lval)
{
	long val;

	val = strtoul(str, NULL, 10);
	if (val == 0 && errno == EINVAL)
		return (EINVAL);
	if (errno == ERANGE)
		return (ERANGE);
	*lval = val;

	return (0);
}

static int
test_getuint(const char *str, unsigned int *ival)
{
	long lval;
	int error;

	error = test_atoi(str, &lval);
	if (error != 0)
		return (error);
	if (lval < 0 || lval > (long)UINT_MAX)
		return (EINVAL);
	*ival = (unsigned int)lval;

	return (0);
}

static int
test_file_read(int fd, void *data, size_t *size)
{
	ssize_t nrd, n, total = 0;

	n = *size;
	while (n > 0) {
		nrd = read(STDIN_FILENO, (char *)data + total, n);
		if (nrd < 0)
			return (errno);
		if (nrd == 0) {
			*size = total;
			break;
		}
		n = n - nrd;
		total = total + nrd;
	}
	*size = total;

	return (0);
}

static int
test_stdin_read(void *data, size_t *size)
{

	return (test_file_read(STDIN_FILENO, data, size));
}

static void
test_show_result(const struct tras_algo *algo, struct tras_ctx *ctx, int id)
{
	char idstr[64];

	snprintf(idstr, sizeof(idstr), "%s test #%d", algo->name, id);

	printf("%-28s: pvalue = %.*f%-8s stats1 = %.*f%-8s %s\n",
	    idstr, 8, ctx->result.pvalue1, "\t", 8, ctx->result.stats1, "\t",
	    (ctx->result.status == TRAS_TEST_PASSED) ? "success" : "failed");
}

static int
test_cmd_test(void)
{
	const struct tras_algo *algo;
	struct tras_ctx ctx;
	size_t nread;
	unsigned int n, size, nupd, ntest;
	int error, id, restart = 0;
	char *data, *p, idstr[64];

	if (test_desc->algo == NULL) {
		printf("test not implemented yet\n");
		return (EINVAL);
	}

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
	ntest = 0;
	nread = 0;

	id = 1;
	n = (test_total > 0) ? test_total : UINT_MAX;

	while (n > 0) {
		nread = min(size, n);
		error = test_stdin_read(data, &nread);
		if (error != 0 || nread == 0)
			break;

		nread = nread * 8;

		if (test_maxnbits > 0) {
			nupd = min(test_maxnbits, ntest);
			nupd = test_maxnbits - nupd;
		} else {
			nupd = UINT_MAX;
		}
		nupd = min(nupd, nread);

		if (nupd > 0) {
			error = algo->update(&ctx, data, nupd);
			if (error != 0) {
				printf("test: failed to updadate data for %s test\n",
				    algo->name);
				break;
			}
			ntest += nupd;
		}
		if (test_maxnbits > 0) {
			nupd = test_maxnbits - ntest;
			if (nupd == 0) {
				error = algo->final(&ctx);
				if (error != 0)
					break;
				restart = 1;
			}
		} else {
			error = algo->final(&ctx);
			if (error != 0) {
				if (error != EALREADY)
					break;
			} else {
				restart = 1;
			}
		}

		if (restart) {
			test_show_result(algo, &ctx, id);
			error = algo->restart(&ctx, test_desc->params);
			if (error != 0) {
				printf("test: failed to restart %s test\n",
				    algo->name);
				break;
			}
			ntest = 0;
			restart = 0;
			id++;
		}
		n = n - nread;
		nread = 0;
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
		if (strcmp(d->name, name) == 0) {
			test_desc = d;
			break;
		}
		d++;
	}
	return ((test_desc == NULL) ? EINVAL : 0);
}

#define	TEST_OPTSTR	"hlt:s:S"

int main(int argc, char *argv[])
{
	int error, c;

	while ((c = getopt(argc, argv, TEST_OPTSTR)) != -1) {
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
		case 'S':
			error = test_getuint(optarg, &test_total);
			if (error != 0) {
				printf("test: invalid total size\n");
				return (error);
			}
		case 's':
			break;
			error = test_getuint(optarg, &test_maxnbits);
			if (error != 0) {
				printf("test: invalid max nbits for the test\n");
				return (error);
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
