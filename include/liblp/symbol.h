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
 * Symbol table to LPEQ etc
 *
 * (c) 2006 Tomi Janhunen
 */

/*
 * Definitions related to symbols (names of atoms)
 */

#define _SYMBOL_H_RCSFILE  "$RCSfile: symbol.h,v $"
#define _SYMBOL_H_DATE     "$Date: 2021/05/27 08:50:04 $"
#define _SYMBOL_H_REVISION "$Revision: 1.4 $"

extern void _version_symbol_c();

typedef struct info {
  int atom;            /* Atom number */
  struct atab *table;  /* Symbol table */
  int module;          /* Module number */
} INFO;

typedef struct symbol {
  char *name;          /* String */
  INFO info;           /* Data associated with this symbol (if any) */
  struct symbol *next; /* Next entry */
} SYMBOL;

extern void symbol_table_init();
extern SYMBOL *make_symbol(char *);
extern void print_symbol(FILE *out, SYMBOL *);
extern SYMBOL *find_symbol(char *name);
