/*=                    -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * librsync -- library for network deltas
 *
 * Copyright 2000, 2001, 2014, 2015 by Martin Pool <mbp@sourcefrog.net>
 * Copyright (C) 2003 by Donovan Baarda <abo@minkirri.apana.org.au>
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
                               | You should never wear your best
                               | trousers when you go out to fight for
                               | freedom and liberty.
                               |        -- Henrik Ibsen
                               */


/*!
 * \file librsync.h
 * \brief Public header for librsync.
 */

#ifndef __LIBRSYNC_LIBRSYNC_H_
#define __LIBRSYNC_LIBRSYNC_H_

#include <sys/types.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "librsync-config.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Library version string.
 * \see \ref versioning
 */
extern char const rs_librsync_version[];
EXPORTABLE const char* rs_get_librsync_version();

/** Summary of the licence for librsync.
 */
extern char const rs_licence_string[];
EXPORTABLE const char* rs_get_license_string();

typedef unsigned char rs_byte_t;



                          /*
                           | "The IETF already has more than enough
                           | RFCs that codify the obvious, make
                           | stupidity illegal, support truth,
                           | justice, and the IETF way, and generally
                           | demonstrate the author is a brilliant and
                           | valuable Contributor to The Standards
                           | Process."
                           |     -- Vernon Schryver
                           */


/**
 * A uint32 magic number, emitted in bigendian/network order at the start of
 * librsync files.
 **/
typedef enum {
    /**
     * A delta file.
     *
     * At present, there's only one delta format.
     *
     * The four-byte literal \c "rs\x026".
     **/
    RS_DELTA_MAGIC          = 0x72730236,

    /**
     * A signature file with MD4 signatures.
     *
     * Backward compatible with
     * librsync < 1.0, but strongly deprecated because it creates a security
     * vulnerability on files containing partly untrusted data. See
     * <https://github.com/librsync/librsync/issues/5>.
     *
     * The four-byte literal \c "rs\x016".
     *
     * \see rs_sig_begin()
     **/
    RS_MD4_SIG_MAGIC        = 0x72730136,

    /**
     * A signature file using the BLAKE2 hash. Supported from librsync 1.0.
     *
     * The four-byte literal \c "rs\x017".
     *
     * \see rs_sig_begin()
     **/
    RS_BLAKE2_SIG_MAGIC     = 0x72730137,

    RS_DEFAULT_SIG_MAGIC    = RS_MD4_SIG_MAGIC
} rs_magic_number;


/**
 * \brief Log severity levels.
 *
 * These are the same as syslog, at least in glibc.
 *
 * \sa rs_trace_set_level()
 * \sa \ref api_trace
 */
typedef enum {
    RS_LOG_EMERG         = 0,   /**< System is unusable */
    RS_LOG_ALERT         = 1,   /**< Action must be taken immediately */
    RS_LOG_CRIT          = 2,   /**< Critical conditions */
    RS_LOG_ERR           = 3,   /**< Error conditions */
    RS_LOG_WARNING       = 4,   /**< Warning conditions */
    RS_LOG_NOTICE        = 5,   /**< Normal but significant condition */
    RS_LOG_INFO          = 6,   /**< Informational */
    RS_LOG_DEBUG         = 7,   /**< Debug-level messages */
    RS_LOG_TRACE         = 8    /**< Trace-level messages */
} rs_loglevel;


/**
 * \typedef rs_trace_fn_t
 * \brief Callback to write out log messages.
 * \param level a syslog level.
 * \param msg message to be logged.
 * \sa \ref api_trace
 */
typedef void    rs_trace_fn_t(rs_loglevel level, char const *msg);

/**
 * Set the least important message severity that will be output.
 * \sa \ref api_trace
 */
EXPORTABLE void rs_trace_set_level(rs_loglevel level);

/** Set trace callback.
 * \sa \ref api_trace
 */
EXPORTABLE void rs_trace_to(rs_trace_fn_t *);

/** Default trace callback that writes to stderr.  Implements
 * ::rs_trace_fn_t, and may be passed to rs_trace_to().
 * \sa \ref api_trace
 */
EXPORTABLE void rs_trace_stderr(rs_loglevel level, char const *msg);

/** Check whether the library was compiled with debugging trace
 *
 * \returns True if the library contains trace code; otherwise false.
 * If this returns false, then trying to turn trace on will achieve
 * nothing.
 * \sa \ref api_trace
 */
