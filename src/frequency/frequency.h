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

#ifndef __TRAS_FREQUENCY_H__
#define	__TRAS_FREQUENCY_H__

/*
 * Parameter structure for the frequency test, only alpha.
 */
struct frequency_params {
	double	alpha;	/* significance level for H0 */
};

#define	FREQUENCY_MIN_BITS	100	/* minimum number of bits */
#define	FREQUENCY_MAX_BITS	0	/* no maximum number of bits */

/*
 * FIPS 140-1 and 140-2 have constant bits sequence length.
 */
#define	FREQUENCY_FIPS_MIN_BITS	20000	/* for FIPS 140-1,2 */
#define	FREQUENCY_FIPS_MAX_BITS	20000	/* for FIPS 140-1,2 */

/*
 * Min and max of frequency sums for FIPS 140-1.
 */
#define	FREQUENCY_FIPS_140_1_MIN_SUM	9654
#define	FREQUENCY_FIPS_140_1_MAX_SUM	10346

/*
 * Min and max of frequency sums for FIPS 140-2.
 */
#define	FREQUENCY_FIPS_140_2_MIN_SUM	9725
#define	FREQUENCY_FIPS_140_2_MAX_SUM	10275

TRAS_DECLARE_ALGO(frequency);

TRAS_DECLARE_ALGO(frequency_fips_140_1);

TRAS_DECLARE_ALGO(frequency_fips_140_2);

#endif
