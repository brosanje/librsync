/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * librsync -- the library for network deltas
 *
 * Copyright (C) 2000, 2001 by Martin Pool <mbp@sourcefrog.net>
 * Copyright (C) 2002, 2003 by Donovan Baarda <abo@minkirri.apana.org.au>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __LIBRSYNC_MDFOUR_H_
#define __LIBRSYNC_MDFOUR_H_

#include <stdlib.h>
#include "types.h"

/** \private
  * Internal state while computing an MD4 hash.
  */
struct rs_mdfour {
    unsigned int        A, B, C, D;
#if HAVE_UINT64
    uint64_t            totalN;
#else
    uint32_t            totalN_hi, totalN_lo;
#endif
    int                 tail_len;
    unsigned char       tail[64];
};

/** \typedef struct rs_mdfour rs_mdfour_t
 *
 * \brief MD4 message-digest accumulator.
 *
 * \sa rs_mdfour(), rs_mdfour_begin(), rs_mdfour_update(),
 * rs_mdfour_result()
 */
typedef struct rs_mdfour rs_mdfour_t;

#ifdef __cplusplus
extern "C" {
#endif

void rs_mdfour(unsigned char *out, void const *in, size_t);
void rs_mdfour_begin(/* @out@ */ rs_mdfour_t * md);

/**
 * Feed some data into the MD4 accumulator.
 *
 * \param md  MD4 accumulator.
 * \param in_void Data to add.
 * \param n   Number of bytes fed in.
 */
void rs_mdfour_update(rs_mdfour_t * md, void const *in_void, size_t n);
void rs_mdfour_result(rs_mdfour_t * md, unsigned char *out);

#ifdef __cplusplus
}
#endif

#endif // __LIBRSYNC_MDFOUR_H_

/* vim: expandtab shiftwidth=4
 */
