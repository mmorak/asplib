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
 * Symbol table for LPEQ etc
 *
 * (c) 2006 Tomi Janhunen
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "version.h"
#include "symbol.h"

#define HASH_SIZE   32771  /* Prime */

SYMBOL **symbol_table = NULL; /* Hash table */

/*
 * _version_symbol_c -- print version information
 */

void _version_symbol_h()
{
  _version(_SYMBOL_H_RCSFILE, _SYMBOL_H_DATE, _SYMBOL_H_REVISION);
}

void _version_symbol_c()
{
  _version_symbol_h();
  _version("$RCSfile: symbol.c,v $",
	   "$Date: 2021/05/27 07:55:48 $",
	   "$Revision: 1.5 $");
}

/*
 * symbol_table_init -- Initialize symbol table
 */

void symbol_table_init()
{
  if(!symbol_table) {
    int i=0;
    symbol_table = (SYMBOL **)malloc(sizeof(SYMBOL *)*HASH_SIZE);

    for(i=0; i<HASH_SIZE; i++)
      symbol_table[i] = NULL;
  }

  return;
}

/*
 * make_symbol -- Allocate entry for a new symbol
 */

SYMBOL *make_symbol(char *name)
{
  SYMBOL *s = (SYMBOL *)malloc(sizeof(struct symbol));
  int len = strlen(name)+1;

  s->name = (char *)malloc(len);
  strncpy(s->name, name, len);
  s->info.atom = 0;
  s->info.table = NULL;
  s->info.module = 0;
  s->next = NULL;

  return s;
}

/*
 * hash -- Calculate a hash value for a string (a la P. J. Weinberger)
 */

int hash(char *name)
{
  char *scan;
  unsigned int h = 0, g;

  for (scan = name; *scan; scan++) {
    h = (h << 4) + *scan;
    if (g = h&0xf0000000) {
      h ^= g >> 24;
      h ^= g;
    }
  }
  return h % HASH_SIZE;
}

/*
 * find_symbol -- Fetch the entry for the identifier from the hash table
 */

SYMBOL *find_symbol(char *name)
{
  int hval = hash(name);
  SYMBOL *scan = symbol_table[hval];

  while (scan && strcmp(scan->name, name) != 0)
    scan = scan->next;

  if (scan == NULL) {
    scan = make_symbol(name);
    scan->next = symbol_table[hval];
    symbol_table[hval] = scan;
  }

  return scan;
}

/*
 * print_symbol -- Print a symbol
 */

void print_symbol(FILE *out, SYMBOL *s)
{
  fprintf(out, "%s", s->name);
}
