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

#ifndef	__SEQUENCES_H__
#define	__SEQUENCES_H__

#define	offs_to8(s, o)	(((uint8_t *)(s))[(o) >> 3])
#define	offs_to16(s, o)	(((uint16_t *)(s))[(o) >> 4])
#define	offs_to32(s, o)	(((uint32_t *)(s))[(o) >> 5])

#define	seq_get8(s, o)	((offs_to8(s, o) << ((o) & 0x07)) |	\
	(((o) & 0x07) ? offs_to8(s, (o) + 8) >> (8 - ((o) & 0x07)) : 0))
#define	seq_get16(s, o)	((offs_to16(s, o) << ((o) & 0x0f)) |	\
	(((o) & 0x0f) ? offs_to16(s, (o) + 16) >> (16 - ((o) & 0x0f)) : 0))
#define	seq_get32(s, o)	((offs_to32(s, o) << ((o) & 0x1f)) |	\
	(((o) & 0x1f) ? offs_to32(s, (o) + 32) >> (32 - ((o) & 0x1f)) : 0))

#endif

