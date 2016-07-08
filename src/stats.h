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

#ifndef __LIBRSYNC_STATS_H_
#define __LIBRSYNC_STATS_H_

#include "librsync.h"

/**
 * \brief Return a human-readable representation of statistics.
 *
 * The string is truncated if it does not fit.  100 characters should
 * be sufficient space.
 *
 * \param stats Statistics from an encoding or decoding operation.
 *
 * \param buf Buffer to receive result.
 * \param size Size of buffer.
 * \return \p buf.
 * \see \ref api_stats
 */
char* rs_format_stats(rs_stats_t const * stats, char *buf, size_t size);

#endif /* !__LIBRSYNC_STATS_H_ */

/* vim: expandtab shiftwidth=4 tabstop=4 sts=4
*/
