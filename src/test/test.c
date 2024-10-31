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
#include <ctype.h>
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

struct oxso_params dna_params = {
	.boff = 30,
	.alpha = 0.01,
};

struct oxso_params opso_params = {
	.boff = 22,
	.alpha = 0.01,
};

struct oxso_params otso_params = {
	.boff = 26,
	.alpha = 0.01,
};

struct oxso_params oqso_params = {
	.boff = 27,
	.alpha = 0.01,
};

struct bstream_params bstream_params = {
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
	.m = 512,		/* 2 ^ 9 */
	.n = 16 * 1024 * 1024,	/* 2 ^ 24 */
	.b = 0,			/* for test purpose only */
	.q = 24,		/* 24 least significance bits */
	.jn = 200,		/* number of sub-tests */
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

struct bmrank_params bmrank_pq31_params = {
	.uniform = 1,
	.m = 31,
	.q = 31,
	.nr = 3,
	.s0 = 0,
	.N = 40000,
	.alpha = 0.01,
};

struct bmrank_params bmrank_pq31_params_nonuni = {
	.uniform = 0,
	.m = 31,
	.q = 31,
	.nr = 2,
	.s0 = 0,
	.N = 38,
	.alpha = 0.01,
};

struct bmrank_params bmrank_pq32_params = {
	.uniform = 0,
	.m = 32,
	.q = 32,
	.nr = 3,
	.s0 = 0,
	.N = 40000,
	.alpha = 0.01,
};

struct bmrank_params bmrank_pq68_params = {
	.uniform = 0,
	.m = 6,
	.q = 8.,
	.nr = 2,
	.s0 = 0,
	.N = 100000,
	.alpha = 0.01,
};

struct brank31_params brank31_params = {
	.alpha = 0.01,
};

struct brank32_params brank32_params = {
	.alpha = 0.01,
};

struct brank68_params brank68_params = {
	.byte = 0,
	.alpha = 0.01,
};

static const struct test_algo algo_list[] = {
	{ "frequency", &frequency_algo, &frequency_params, 0 },
	{ "sphere3d", &sphere3d_algo, &sphere3d_params, 0 },
	{ "approxe", &approxe_algo, &approxe_params },
	{ "blkfreq", &blkfreq_algo, &blkfreq_params },
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

	{ "bmrank-pq31", &bmrank_algo, &bmrank_pq31_params },
	{ "bmrank-pq32", &bmrank_algo, &bmrank_pq32_params },
	{ "bmrank-pq68", &bmrank_algo, &bmrank_pq68_params },

	{ "bmrank-pq31-non-uniform", &bmrank_algo, &bmrank_pq31_params_nonuni },

	{ "brank32", &brank32_algo, &brank32_params },
	{ "brank31", &brank31_algo, &brank31_params },
	{ "brank68", &brank68_algo, &brank68_params },

	{ "bspace", &bspace_algo, &bspace_params },
	{ "c1tsbits", NULL, NULL },
	{ "craps", &craps_algo, &craps_params },
	{ "opso", &opso_algo, &opso_params },
	{ "otso", &otso_algo, &otso_params },
	{ "oqso", &oqso_algo, &oqso_params },
	{ "dna", &dna_algo, &dna_params },
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

static void
test_shift_data(void *data, size_t size, int offs)
{
	char *p;

	/* todo: */
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

struct mulstr {
	const char *	str;
	unsigned long	mul;
};

static const struct mulstr test_mulstr[] = {
	{ .str = "b",  .mul = 1, },
	{ .str = "B",  .mul = 8, },
	{ .str = "kb", .mul = 1024, },
	{ .str = "Mb", .mul = 1024 * 1024, },
	{ .str = "Gb", .mul = 1024 * 1024 * 1024, },
	{ .str = "kB", .mul = 8 * 1024, },
	{ .str = "MB", .mul = 8 * 1024 * 1024, },
//	{ .str = "GB", .mul = 8 * 1024 * 1024 * 1024 },
	{ .str = NULL, .mul = 0, },
};

static int
test_getsize(char *str, unsigned int *ival)
{
	const struct mulstr *m = test_mulstr;
	char *p = str, *ep, c;
	long lval;
	unsigned int mul;

	if (str == NULL || ival == NULL)
		return (EINVAL);

	mul = 1;

	lval = strtoul(str, &ep, 0);
	if (lval == 0 && errno != 0)
		return (errno);
	if (lval == ULONG_MAX)
		return (errno);
	if (lval < 0)
		return (EINVAL);
	if (*ep != '\0') {
		if (strncmp(str, "0x", 2) == 0)
			return (EINVAL);
		while (m->str != NULL) {
			if (strcmp(m->str, ep) == 0)
				break;
			m++;
		}
		if (m->str == NULL)
			return (EINVAL);
		mul = m->mul;
	} 
	if (lval >= ULONG_MAX / mul)
		return (EINVAL);

	*ival = (unsigned int)(lval * mul);

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

#define miss(c, cmax)   (((c) < (cmax)) ? (cmax) - (c) : 0)

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
	if (test_maxnbits == 0) {
		printf("number of bits to test not specified\n");
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
	id = 0;

	n = (test_total > 0) ? test_total : UINT_MAX;

	while (n > 0) {
		if (ntest == 0 && id != 0) {
			error = algo->restart(&ctx, test_desc->params);
			if (error != 0) {
				printf("test: failed to restart %s test\n",
				    algo->name);
				break;
			}
		}

		nread = min(size, n);
		error = test_stdin_read(data, &nread);
		if (error != 0 || nread == 0)
			break;

		nread = nread * 8;
		nupd = miss(ntest, test_maxnbits);
		nupd = min(nupd, nread);

		if (nupd > 0) {
			error = algo->update(&ctx, data, nupd);
			if (error != 0) {
				printf("test: failed to update data for %s test (%d)\n",
				    algo->name, error);
				break;
			}
			ntest += nupd;
		}
		nupd = miss(ntest, test_maxnbits);
		if (nupd == 0) {
			error = algo->final(&ctx);
			if (error != 0) {
				printf("failed to finalize the test (%d)\n", error);
				break;
			}
			test_show_result(algo, &ctx, id + 1);
			ntest = 0;
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
			error = test_getsize(optarg, &test_maxnbits);
//			error = test_getuint(optarg, &test_maxnbits);
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
