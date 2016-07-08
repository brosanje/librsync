/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * librsync -- the library for network deltas
 * 
 * Copyright (C) 2001 by Martin Pool <mbp@sourcefrog.net>
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

#ifndef __LIBRSYNC_WHOLE_H_
#define __LIBRSYNC_WHOLE_H_

#include "librsync.h"

#ifdef __cplusplus
extern "C" {
#endif

rs_result rs_whole_run(rs_job_t *job, FILE *in_file, FILE *out_file);

#ifdef __cplusplus
}
#endif

#endif /* __LIBRSYNC_WHOLE_H_ */

/* vim: expandtab shiftwidth=4
 */
