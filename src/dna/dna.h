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

#ifndef __DNA_H__
#define	__DNA_H__

struct dna_params {
	double	alpha;	/* significance level for H0 */
};

/* The number of possible 10 DNA letters words possible, 2 ^ 20 */
#define	DNA_WORDS		1048576	

/* The length of the DNA word */
#define	DNA_WORDLEN		10

/* Total number of keystrokes to finalize the test, 2^21 + 9 */
#define	DNA_LETTERS		(2 * DNA_WORDS + 9)

/* Number of bits to finalize the DNA test */
#define	DNA_BITS		(DNA_LETTERS * 32)

#define	DNA_MIN_BITS		DNA_BITS	/* minimum nuber of bits */
#define	DNA_MAX_BITS		DNA_BITS	/* maximum number of bits */

/* Letters encoding, not used */
#define	DNA_C			0x00
#define	DNA_G			0x01
#define	DNA_A			0x02
#define	DNA_T			0x03
#define	DNA_L_MASK		0x03

#define	DNA_WORD_MASK		0x000fffff	/* mask for bits in stroke */
#define	DNA_LETTER_MASK		0x00000003	/* mask bits for letter */

TRAS_DECLARE_ALGO(dna);

#endif


