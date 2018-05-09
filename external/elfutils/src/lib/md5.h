/* Declaration of functions and data types used for MD5 sum computing
   library functions.
   Copyright (C) 1995,1996,1997,1999-2001,2004,2005,2008 Red Hat, Inc.
   This file is part of elfutils.
   Written by Ulrich Drepper <drepper@redhat.com>, 1995.

   This file is free software; you can redistribute it and/or modify
   it under the terms of either

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at
       your option) any later version

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at
       your option) any later version

   or both in parallel, as here.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _MD5_H
#define _MD5_H 1

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#define MD5_DIGEST_SIZE 16
#define MD5_BLOCK_SIZE 64

typedef uint32_t md5_uint32;
typedef uintptr_t md5_uintptr;

struct md5_ctx
{
  md5_uint32 A;
  md5_uint32 B;
  md5_uint32 C;
  md5_uint32 D;

  md5_uint32 total[2];
  md5_uint32 buflen;
  char buffer[128] __attribute__ ((__aligned__ (__alignof__ (md5_uint32))));
};


extern void md5_init_ctx (struct md5_ctx *ctx);

extern void md5_process_block (const void *buffer, size_t len,
			       struct md5_ctx *ctx);

extern void md5_process_bytes (const void *buffer, size_t len,
			       struct md5_ctx *ctx);

extern void *md5_finish_ctx (struct md5_ctx *ctx, void *resbuf);


extern void *md5_read_ctx (const struct md5_ctx *ctx, void *resbuf);


/* Compute MD5 message digest for bytes read from STREAM.  The
   resulting message digest number will be written into the 16 bytes
   beginning at RESBLOCK.  */
extern int md5_stream (FILE *stream, void *resblock);

extern void *md5_buffer (const char *buffer, size_t len, void *resblock);

#endif 
