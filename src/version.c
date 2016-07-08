/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * librsync
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

#include "librsync.h"

char const rs_librsync_version[] = (PACKAGE " " VERSION);
EXPORTABLE const char* rs_get_librsync_version() { return rs_librsync_version; }

char const rs_licence_string[] = "";
EXPORTABLE const char* rs_get_licence_string() { return rs_licence_string; }

/* vim: expandtab shiftwidth=4 tabstop=4 sts=4
 */
