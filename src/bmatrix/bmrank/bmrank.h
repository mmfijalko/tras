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

#ifndef __BMRANK_H__
#define	__BMRANK_H__

/*
 * The parameters for the Binary Matrix Rank Test.
 */
struct bmrank_params {
	int		uniform;/* take always full 32-bits words */
	unsigned int	m;	/* the number of rows for matrices */
	unsigned int	q;	/* the number of columns for matrices */
	unsigned int	nr;	/* the number of ranks for chi-2 */
	unsigned int	s0;	/* the start bit in the 32-bits word */
	unsigned int	N;	/* the mininum number of matrices */
	double		alpha;	/* the significance level for H0 */
};

/*
 * The minimum number of rows for each matrix, not defined yet.
 */
#define	BMRANK_MIN_M	16	/* XXX: temporary definition */

#define	BMRANK_MAX_M	32	/* XXX: temporary definition */

/*
 * The minimum number of columns for each matrix, not defined yet.
 */
#define	BMRANK_MIN_Q	16	/* XXX: temporary definition */

#define	BMRANK_MAX_Q	32	/* XXX: temporary definition */

/*
 * The minimum number of matrices to process for valid statistics.
 */
#define	BMRANK_MIN_MATRICES	38	/* min number of matrices */

#define	BMRANK_MAX_MATRICES	0	/* no restriction */

TRAS_DECLARE_ALGO(bmrank);

#endif

