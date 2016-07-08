/*                    -*- c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* librsync-config.h.  Generated from librsync-config.h.cmake by cmake.  */

/*
 * librsync -- library for network deltas
 * 
 * Copyright (C) 2000, 2001 by Martin Pool <mbp@sourcefrog.net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/** \file librsync-config.h
 *
 * \brief System-specific configuration for librsync.h.
 */

#ifndef __LIBRSYNC_CONFIG_H
#define __LIBRSYNC_CONFIG_H

/**
 * \brief A long integer type that can handle the largest file
 * offsets.
 *
 * Perhaps this might have to be configured to be 'long long', 'long',
 * or something else depending on the platform.  On WIN32, many config.h's
 * define LONG_LONG as "__int64".
 */
typedef ${RS_LONG_T}    rs_long_t;

#ifdef _MSC_VER
// for windows
// export symbols in dll
#define EXPORTABLE __declspec(dllexport)
#elif __GNUC__
// gcc - also put -fvisibility=hidden
// export symbols in so
//#define EXPORTABLE __attribute__ ((dllexport))
#define EXPORTABLE
#endif

// compiler_warning.h
#define STRINGISE_IMPL(x) #x
#define STRINGISE(x) STRINGISE_IMPL(x)

// Use: #pragma message WARN("My message")
#if _MSC_VER
#   define FILE_LINE_LINK __FILE__ "(" STRINGISE(__LINE__) ") : "
#   define WARN(exp) (FILE_LINE_LINK "WARNING: " exp)
#else//__GNUC__ - may need other defines for different compilers
#   define WARN(exp) ("WARNING: " exp)
#endif

#ifndef __cplusplus
#define inline __inline
#endif

/* snprintf should not be defined as macro with MSC_VER >= 1900 */
#if defined(_WIN32) || defined(WIN32)
#  if defined(_MSC_VER)  /* MS compiller */
#    if (_MSC_VER < 1900)  /* snprintf not introduced */
#      if !defined(snprintf)
#        define snprintf _snprintf
#        define HAVE_SNPRINTF 1 /* snprintf defined manually */
#      endif
#    else
#      define HAVE_SNPRINTF 1 /* snprintf available via sdk */
#    endif
#  endif
#endif

#endif /* __LIBRSYNC_CONFIG_H */

/* vim: expandtab shiftwidth=4 tabstop=4 sts=4
 */
