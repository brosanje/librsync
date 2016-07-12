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
 * See md4.c for more information.
 */

#ifndef __LIBRSYNC_MDFOUR_H_
#define __LIBRSYNC_MDFOUR_H_

#include <sys/types.h>
#include "types.h"

#ifdef HAVE_OPENSSL
#include <openssl/md4.h>
#else

/* Any 32-bit or wider unsigned integer data type will do */
typedef uint32_t MD4_u32plus;

/*! structure holding the MD4 calculations
 * The caller needs to allocate this storage, otherwise it would be a private structure.
 */
typedef struct {
#ifdef HAVE_UINT64
    uint64_t totalN;    /*!< byte count, put at the end of the trailing padding block */
#else
    MD4_u32plus totalN_lo; /*!< lo 32 of byte count, put at the end of the trailing padding block */
    MD4_u32plus totalN_hi; /*!< hi 32 of byte count, put at the end of the trailing padding block */
#endif

    MD4_u32plus a, b, c, d; /*!< the accumulators */

    /*! data is processed in chunks of 64.  this is a holding area for extra bytes.
     * The amount of this buffer in use is tracked in the totals above.
     */
    unsigned char buffer[64];

#if !defined(__i386__) && !defined(__x86_64__) && !defined(__vax__)
    /*! staging area to reorder bytes to convert input into little endian */
    MD4_u32plus block[16];
#endif
} MD4_CTX;

#endif

typedef MD4_CTX rs_mdfour_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void rs_mdfour(unsigned char *out, void const *in, unsigned long inlen);
extern void rs_mdfour_begin(/* @out@ */ rs_mdfour_t * md);

/**
 * Feed some data into the MD4 accumulator.
 *
 * \param md  MD4 accumulator.
 * \param in_void Data to add.
 * \param n   Number of bytes fed in.
 */
extern void rs_mdfour_update(rs_mdfour_t * md, void const *in_void, unsigned long n);
extern void rs_mdfour_result(rs_mdfour_t * md, unsigned char *out);

#ifdef __cplusplus
}
#endif

#endif /* ! __LIBRSYNC_MDFOUR_H_ */

/* vim: expandtab shiftwidth=4 tabstop=4 sts=4
 */
