/* x86-64 specific core note handling.
   Copyright (C) 2005, 2007, 2008 Red Hat, Inc.
   This file is part of elfutils.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <elf.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>

#define BACKEND		x86_64_
#include "libebl_CPU.h"


static const Ebl_Register_Location prstatus_regs[] =
  {
#define GR(at, n, dwreg)						\
    { .offset = at * 8, .regno = dwreg, .count = n, .bits = 64 }
#define SR(at, n, dwreg)						\
    { .offset = at * 8, .regno = dwreg, .count = n, .bits = 16, .pad = 6 }

    GR (0, 1, 15),		
    GR (1, 1, 14),		
    GR (2, 1, 13),		
    GR (3, 1, 12),		
    GR (4, 1, 6),		
    GR (5, 1, 3),		
    GR (6, 1, 11),		
    GR (7, 1, 10),		
    GR (8, 1, 9),		
    GR (9, 1, 8),		
    GR (10,1, 0),		
    GR (11,1, 2),		
    GR (12,1, 1),		
    GR (13,2, 4),		
    
    GR (16,1, 16),		
    SR (17,1, 51),		
    GR (18,1, 49),		
    GR (19,1, 7),		
    SR (20,1, 52),		
    GR (21,2, 58),		
    SR (23,1, 53),		
    SR (24,1, 50),		
    SR (25,2, 54),		

#undef	GR
#undef	SR
  };
#define PRSTATUS_REGS_SIZE	(27 * 8)

#define	ULONG			uint64_t
#define PID_T			int32_t
#define	UID_T			uint32_t
#define	GID_T			uint32_t
#define ALIGN_ULONG		8
#define ALIGN_PID_T		4
#define ALIGN_UID_T		4
#define ALIGN_GID_T		4
#define TYPE_ULONG		ELF_T_XWORD
#define TYPE_PID_T		ELF_T_SWORD
#define TYPE_UID_T		ELF_T_SWORD
#define TYPE_GID_T		ELF_T_SWORD

#define PRSTATUS_REGSET_ITEMS						      \
  {									      \
    .name = "orig_rax", .type = ELF_T_SXWORD, .format = 'd',		      \
    .offset = offsetof (struct EBLHOOK(prstatus), pr_reg) + (8 * 15),	      \
    .group = "register"	       			  	       	 	      \
  }

static const Ebl_Register_Location fpregset_regs[] =
  {
    { .offset = 0, .regno = 65, .count = 2, .bits = 16 }, 
    { .offset = 24, .regno = 64, .count = 1, .bits = 32 }, 
    { .offset = 32, .regno = 33, .count = 8, .bits = 80, .pad = 6 }, 
    { .offset = 32 + 128, .regno = 17, .count = 16, .bits = 128 }, 
  };
#define FPREGSET_SIZE	512

#define	EXTRA_NOTES	EXTRA_NOTES_IOPERM

#include "x86_corenote.c"
#include "linux-core-note.c"
