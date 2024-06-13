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

/*
 * This is the good place to start temporary collecting knowledge of the
 * algorithms restrictions, how to use them and how to properly test
 * them according to the results provided by other suits (sts, dieharder).
 */

#include <regression.h>
#include <algos.h>

/*
 * The list of regression tests for the serial test. The m = <1, 24>
 */
const struct serial_params serial_params_1 = {
	.m = 1,
	.alpha = 0.01,
};

const struct serial_params serial_params_2 = {
	.m = 2,
	.alpha = 0.01,
};

struct regress_alg regsalg_serial_1 = {
	.trasalg = &serial_algo,
	.algparm = &serial_params_1,
};

struct regress_alg regsalg_serial_2 = {
	.trasalg = &serial_algo,
	.algparm = &serial_params_2,
};

struct regress_run serial_run_1 = {
	.uniform = 1,
	.maxbits = 1,
	.regsalg = &regsalg_serial_1,
};

/*
 * The list of regression test for the approximated entropy test. m = <1, 16>
 */
struct approxe_params approxe_params_1 = {
	.m = 1,
	.alpha = 0.01,
};

struct approxe_params approxe_params_2 = {
	.m = 2,
	.alpha = 0.01,
};

struct regress_alg regsalg_approxe_1 = {
	.trasalg = &approxe_algo,
	.algparm = &approxe_params_1,
};

/*
 * The list of regression test for the Frequency Test Within a Block Test.
 */
struct blkfreq_params blkfreq_params_1 = {
	.m = 20,
	.alpha = 0.01,
};

struct blkfreq_params blkfreq_params_2 = {
	.m = 128,
	.allpha = 0.01,
};

struct blkfreq_params blkfreq_params_3 = {
	.m = 512,
	.alpha = 0.01,
};

struct blkfreq_params blkfreq_params_4 = {
	.m = 1024,
	.alpha = 0.01,
};

/*
 * The list of regression test for the Binary Matrix Rank Test.
 */
struct bmatrix_params bmatrix_params_1 = {
	.m = 16,
	.q = 16,
	.alpha = 0.01,
};

struct bmatrix_params bmatrix_params_2 = {
	.m = 16,
	.q = 24,
	.alpha = 0.01,
};

struct bmatrix_params bmatrix_params_3 = {
	.m = 24,
	.q = 16,
	.alpha = 0.01,
};

struct bmatrix_params bmatrix_params_4 = {
	.m = 24,
	.q = 24,
	.alpha = 0.01,
};

struct bmatrix_params bmatrix_params_5 = {
	.m = 17,
	.q = 17,
	.alpha = 0.01,
};

struct bmatrix_params bmatrix_params_6 = {
	.m = 19,
	.q = 19,
	.alpha = 0.01,
};

struct bmatrix_params bmatrix_params_7 = {
	.m = 31,
	.q = 31,
	.alpha = 0.01,
};

struct bmatrix_params bmatrix_params_8 = {
	.m = 32,
	.q = 32,
	.alpha = 0.01,
};
