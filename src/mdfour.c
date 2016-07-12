/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * librsync -- the library for network deltas
 *
 * Copyright (C) 2000, 2001 by Martin Pool <mbp@sourcefrog.net>
 * Copyright (C) 1997-1999 by Andrew Tridgell
 * Copyright (C) 2002, 2003 by Donovan Baarda <abo@minkirri.apana.org.au>
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

/* MD4 message digest algorithm.
 *
 * TODO: Perhaps use the MD4 routine from OpenSSL if it's installed.
 * It's probably not worth the trouble.
 *
 * This was originally written by Andrew Tridgell for use in Samba.
 * It was then modified by;
 *
 * 2002-06-xx: Robert Weber <robert.weber@Colorado.edu>
 *   optimisations and fixed >512M support.
 *
 * 2002-06-27: Donovan Baarda <abo@minkirri.apana.org.au>
 *   further optimisations and cleanups.
 *
 * 2004-09-09: Simon Law  <sfllaw@debian.org>
 *   handle little-endian machines that can't do unaligned access
 *   (e.g. ia64, pa-risc).
 *
 * 2016-07-08: Bro Sanje <bro.sanje@gmail.com>
 *   replaced with code from 
 *   http://openwall.info/wiki/people/solar/software/public-domain-source-code/md4
 *   and adapted to librsync, with Robert Weber's optimizations.
 */

/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD4 Message-Digest Algorithm (RFC 1320).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md4
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * (This is a heavily cut-down "BSD license".)
 *
 * This differs from Colin Plumb's older public domain implementation in that
 * no exactly 32-bit integer data type is required (any 32-bit or wider
 * unsigned integer data type will do), there's no compile-time endianness
 * configuration, and the function prototypes match OpenSSL's.  No code from
 * Colin Plumb's implementation has been reused; this comment merely compares
 * the properties of the two independent implementations.
 *
 * The primary goals of this implementation are portability and ease of use.
 * It is meant to be fast, but not as fast as possible.  Some known
 * optimizations are not included to reduce source code size and avoid
 * compile-time configuration.
 */

#include <string.h>
#include <stdio.h>

#include "mdfour.h"
//#include "librsync-config.h"

#ifdef HAVE_OPENSSL
//#pragma message WARN("Using OpenSSL")

#include <openssl/md4.h>

#else

void MD4_Init(MD4_CTX *ctx);
void MD4_Update(MD4_CTX *ctx, const void *data, unsigned long size);
void MD4_Final(unsigned char *result, MD4_CTX *ctx);

#endif

void rs_mdfour(unsigned char *out, void const *in, unsigned long inlen)
{
#ifdef HAVE_OPENSSL
    MD4(in, inlen, out);
#else
    MD4_CTX ctx;
    MD4_Init(&ctx);
    MD4_Update(&ctx, in, inlen);
    MD4_Final(out, &ctx);
#endif
}

void rs_mdfour_begin(rs_mdfour_t * md)
{
    MD4_Init(md);
}

void rs_mdfour_update(rs_mdfour_t * md, void const *in_void, unsigned long n)
{
    MD4_Update(md, in_void, n);
}

void rs_mdfour_result(rs_mdfour_t * md, unsigned char *out)
{
    MD4_Final(out, md);
}

#ifndef HAVE_OPENSSL

//#pragma message WARN("No OpenSSL - providing substitution MD4")

/*
 * The basic MD4 functions.
 *
 * F and G are optimized compared to their RFC 1320 definitions, with the
 * optimization for F borrowed from Colin Plumb's MD5 implementation.
 */
#define F(x, y, z)            ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)            (((x) & ((y) | (z))) | ((y) & (z)))
#define H(x, y, z)            ((x) ^ (y) ^ (z))

/*
 * The MD4 transformation for all three rounds.
 */
#define STEP(f, a, b, c, d, x, s) \
    (a) += f((b), (c), (d)) + (x); \
    (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));

/*
 * SET reads 4 input bytes in little-endian byte order and stores them in a
 * properly aligned word in host byte order.
 *
 * The check for little-endian architectures that tolerate unaligned memory
 * accesses is just an optimization.  Nothing will break if it fails to detect
 * a suitable architecture.
 *
 * Unfortunately, this optimization may be a C strict aliasing rules violation
 * if the caller's data buffer has effective type that cannot be aliased by
 * MD4_u32plus.  In practice, this problem may occur if these MD4 routines are
 * inlined into a calling function, or with future and dangerously advanced
 * link-time optimizations.  For the time being, keeping these MD4 routines in
 * their own translation unit avoids the problem.
 */
