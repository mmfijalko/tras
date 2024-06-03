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

#ifndef __TRAS_NT_MATCH_H__
#define	__TRAS_NT_MATCH_H__

/*
 * Non-overlapping Template Matching Test definitions params.
 */
struct ntmatch_params {
	unsigned int	m;	/* length in bits of each template */
	unsigned int	M;	/* length of subsequence to be tested */
	unsigned int	N;	/* number of independent blocks */
	uint32_t	B;	/* the m-bits template to be matched */
	double		alpha;	/* the significance level for H0 */
};

#define	NTMATCH_MIN_M		2	/* the minimum length of template */
#define	NTMATCH_MAX_M		10	/* the maximum length of template */

#define	NTMATCH_MIN_N		100	/* minimum number of substrings */
#define	NTMATCH_MAX_N		0	/* maximum number of substrings */

#define	NTMATCH_MIN_SUBS_LEN	100	/* the minimum length of substrings */
#define	NTMATCH_MAX_SUBS_LEN	0	/* the maximum length of substrings */

TRAS_DECLARE_ALGO(ntmatch);

#endif
