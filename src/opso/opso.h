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

#ifndef __OPSO_H__
#define	__OPSO_H__

struct opso_params {
	double	alpha;	/* significance level for H0 */
};

/* The number of possible OPSO 2-letter words, 2^20 */
#define	OPSO_WORDS		1048576	

/* The length of the OPSO word, number of letters */
#define	OPSO_WORDLEN		2

/* Total number of keystrokes to finalize the test, 2^21 + 9 */
#define	OPSO_LETTERS		(2 * OPSO_WORDS + 1)

/* Number of bits to finalize the OPSO test */
#define	OPSO_BITS		(OPSO_LETTERS * 32)

#define	OPSO_MIN_BITS		OPSO_BITS	/* minimum nuber of bits */
#define	OPSO_MAX_BITS		OPSO_BITS	/* maximum number of bits */

#define	OPSO_WORD_MASK		0x000fffff	/* mask bits for word */
#define	OPSO_LETTER_MASK	0x000003ff	/* mask bits for letter */

TRAS_DECLARE_ALGO(opso);

#endif