EXPORTABLE int rs_supports_trace(void);

/**
 * \enum rs_result
 * \brief Return codes from nonblocking rsync operations.
 * \see rs_strerror()
 * \see api_callbacks
 */
typedef enum rs_result {
    RS_DONE =        0,    /**< Completed successfully. */
    RS_BLOCKED =    1,     /**< Blocked waiting for more data. */

    /** The job is still running, and not yet finished or blocked.
     * (This value should never be seen by the application.) */
    RS_RUNNING  =       2,

    RS_TEST_SKIPPED =   77,     /**< Test neither passed or failed. */

    RS_IO_ERROR =    100,    /**< Error in file or network IO. */
    RS_SYNTAX_ERROR =   101,    /**< Command line syntax error. */
    RS_MEM_ERROR =    102,    /**< Out of memory. */
    /** Unexpected end of input file, perhaps due to a truncated file
     * or dropped network connection. */
    RS_INPUT_ENDED =    103,
    RS_BAD_MAGIC =      104,    /**< Bad magic number at start of
                                   stream.  Probably not a librsync
                                   file, or possibly the wrong kind of
                                   file or from an incompatible
                                   library version. */
    RS_UNIMPLEMENTED =  105,    /**< Author is lazy. */
    RS_CORRUPT =        106,    /**< Unbelievable value in stream. */
    RS_INTERNAL_ERROR = 107,    /**< Probably a library bug. */
    RS_PARAM_ERROR =    108     /**< Bad value passed in to library,
                                 * probably an application bug. */
} rs_result;


/**
 * Return an English description of a ::rs_result value.
 */
EXPORTABLE char const *rs_strerror(rs_result r);


/**
 * \brief Performance statistics from a librsync encoding or decoding
 * operation.
 *
 * \sa api_stats
 * \sa rs_format_stats()
 * \sa rs_log_stats()
 */
typedef struct rs_stats {
    char const     *op;     /**< Human-readable name of current
                             * operation.  For example, "delta". */
    int             lit_cmds;   /**< Number of literal commands. */
    rs_long_t       lit_bytes;  /**< Number of literal bytes. */
    rs_long_t       lit_cmdbytes; /**< Number of bytes used in literal
                                   * command headers. */

    rs_long_t       copy_cmds, copy_bytes, copy_cmdbytes;
    rs_long_t       sig_cmds, sig_bytes;
    int             false_matches;

    rs_long_t       sig_blocks; /**< Number of blocks described by the
                                   signature. */

    size_t          block_len;

    rs_long_t       in_bytes;   /**< Total bytes read from input. */
    rs_long_t       out_bytes;  /**< Total bytes written to output. */

    time_t          start, end;
} rs_stats_t;

/**
 * Write statistics into the current log as text.
 *
 * \see \ref api_stats
 * \see \ref api_trace
 */
EXPORTABLE int rs_log_stats(rs_stats_t const *stats);

extern const int RS_MD4_SUM_LENGTH, RS_BLAKE2_SUM_LENGTH;

#define RS_MAX_STRONG_SUM_LENGTH 32

typedef unsigned int rs_weak_sum_t;
typedef unsigned char rs_strong_sum_t[RS_MAX_STRONG_SUM_LENGTH];

/*
 * TODO: These sumset structures are not terribly useful.  Perhaps we need a
 * splay tree or something that will let us smoothly grow as data is
 * read in.
 */


/**
 * \brief Description of the match described by a signature.
 */
typedef struct rs_target {
    unsigned short  t;
    int             i;
} rs_target_t;

typedef struct rs_block_sig rs_block_sig_t;

typedef struct rs_tag_table_entry {
    int l; // left bound of the hash tag in sorted array of targets
    int r; // right bound of the hash tag in sorted array of targets
    // all tags between l and r inclusively are the same
} rs_tag_table_entry_t ;

/*
 * This structure describes all the sums generated for an instance of
 * a file.  It incorporates some redundancy to make it easier to
 * search.
 */
struct rs_signature {
    rs_long_t       flength;        /* total file length */
    int             count;          /* how many chunks */
    int             remainder;      /* flength % block_length */
    int             block_len;      /* block_length */
    int             strong_sum_len;
    rs_block_sig_t  *block_sigs;    /* points to info for each chunk */
    rs_tag_table_entry_t        *tag_table;
    rs_target_t     *targets;
    int             magic;
};