#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
#define SET(n) \
    (*(MD4_u32plus *)&ptr[(n) * 4])
#define GET(n) \
    SET(n)
#else
#define SET(n) \
    (ctx->block[(n)] = \
    (MD4_u32plus)ptr[(n) * 4] | \
    ((MD4_u32plus)ptr[(n) * 4 + 1] << 8) | \
    ((MD4_u32plus)ptr[(n) * 4 + 2] << 16) | \
    ((MD4_u32plus)ptr[(n) * 4 + 3] << 24))
#define GET(n) \
    (ctx->block[(n)])
#endif

/*
 * This processes one or more 64-byte data blocks, but does NOT update the bit
 * counters.  There are no alignment requirements.
 */
static const void *md4cruncher(MD4_CTX *ctx, const void *data, unsigned long size)
{
    const unsigned char *ptr;
    MD4_u32plus a, b, c, d;
    MD4_u32plus saved_a, saved_b, saved_c, saved_d;
    const MD4_u32plus ac1 = 0x5a827999, ac2 = 0x6ed9eba1;

    ptr = (const unsigned char *)data;

    a = ctx->a;
    b = ctx->b;
    c = ctx->c;
    d = ctx->d;

    do {
        saved_a = a;
        saved_b = b;
        saved_c = c;
        saved_d = d;

/* Round 1 */
        STEP(F, a, b, c, d, SET(0), 3)
        STEP(F, d, a, b, c, SET(1), 7)
        STEP(F, c, d, a, b, SET(2), 11)
        STEP(F, b, c, d, a, SET(3), 19)
        STEP(F, a, b, c, d, SET(4), 3)
        STEP(F, d, a, b, c, SET(5), 7)
        STEP(F, c, d, a, b, SET(6), 11)
        STEP(F, b, c, d, a, SET(7), 19)
        STEP(F, a, b, c, d, SET(8), 3)
        STEP(F, d, a, b, c, SET(9), 7)
        STEP(F, c, d, a, b, SET(10), 11)
        STEP(F, b, c, d, a, SET(11), 19)
        STEP(F, a, b, c, d, SET(12), 3)
        STEP(F, d, a, b, c, SET(13), 7)
        STEP(F, c, d, a, b, SET(14), 11)
        STEP(F, b, c, d, a, SET(15), 19)

/* Round 2 */
        STEP(G, a, b, c, d, GET(0) + ac1, 3)
        STEP(G, d, a, b, c, GET(4) + ac1, 5)
        STEP(G, c, d, a, b, GET(8) + ac1, 9)
        STEP(G, b, c, d, a, GET(12) + ac1, 13)
        STEP(G, a, b, c, d, GET(1) + ac1, 3)
        STEP(G, d, a, b, c, GET(5) + ac1, 5)
        STEP(G, c, d, a, b, GET(9) + ac1, 9)
        STEP(G, b, c, d, a, GET(13) + ac1, 13)
        STEP(G, a, b, c, d, GET(2) + ac1, 3)
        STEP(G, d, a, b, c, GET(6) + ac1, 5)
        STEP(G, c, d, a, b, GET(10) + ac1, 9)
        STEP(G, b, c, d, a, GET(14) + ac1, 13)
        STEP(G, a, b, c, d, GET(3) + ac1, 3)
        STEP(G, d, a, b, c, GET(7) + ac1, 5)
        STEP(G, c, d, a, b, GET(11) + ac1, 9)
        STEP(G, b, c, d, a, GET(15) + ac1, 13)

/* Round 3 */
        STEP(H, a, b, c, d, GET(0) + ac2, 3)
        STEP(H, d, a, b, c, GET(8) + ac2, 9)
        STEP(H, c, d, a, b, GET(4) + ac2, 11)
        STEP(H, b, c, d, a, GET(12) + ac2, 15)
        STEP(H, a, b, c, d, GET(2) + ac2, 3)
        STEP(H, d, a, b, c, GET(10) + ac2, 9)
        STEP(H, c, d, a, b, GET(6) + ac2, 11)
        STEP(H, b, c, d, a, GET(14) + ac2, 15)
        STEP(H, a, b, c, d, GET(1) + ac2, 3)
        STEP(H, d, a, b, c, GET(9) + ac2, 9)
        STEP(H, c, d, a, b, GET(5) + ac2, 11)
        STEP(H, b, c, d, a, GET(13) + ac2, 15)
        STEP(H, a, b, c, d, GET(3) + ac2, 3)
        STEP(H, d, a, b, c, GET(11) + ac2, 9)
        STEP(H, c, d, a, b, GET(7) + ac2, 11)
        STEP(H, b, c, d, a, GET(15) + ac2, 15)

        a += saved_a;
        b += saved_b;
        c += saved_c;
        d += saved_d;

        ptr += 64;
    } while (size -= 64);

    ctx->a = a;
    ctx->b = b;
    ctx->c = c;
    ctx->d = d;

    return ptr;
}

