/* liblp -- ASPTOOLS library for the Smodels file format

   Copyright (C) 2022 Tomi Janhunen

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
   USA
*/

/*
 * Definitions related to version management
 *
 * (c) 2005-2021 Tomi Janhunen
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "version.h"

void _rcs_keyword(FILE *out, char* keyword)
{
  char ch;

  if(*(keyword++) != '$')
    return;

  while((ch = *keyword) != ':' && ch != '\0') keyword++;

  if(*keyword == ':') keyword++;
  if(*keyword == ' ') keyword++;

  while((ch = *keyword) != '$' && ch != '\0') {
    keyword++;
    if(ch != ' ' || *keyword != '$')
      fputc((int)ch, out);
  }

  return;
}

void _version(char *kw1, char *kw2, char *kw3)
{
  fputs("File ", stderr);
  _rcs_keyword(stderr, kw1);
  fputs(" [", stderr);
  _rcs_keyword(stderr, kw2);
  fputs("]: ", stderr);
  _rcs_keyword(stderr, kw3);
  fputs("\n", stderr);
}