/**
 * \typedef rs_signature_t
 */
typedef struct rs_signature rs_signature_t;


/*
 * All blocks are the same length in the current algorithm except for
 * the last block which may be short.
 */
struct rs_block_sig {
    int             i;            /* index of this chunk */
    rs_weak_sum_t   weak_sum;     /* simple checksum */
    rs_strong_sum_t strong_sum;   /* checksum  */
};


/**
 * Deep deallocation of checksums.
 */
EXPORTABLE void rs_free_sumset(rs_signature_t *);


/**
 * Description of input and output buffers.
 *
 * On each call to ::rs_job_iter(), the caller can make available
 *
 *  - #avail_in bytes of input data at #next_in
 *  - #avail_out bytes of output space at #next_out
 *  - or some of both
 *
 * Buffers must be allocated and passed in by the caller.
 *
 * On input, the buffers structure must contain the address and length of
 * the input and output buffers.  The library updates these values to
 * indicate the amount of \b remaining buffer.  So, on return,
 * #avail_out is not the amount of output data produced, but rather the
 * amount of output buffer space still available.
 *
 * This means that the values on
 * return are consistent with the values on entry, and suitable to be passed
 * in on a second call, but they don't directly tell you how much output
 * data was produced.
 *
 * Note also that if *#avail_in is nonzero on return, then not all of
 * the input data has been consumed.  The caller should either provide
 * more output buffer space and call ::rs_job_iter() again passing the same
 * #next_in and #avail_in, or put the remaining input data into some
 * persistent buffer and call rs_job_iter() with it again when there is
 * more output space.
 *
 * \sa rs_job_iter()
 */
struct rs_buffers_s {
    /** \brief Next input byte.
     * References a pointer which on entry should point to
     * the start of the data to be encoded.  Updated to point to the byte
     * after the last one consumed.
     **/
    char *next_in;

    /**
     * \brief Number of bytes available at next_in
     * References the length of available input.  Updated to
     * be the number of unused data bytes, which will be zero if all the
     * input was consumed.  May be zero if there is no new input, but the
     * caller just wants to drain output.
     */
    size_t avail_in;

     /**
      * \brief True if there is no more data after this.
      */
    int eof_in;

    /**
     * \brief Next output byte should be put there
     * References a pointer which on entry points to the
     * start of the output buffer.  Updated to point to the byte after the
     * last one filled.
     */
    char *next_out;

    /**
     * \brief Remaining free space at next_out
     *
     * References the size of available output buffer.
     * Updated to the size of unused output buffer.
     */
    size_t avail_out;
};

/**
 * \see ::rs_buffers_s
 */
typedef struct rs_buffers_s rs_buffers_t;

/** Default block length, if not determined by any other factors. */
#define RS_DEFAULT_BLOCK_LEN 2048

/**
* Buffer sizes for file IO.
*
* You probably only need to change these in testing.
*/
extern int rs_inbuflen;
extern int rs_outbuflen;

EXPORTABLE void rs_set_inbuflen(int new_inbuflen);
EXPORTABLE int rs_get_inbuflen();
EXPORTABLE void rs_set_outbuflen(int new_outbuflen);
EXPORTABLE int rs_get_outbuflen();


/**
 * \brief Job of work to be done.
 *
 * Created by functions such as rs_sig_begin(), and then iterated
 * over by rs_job_iter().
 *
 * The contents are opaque to the application, and instances are always
 * allocated by the library.
 *
 * \see \ref api_streaming
 * \see rs_job
 */
typedef struct rs_job rs_job_t;


/**
 * Type of application-supplied function for rs_job_drive().
 *
 * \see \ref api_pull.
 **/
typedef rs_result rs_driven_cb(rs_job_t *job, rs_buffers_t *buf, void *opaque);

EXPORTABLE void rs_set_roll_paranoia(int new_roll_paranoia);
EXPORTABLE int rs_get_roll_paranoia();

/**
 * \brief Read a signature from a file into an ::rs_signature structure
 * in memory.
 *
 * Once there, it can be used to generate a delta to a newer version of
 * the file.
 *
 * \note After loading the signatures, you must call
 * \ref rs_build_hash_table() before you can use them.
 */
