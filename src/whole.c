/*= -*- c-basic-offset: 4; indent-tabs-mode: nil; -*-
 *
 * librsync -- the library for network deltas
 *
 * Copyright 2000, 2001, 2014, 2015 by Martin Pool <mbp@sourcefrog.net>
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

                              /*
                               | Is it possible that software is not
                               | like anything else, that it is meant
                               | to be discarded: that the whole point
                               | is to always see it as a soap bubble?
                               |        -- Alan Perlis
                               */



#include "whole.h"

#include <assert.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "fileutil.h"
#include "sumset.h"
#include "job.h"
#include "buf.h"
#include "util.h"
#include "mksum.h"
#include "delta.h"


/**
 * Run a job continuously, with input to/from the two specified files.
 * The job should already be set up, and must be free by the caller
 * after return.
 *
 * Buffers of ::rs_inbuflen and ::rs_outbuflen are allocated for
 * temporary storage.
 *
 * \param in_file Source of input bytes, or NULL if the input buffer
 * should not be filled.
 *
 * \return RS_DONE if the job completed, or otherwise an error result.
 */
rs_result
rs_whole_run(rs_job_t *job, FILE *in_file, FILE *out_file)
{
    rs_buffers_t    buf;
    rs_result       result;
    rs_filebuf_t    *in_fb = NULL, *out_fb = NULL;

    if (in_file)
        in_fb = rs_filebuf_new(in_file, rs_inbuflen);

    if (out_file)
        out_fb = rs_filebuf_new(out_file, rs_outbuflen);

    result = rs_job_drive(job, &buf,
                          in_fb ? rs_infilebuf_fill : NULL, in_fb,
                          out_fb ? rs_outfilebuf_drain : NULL, out_fb);

    if (in_fb)
        rs_filebuf_free(in_fb);

    if (out_fb)
        rs_filebuf_free(out_fb);

    return result;
}



EXPORTABLE rs_result rs_sig_file_magic(FILE *old_file, FILE *sig_file, size_t new_block_len,
            size_t strong_len,
            rs_magic_number sig_magic,
            rs_stats_t *stats)
{
    rs_job_t        *job;
    rs_result       r;

    job = rs_sig_begin(new_block_len, strong_len, sig_magic);
    r = rs_whole_run(job, old_file, sig_file);
    if (stats)
        memcpy(stats, &job->stats, sizeof *stats);
    rs_job_free(job);

    return r;
}

rs_result
EXPORTABLE rs_sig_filename_magic(const char *basis_filename, const char *sig_filename, size_t new_block_len,
            size_t strong_len,
            rs_magic_number sig_magic,
            rs_stats_t *stats)
{
    rs_job_t        *job;
    rs_result       r;

    if (RS_BLAKE2_SIG_MAGIC == sig_magic) {
        rs_debug("Generating Blake2 Signature file %s of %s\n", sig_filename, basis_filename);
    } else if (RS_MD4_SIG_MAGIC == sig_magic) {
        rs_debug("Generating MD4 Signature file %s of %s\n", sig_filename, basis_filename);
    }

    FILE* basis_file = rs_file_open(basis_filename, "rb");
    FILE* sig_file = rs_file_open(sig_filename, "wb");

    job = rs_sig_begin(new_block_len, strong_len, sig_magic);
    r = rs_whole_run(job, basis_file, sig_file);

    rs_file_close(sig_file);
    rs_file_close(basis_file);

    rs_debug("Done Generating Signature file.\n");

    if (stats)
        memcpy(stats, &job->stats, sizeof *stats);
    rs_job_free(job);

    return r;
}

EXPORTABLE rs_result rs_sig_file(FILE *old_file, FILE *sig_file, size_t new_block_len,
            size_t strong_len, rs_stats_t *stats)
{
    return rs_sig_file_magic(old_file, sig_file, new_block_len, strong_len, RS_DEFAULT_SIG_MAGIC, stats);
}


EXPORTABLE rs_result rs_loadsig_file(FILE *sig_file, rs_signature_t **sumset, rs_stats_t *stats)
{
    rs_job_t            *job;
    rs_result           rr = RS_INTERNAL_ERROR;

    if (NULL != sig_file)
    {
#ifdef WIN32
        struct _stat statbuf;
        int fstat_return = _fstat(fileno(sig_file), &statbuf);
#else
        struct stat statbuf;
        int fstat_return = fstat(fileno(sig_file), &statbuf);
#endif

        job = rs_loadsig_begin(sumset);

        job->signature->flength = 0 == fstat_return ? statbuf.st_size : 0;

        rr = rs_whole_run(job, sig_file, NULL);
        if (stats)
            memcpy(stats, &job->stats, sizeof(*stats));
        rs_job_free(job);
    }

    return rr;
}


rs_result
EXPORTABLE rs_loadsig_filename(const char *sig_name, rs_signature_t **sumset, rs_stats_t *stats)
{
    FILE *sig_file = rs_file_open(sig_name, "rb");

    rs_log0(RS_LOG_DEBUG, __FUNCTION__, "Loading signature file %s.\n", sig_name);
    rs_result result = rs_loadsig_file(sig_file, sumset, stats);
    rs_log0(RS_LOG_DEBUG, __FUNCTION__, "Finished loading signature file, result %d.\n", result);
    rs_file_close(sig_file);
    return result;
}

EXPORTABLE rs_result rs_delta_file(rs_signature_t *sig
    , FILE *new_file, FILE *delta_file
    , rs_stats_t *stats)
{
    rs_job_t            *job;
    rs_result           rr = RS_INTERNAL_ERROR;

    if (NULL != new_file && NULL != delta_file)
    {
        job = rs_delta_begin(sig);

        rr = rs_whole_run(job, new_file, delta_file);

        if (stats)
            memcpy(stats, &job->stats, sizeof *stats);

        rs_job_free(job);
    }

    return rr;
}

rs_result
EXPORTABLE rs_delta_filename(rs_signature_t *sig
    , const char *new_filename, const char *delta_filename
    , rs_stats_t *stats)
{
    FILE* new_file = rs_file_open(new_filename, "rb");
    FILE* delta_file = rs_file_open(delta_filename, "wb");

    rs_log0(RS_LOG_DEBUG, __FUNCTION__, "Processing delta file %s.\n", delta_filename);
    rs_result result = rs_delta_file(sig, new_file, delta_file, stats);
    rs_log0(RS_LOG_DEBUG, __FUNCTION__, "Finished processing delta file, result %d.\n", result);

    rs_file_close(delta_file);
    rs_file_close(new_file);

    return result;
}

EXPORTABLE rs_result rs_patch_file(FILE *basis_file, FILE *delta_file, FILE *new_file,
                        rs_stats_t *stats)
{
    rs_job_t            *job;
    rs_result           r;

    job = rs_patch_begin(rs_file_copy_cb, basis_file);

    r = rs_whole_run(job, delta_file, new_file);
    
    if (stats)
        memcpy(stats, &job->stats, sizeof *stats);

    rs_job_free(job);

    return r;
}

/* vim: expandtab shiftwidth=4 tabstop=4 sts=4
 */
