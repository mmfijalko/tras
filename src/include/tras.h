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

#ifndef __TRAS_H__
#define	__TRAS_H__

struct tras_algo;

/*
 * Test results context.
 */
struct tras_result {
	int			status;		/* true if test passed */
	unsigned int		discard;	/* number of bits discarded */
	double			pvalue1;	/* P-value #1 */
	double			pvalue2;	/* P-value #2 */
};

#define	TRAS_TEST_PASSED	1		/* test passed */
#define	TRAS_TEST_FAILED	0		/* test failed */


/*
 * Generic structure for test context.
 */
struct tras_ctx {
	int			state;	/* state of test */
	void *			context;/* private test context */
	struct tras_result	result;	/* to keep results of test */
	const struct tras_algo*	algo;	/* the test description */
};

#define TRAS_STATE_NONE		0	/* state before initialization */
#define TRAS_STATE_INIT		1	/* state when correctly inited */
#define TRAS_STATE_FINAL	2	/* state when test finalized */
#define TRAS_STATE_ERROR	3	/* error state unrecover */

/*
 * Algorithm methods definition.
 */
typedef int (tras_test_init_t)(struct tras_ctx *, void *);
typedef int (tras_test_test_t)(struct tras_ctx *, void *, unsigned int);
typedef int (tras_test_update_t)(struct tras_ctx *, void *, unsigned int);
typedef int (tras_test_final_t)(struct tras_ctx *);
typedef int (tras_test_restart_t)(struct tras_ctx *, void *);
typedef int (tras_test_free_t)(struct tras_ctx *);

/*
 * Version structure for algorithms.
 */
struct tras_version {
	int	major;				/* major number */
	int	minor;				/* minor number */
	int	revision;			/* revision number */
};

/*
 * The structure to describe statistical test algorithm.
 */
struct tras_algo {
	const char *		name;		/* algorithm short name */
	const char *		desc;		/* algorithm description */
	int			id;		/* for internal selection */
	struct tras_version	version;	/* algorithm version */
	tras_test_init_t *	init;		/* initialize method */
	tras_test_test_t *	test;		/* test and final method */
	tras_test_update_t *	update;		/* update data method */
	tras_test_final_t *	final;		/* finalize method */
	tras_test_restart_t *	restart;	/* restar test method */
	tras_test_free_t *	free;		/* free memory method */
};

#define TRAS_DEFINE_ALGO(pref, name, desc, parent, mj, mn, b)	\
	const struct tras_algo pref##_algo = {			\
		.name =	name;					\
		.desc = desc;					\
		.version = { mj, mn, b };			\
		.init =	&pref##_init;				\
		.update = &pref##_update;			\
		.test = &pref##_test;				\
		.final = &pref##_final;				\
		.restart = &pref##_restart;			\
		.free = &pref##_free;				\
	}

#define TRAS_DECLARE_ALGO(pref)					\
	extern const struct tras_algo pref##_algo;		\
								\
tras_test_init_t pref##_init;					\
tras_test_test_t pref##_test;					\
tras_test_update_t pref##_update;				\
tras_test_final_t pref##_final;					\
tras_test_restart_t pref##_restart;				\
tras_test_free_t pref##_free;

void tras_ctx_init(struct tras_ctx *);
void tras_ctx_free(struct tras_ctx *);

int tras_test_init(struct tras_ctx *, const struct tras_algo *, size_t);
int tras_test_update(struct tras_ctx *, void *, unsigned int);
int tras_test_test(struct tras_ctx *, void *, unsigned int);
int tras_test_final(struct tras_ctx *);
int tras_test_restart(struct tras_ctx *, void *);
int tras_test_free(struct tras_ctx *);

int tras_do_free(struct tras_ctx *);
int tras_do_restart(struct tras_ctx *, void *);
int tras_do_test(struct tras_ctx *, void *, unsigned int);

#endif
