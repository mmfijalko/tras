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
 * The Parking Lot Test definitions.
 */

#ifndef __TRAS_PLOT_H__
#define	__TRAS_PLOT_H__

/*
 * Notice: no parameters for the parking lot test.
 */

/*
 * Notice: since we don't have well-defined alpha parameter (no decision
 * where to place it) we need to add parameter structure for this test.
 */
struct plot_params {
	double	alpha;	/* significance level for H0 */
	int	idist;	/* distance function identifier */
};

#define	PARKING_LOT_IDIST_EUCLIDEAN	1
#define	PARKING_LOT_IDIST_COORD_MIN	2
#define	PARKING_LOT_IDIST_COORD_MAX	3

/*
 * Minimum number of cars to be parked.
 */
#define	PARKING_LOT_MIN_CARS	12000

/*
 * Maximum number of cars to be parked.
 */
#define	PARKING_LOT_MAX_CARS	12000

/*
 * Minimum and maximum number of coordinations (x and y).
 */
#define	PARKING_LOT_MIN_COORD	(2 * PARKING_LOT_MIN_CARS)
#define	PARKING_LOT_MAX_COORD	(2 * PARKING_LOT_MAX_CARS)

/*
 * Minimum and maximum number of bits to simualate car parking.
 */
#define	PARKING_LOT_MIN_NBITS	(32 * PARKING_LOT_MIN_COORD)
#define	PARKING_LOT_MAX_NBITS	(32 * PARKING_LOT_MAX_COORD)

TRAS_DECLARE_ALGO(plot);

#endif
