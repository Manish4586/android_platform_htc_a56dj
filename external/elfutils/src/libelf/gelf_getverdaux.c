/* Get additional symbol version definition information at the given offset.
   Copyright (C) 1999, 2000, 2001, 2002 Red Hat, Inc.
   This file is part of elfutils.
   Written by Ulrich Drepper <drepper@redhat.com>, 1999.

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

#include <assert.h>
#include <gelf.h>
#include <string.h>

#include "libelfP.h"


GElf_Verdaux *
gelf_getverdaux (data, offset, dst)
     Elf_Data *data;
     int offset;
     GElf_Verdaux *dst;
{
  GElf_Verdaux *result;

  if (data == NULL)
    return NULL;

  if (unlikely (data->d_type != ELF_T_VDEF))
    {
      __libelf_seterrno (ELF_E_INVALID_HANDLE);
      return NULL;
    }

  assert (sizeof (GElf_Verdaux) == sizeof (Elf32_Verdaux));
  assert (sizeof (GElf_Verdaux) == sizeof (Elf64_Verdaux));

  rwlock_rdlock (((Elf_Data_Scn *) data)->s->elf->lock);

  if (unlikely (offset < 0)
      || unlikely (offset + sizeof (GElf_Verdaux) > data->d_size)
      || unlikely (offset % __alignof__ (GElf_Verdaux) != 0))
    {
      __libelf_seterrno (ELF_E_OFFSET_RANGE);
      result = NULL;
    }
  else
    result = (GElf_Verdaux *) memcpy (dst, (char *) data->d_buf + offset,
				      sizeof (GElf_Verdaux));


  rwlock_unlock (((Elf_Data_Scn *) data)->s->elf->lock);

  return result;
}