rs_job_t *rs_loadsig_begin(rs_signature_t **);


/**
 * Call this after loading a signature to index it.
 *
 * Use rs_free_sumset() to release it after use.
 */
EXPORTABLE rs_result rs_build_hash_table(rs_signature_t* sums);


/**
 * \brief Callback used to retrieve parts of the basis file.
 *
 * \param pos Position where copying should begin.
 *
 * \param len On input, the amount of data that should be retrieved.
 * Updated to show how much is actually available, but should not be greater
 * than the input value.
 *
 * \param buf On input, a buffer of at least \p *len bytes.  May be
 * updated to point to a buffer allocated by the callback if it
 * prefers.
 */
typedef rs_result rs_copy_cb(void *opaque, rs_long_t pos,
                             size_t *len, void **buf);



/**
 * \brief Apply a \a delta to a \a basis file to recreate
 * the \a new file.
 *
 * This gives you back a ::rs_job_t object, which can be cranked by
 * calling rs_job_iter() and updating the stream pointers.  When
 * finished, call rs_job_free() to dispose of it.
 *
 * \param copy_cb Callback used to retrieve content from the basis
 * file.
 *
 * \param copy_arg Opaque environment pointer passed through to the
 * callback.
 *
 * \todo As output is produced, accumulate the MD4 checksum of the
 * output.  Then if we find a CHECKSUM command we can check it's
 * contents against the output.
 *
 * \todo Implement COPY commands.
 *
 * \sa rs_patch_file()
 * \sa \ref api_streaming
 */
rs_job_t *rs_patch_begin(rs_copy_cb *copy_cb, void *copy_arg);


#ifndef RSYNC_NO_STDIO_INTERFACE
#include <stdio.h>


/**
 * Generate the signature of a basis file, and write it out to
 * another.
 *
 * \param old_file Stdio readable file whose signature will be generated.
 *
 * \param sig_file Writable stdio file to which the signature will be written./
 *
 * \param block_len block size for signature generation, in bytes
 *
 * \param strong_len truncated length of strong checksums, in bytes
 *
 * \param sig_magic A signature magic number indicating
 * what format to use.
 *
 * \param stats Optional pointer to receive statistics.
 *
 * \sa \ref api_whole
 */
EXPORTABLE rs_result rs_sig_file_magic(FILE *old_file, FILE *sig_file,
                      size_t block_len, size_t strong_len,
                      rs_magic_number sig_magic,
                      rs_stats_t *stats);

/**
 * Generate the signature of a basis file, and write it out to
 * another.
 *
 * \param old_file Stdio readable file whose signature will be generated.
 *
 * \param sig_file Writable stdio file to which the signature will be written./
 *
 * \param block_len block size for signature generation, in bytes
 *
 * \param strong_len truncated length of strong checksums, in bytes
 *
 * \param stats Optional pointer to receive statistics.
 *
 * \sa \ref api_whole
 */
EXPORTABLE rs_result rs_sig_file(FILE *old_file, FILE *sig_file,
                      size_t block_len, size_t strong_len,
                      rs_stats_t *stats);

/**
 * Load signatures from a signature file into memory.  Return a
 * pointer to the newly allocated structure in \p sumset.
 *
 * \sa \ref api_whole
 */
EXPORTABLE rs_result rs_loadsig_file(FILE *sig_file, rs_signature_t **sumset,
    rs_stats_t *stats);

/**
 * ::rs_copy_cb that reads from a stdio file.
 **/
rs_result rs_file_copy_cb(void *arg, rs_long_t pos, size_t *len, void **buf);


/**
 * Generate a delta between a signature and a new file, int a delta file.
 * \sa \ref api_whole
 **/
EXPORTABLE rs_result rs_delta_file(rs_signature_t *, FILE *new_file, FILE *delta_file, rs_stats_t *);

/**
 * Apply a patch, relative to a basis, into a new file.
 * \sa \ref api_whole
 */
EXPORTABLE rs_result rs_patch_file(FILE *basis_file, FILE *delta_file, FILE *new_file, rs_stats_t *);
#endif /* ! RSYNC_NO_STDIO_INTERFACE */

#ifdef __cplusplus
}
#endif

#include "trace.h"

#endif /* ! __LIBRSYNC_LIBRSYNC_H_ */

/* vim: expandtab shiftwidth=4 tabstop=4 sts=4
 */
