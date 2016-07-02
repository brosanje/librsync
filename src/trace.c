/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
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

                                     /*
                                      | Finality is death.
                                      | Perfection is finality.
                                      | Nothing is perfect.
                                      | There are lumps in it.
                                      */



/*
 * TODO: Have a bit set in the log level that says not to include the
 * function name.
 */

#include "trace.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#define STDERR_FILENO 2
#endif
#include <stdio.h>
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

#include "util.h"

#ifndef WIN32
#include <sys/time.h>
#else
#include <Windows.h>
//typedef struct timeval {
//    long tv_sec;
//    long tv_usec;
//};

/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = ((unsigned __int64) 116444736000000000ULL);

/*
* timezone information is stored outside the kernel so tzp isn't used anymore.
*
* Note: this function is not for Win32 high precision timing purpose. See
* elapsed_time().
*/
int
gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    FILETIME    file_time;
    SYSTEMTIME  system_time;
    ULARGE_INTEGER ularge;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;

    tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);

    return 0;
}
#endif

rs_trace_fn_t  *rs_trace_impl = rs_trace_stderr;

int rs_trace_level = RS_LOG_INFO;

#ifdef HAVE_PROGRAM_INVOCATION_NAME
#  define MY_NAME program_invocation_short_name
#else
#  define MY_NAME "librsync"
#endif

static char* rs_timestamp(char* buffer, int bufsz);
static void  rs_log_va(int level, char const *fn, char const *fmt, va_list va);

#if SIZEOF_SIZE_T > SIZEOF_LONG
#  warning size_t is larger than a long integer, values in trace messages may be wrong
#endif


/**
 * Log severity strings, if any.  Must match ordering in
 * ::rs_loglevel.
 */
static const char *rs_severities[] = {
    "EMERGENCY!"
    , "ALERT!"
    , "CRITICAL!"
    , "ERROR:"
    , "Warning:"
    , "Notice:"
    , "Info:"
    , "Debug:"
    , "Trace:"
};


int
EXPORTABLE rs_supports_trace(void)
{
#ifdef DO_RS_TRACE
    return 1;
#else
    return 0;
#endif                                /* !DO_RS_TRACE */
}

/**
 * \brief Set the destination of trace information.
 *
 * The callback scheme allows for use within applications that may
 * have their own particular ways of reporting errors: log files for a
 * web server, perhaps, and an error dialog for a browser.
 *
 * \todo Do we really need such fine-grained control, or just yes/no
 * tracing?
 */
void
EXPORTABLE rs_trace_to(rs_trace_fn_t * new_impl)
{
    rs_trace_impl = new_impl;
}


void
EXPORTABLE rs_trace_set_level(rs_loglevel level)
{
    rs_trace_level = level;
}


/**
 * Called by a macro, used on platforms where we can't determine the
 * calling function name.
 */
void
EXPORTABLE rs_log0_nofn(int level, char const *fmt, ...)
{
    va_list         va;

    va_start(va, fmt);
    rs_log_va(level, PACKAGE, fmt, va);
    va_end(va);
}


/* Called by a macro that prepends the calling function name,
 * etc.  */
void
EXPORTABLE rs_log0(int level, char const *fn, char const *fmt, ...)
{
    va_list         va;

    va_start(va, fmt);
    rs_log_va(level, fn, fmt, va);
    va_end(va);
}


void
EXPORTABLE rs_trace_stderr(rs_loglevel UNUSED(level), char const *msg)
{
    /* NOTE NO TRAILING NUL */
#ifdef WIN32
    fputs(msg, stderr);
#else
    write(STDERR_FILENO, msg, strlen(msg));
#endif
}


/* This is called directly if the machine doesn't allow varargs
 * macros. */
void
EXPORTABLE rs_fatal0(char const *s, ...)
{
    va_list        va;

    va_start(va, s);
    rs_log_va(RS_LOG_CRIT, PACKAGE, s, va);
    va_end(va);
    abort();
}

/* This is called directly if the machine doesn't allow varargs
* macros. */
void
EXPORTABLE rs_debug0(char const *s, ...)
{
    va_list        va;

    va_start(va, s);
    rs_log_va(RS_LOG_DEBUG, PACKAGE, s, va);
    va_end(va);
}

/* This is called directly if the machine doesn't allow varargs
 * macros. */
void
EXPORTABLE rs_error0(char const *s, ...)
{
    va_list        va;

    va_start(va, s);
    rs_log_va(RS_LOG_ERR, PACKAGE, s, va);
    va_end(va);
}


/* This is called directly if the machine doesn't allow varargs
 * macros. */
void
EXPORTABLE rs_trace0(char const *s, ...)
{
#ifdef DO_RS_TRACE
    va_list        va;

    va_start(va, s);
    rs_log_va(RS_LOG_TRACE, PACKAGE, s, va);
    va_end(va);
#endif /* !DO_RS_TRACE */
}


static void
rs_log_va(int flags, char const *fn, char const *fmt, va_list va)
{
    int level = flags & RS_LOG_PRIMASK;
    
    if (rs_trace_impl && level <= rs_trace_level) {
        char            buf[1000];
        char            full_buf[1000];
        char timestamp[80];
        rs_timestamp(timestamp, sizeof(timestamp));

        vsnprintf(buf, sizeof buf - 1, fmt, va);

        if (flags & RS_LOG_NONAME) {
            snprintf(full_buf, sizeof full_buf - 1,
                     "%s %10s %s\n", timestamp, rs_severities[level], buf);
        } else {
            snprintf(full_buf, sizeof full_buf - 1,
                     "%s %10s [%s/%s] %s\n", timestamp, rs_severities[level]
                     , MY_NAME, fn, buf);
        }

        rs_trace_impl(level, full_buf);
    }
}


static char*
rs_timestamp(char* buffer, int bufsz) {
    //int millisec;
    struct timeval tmnow;
    struct tm *info;
    char msec_buf[10];
    static char mybuffer[80];
    time_t long_time;
    if (NULL == buffer) {
        buffer = mybuffer;
        bufsz = sizeof(mybuffer);
    }

    gettimeofday(&tmnow, NULL);
    
    time(&long_time);
    info = localtime(&long_time);
    strftime(buffer, bufsz, "%Y-%m-%d %H:%M:%S", info);
    sprintf(msec_buf, ".%03d", ((int)tmnow.tv_usec)/1000);
    strcat(buffer, msec_buf);
    return buffer;
}


/* vim: expandtab shiftwidth=4
 */
