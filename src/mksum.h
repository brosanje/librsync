/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * librsync -- the library for network deltas
 * 
 * Copyright (C) 2000, 2001 by Martin Pool <mbp@sourcefrog.net>
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

#ifndef __LIBRSYNC_MKSUM_H_
#define __LIBRSYNC_MKSUM_H_

#include "librsync.h"

/**
 * \brief Start generating a signature.
 *
 * \return A new rs_job_t into which the old file data can be passed.
 *
 * \param sig_magic Indicates the version of signature file format to generate.
 * See ::rs_magic_number.
 *
 * \param new_block_len Size of checksum blocks.  Larger values make the
 * signature shorter, and the delta longer.
 *
 * \param strong_sum_len If non-zero, truncate the strong signatures to this
 * many bytes, to make the signature shorter.  It's recommended you leave this
 * at zero to get the full strength.
 *
 * \sa rs_sig_file()
 */
rs_job_t *rs_sig_begin(size_t new_block_len,
                       size_t strong_sum_len,
                       rs_magic_number sig_magic);

#endif /* !__LIBRSYNC_MKSUM_H_ */

/* vim: expandtab shiftwidth=4 tabstop=4 sts=4
*/
