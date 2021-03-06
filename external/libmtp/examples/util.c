/** 
 * \file util.c
 * A set of common utility functions found
 * in all samples.
 *
 * Copyright (C) 2008 Linus Walleij <triad@df.lth.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "util.h"
#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void checklang(void)
{
  char *langsuff = NULL;
  char *lang = getenv("LANG");

#ifdef HAVE_LOCALE_H
  
  setlocale(LC_ALL, "");
#endif
#ifdef HAVE_LANGINFO_H
  langsuff = nl_langinfo(CODESET);
#else
  if (lang != NULL) {
    if (strlen(lang) > 5) {
      langsuff = &lang[strlen(lang)-5];
    }
  }
#endif
  if (langsuff == NULL) {
    printf("Could not determine language suffix for your system. Please check your setup!\n");
  } else if (strcmp(langsuff, "UTF-8")) {
    printf("Your system does not appear to have UTF-8 enabled ($LANG=\"%s\")\n", lang);
    printf("If you want to have support for diacritics and Unicode characters,\n");
    printf("please switch your locale to an UTF-8 locale, e.g. \"en_US.UTF-8\".\n");
  }
}