void MD4_Init(MD4_CTX *ctx)
{
    ctx->a = 0x67452301;
    ctx->b = 0xefcdab89;
    ctx->c = 0x98badcfe;
    ctx->d = 0x10325476;

#ifdef HAVE_UINT64
    ctx->totalN = 0;
#else
    ctx->totalN_lo = 0;
    ctx->totalN_hi = 0;
#endif
}

void MD4_Update(MD4_CTX *ctx, const void *data, unsigned long size)
{
    unsigned long used, available;

#ifdef HAVE_UINT64
    used = ctx->totalN & 0x3f;
    ctx->totalN += size;
#else
    MD4_u32plus saved_lo = ctx->totalN_lo;
    if ((ctx->totalN_lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
        ctx->totalN_hi++;
    ctx->totalN_hi += size >> 29;

    used = saved_lo & 0x3f;
#endif

    if (used) {
        available = 64 - used;

        if (size < available) {
            memcpy(&ctx->buffer[used], data, size);
            return;
        }

        memcpy(&ctx->buffer[used], data, available);
        data = (const unsigned char *)data + available;
        size -= available;
        md4cruncher(ctx, ctx->buffer, 64);
    }

    if (size >= 64) {
        data = md4cruncher(ctx, data, size & ~(unsigned long)0x3f);
        size &= 0x3f;
    }

    memcpy(ctx->buffer, data, size);
}

/**
 * These next routines are necessary because MD4 is specified in terms of
 * little-endian int32s, but we have a byte buffer.  On little-endian
 * platforms, I think we can just use the buffer pointer directly.
 *
 * There are some nice endianness routines in glib, including assembler
 * variants. If we ever depended on glib, then it could be good to use them
 * instead. */

/* We need this if there is a uint64 */
/* --robert.weber@Colorado.edu             */
#ifdef HAVE_UINT64
inline static void
OUT8( /* @out@ */ unsigned char *dst, uint64_t src)
{
  for (int ii = 0; 8 > ii; ii++, src >>= 8) *dst++ = (unsigned char) (src & 0xff);
}
#endif /* HAVE_UINT64 */

#define OUT4(dst, src) \
    (dst)[0] = (unsigned char)(src); \
    (dst)[1] = (unsigned char)((src) >> 8); \
    (dst)[2] = (unsigned char)((src) >> 16); \
    (dst)[3] = (unsigned char)((src) >> 24);

void MD4_Final(unsigned char *result, MD4_CTX *ctx)
{
    unsigned long used, available;

#ifdef HAVE_UINT64
    used = ctx->totalN & 0x3f;
#else
    used = ctx->totalN_lo & 0x3f;
#endif

    ctx->buffer[used++] = 0x80;

    available = 64 - used;

    if (available < 8) {
        memset(&ctx->buffer[used], 0, available);
        md4cruncher(ctx, ctx->buffer, 64);
        used = 0;
        available = 64;
    }

    memset(&ctx->buffer[used], 0, available - 8);

#ifdef HAVE_UINT64
    ctx->totalN <<= 3;
    OUT8(&ctx->buffer[56], ctx->totalN);
#else
    ctx->totalN_lo <<= 3;
    OUT4(&ctx->buffer[56], ctx->totalN_lo);
    OUT4(&ctx->buffer[60], ctx->totalN_hi);
#endif

    md4cruncher(ctx, ctx->buffer, 64);

    OUT4(&result[0], ctx->a)
    OUT4(&result[4], ctx->b)
    OUT4(&result[8], ctx->c)
    OUT4(&result[12], ctx->d)

    memset(ctx, 0, sizeof(*ctx));
}

#endif /* ! HAVE_OPENSSL */

/* vim: expandtab shiftwidth=4 tabstop=4 sts=4
 */
