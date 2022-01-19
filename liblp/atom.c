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
 * Routines related to atom tables
 *
 * (c) 2005-2021 Tomi Janhunen
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "version.h"
#include "symbol.h"
#include "atom.h"
#include "rule.h"
#include "io.h"

/* --------------------- Print version information ------------------------- */

void _version_atom_h()
{
  _version(_ATOM_H_RCSFILE, _ATOM_H_DATE, _ATOM_H_REVISION);
}

void _version_atom_c()
{
  _version_atom_h();
  _version("$RCSfile: atom.c,v $",
	   "$Date: 2021/05/27 07:54:50 $",
	   "$Revision: 1.18 $");
}

/* ------------------------ Handling atom tables --------------------------- */

ATAB *new_table(int count, int offset)
{
  ATAB *table = (ATAB *)malloc(sizeof(ATAB));
  SYMBOL **names = (SYMBOL **)malloc((count+1) * sizeof(SYMBOL *));
  int *statuses = (int *)malloc((count+1) * sizeof(int));
  int i = 0;

  symbol_table_init();  /* Initialize low level table */

  table->count = count;
  table->offset = offset;
  table->shift = 0;
  table->prefix = NULL;
  table->postfix = NULL;
  table->other = NULL;
  table->next = NULL;
  table->last = table;  /* Defined only for the first piece */
  table->names = names;
  table->statuses = statuses;
  table->others = NULL;

  /* Clear names and status bits */

  for(i=0; i<=count; i++) {
    names[i] = NULL;
    statuses[i] = 0;
  }

  return table;
}

int *initialize_other_table(ATAB *table1, ATAB *table2);

ATAB *extend_table(ATAB *table, int count, int offset)
{
  ATAB *extension = new_table(count, offset);
  ATAB *last = table->last;

  extension->shift = table->shift;
  extension->prefix = table->prefix;
  extension->postfix = table->postfix;
  if(table->other)
    initialize_other_table(extension, table->other);

  /* Add this as the last piece */

  last->next = extension;
  table->last = extension;
  extension->last = NULL;

  return extension;
}

ATAB *copy_table(ATAB *table)
{
  ATAB *copy = (ATAB *)malloc(sizeof(ATAB));
  ATAB *first = copy;

  /* Share structure as far as possible */

  while(table) {  /* Proceed piece by piece */

    copy->count = table->count;
    copy->offset = table->offset;
    copy->shift = table->shift;
    copy->prefix = table->prefix;
    copy->postfix = table->postfix;
    copy->other = table->other;
    copy->last = NULL;
    copy->names = table->names;
    copy->statuses = table->statuses;
    copy->others = table->others;

    if(table = table->next) {
      copy->next = (ATAB *)malloc(sizeof(ATAB));
      copy = copy->next;
    } else
      copy->next = NULL;

  }
  first->last = copy;

  return first;
}

ATAB *append_table(ATAB *table1, ATAB *table2)
{
  if(table1) {
    ATAB *last = table1->last;

    last->next = table2;
    table1->last = table2->last;
    table2->last = NULL;

  } else
    table1 = table2;

  return table1;
}

ATAB *make_contiguous(ATAB *table)
{
  int size = table_size(table);
  int offset = table->offset;
  int count = size-offset;
  ATAB *new = new_table(count, offset);
  ATAB *scan = table;
  int i = 0;

  new->shift = table->shift;
  new->prefix = table->prefix;
  new->postfix = table->postfix;
  new->other = table->other;

  if(scan->others) {
    new->others = (int *)malloc((size-offset+1)*sizeof(int));

    for(i=0; i<=count; i++)
      (new->others)[i] = 0;
  }

  while(scan) {
    ATAB *next = scan->next;

    int count2 = scan->count;
    int offset2 = scan->offset - offset;

    for(i=1; i<=count2; i++) {

      (new->names)[i+offset2] = (scan->names)[i];
      (new->statuses)[i+offset2] = (scan->statuses)[i];
      if(scan->others)
	(new->others)[i+offset2] = (scan->others)[i];
    }

    free(scan->names);
    free(scan->statuses);
    if(scan->others)
      free(scan->others);
    free(scan);

    scan = next;
  }

  return new;
}

/* ----------------------- Enable cross-references ------------------------ */

int *initialize_other_table(ATAB *table1, ATAB *table2)
{
  int i = 0;
  int count = table1->count;
  int *others = (int *)malloc(sizeof(int)*(count+1));

  table1->other = table2;
  table1->others = others;
  for(i = 0; i<=count; i++) others[i] = 0;
  
  return others;
}

void initialize_other_tables(ATAB *table1, ATAB *table2)
{
  while(table1) {
    (void) initialize_other_table(table1, table2);
    table1 = table1->next;
  }
}

/* --------------------- Cross-linking symbol tables ----------------------- */

void attach_atoms_to_names(ATAB *table)
{
  ATAB *scan = table;
  int i = 0;

  while(scan) {
    int count = scan->count;
    int offset = scan->offset;
    SYMBOL **names = scan->names;

    for(i=1; i<=count; i++) {
      int atom = i+offset;
      SYMBOL *sym = names[i];

      if(sym) {
	sym->info.atom = atom;   /* The atom has a symbolic name */
	sym->info.table = scan;  /* For quick access of the table entry */
      }
    }
    scan = scan->next;
  }

  return;
}

void detach_atoms_from_names(ATAB *table)
{
  ATAB *scan = table;
  int i = 0;

  while(scan) {
    int count = scan->count;
    int offset = scan->offset;
    SYMBOL **names = scan->names;

    for(i=1; i<=count; i++) {
      int atom = i+offset;
      SYMBOL *sym = names[i];

      /* Clear entries set by attach_atoms_to_names */

      if(sym) {
	sym->info.atom = 0;  
	sym->info.table = NULL;
      }
    }
    scan = scan->next;
  }
}

int combine_atom_tables(ATAB *table1, ATAB *table2,
			int checkmissing, int checkinput, int checkoutput)
{
  ATAB *scan = table1;
  int i = 0, j = 0;

  while(scan) {
    int count = scan->count;
    int offset = scan->offset;
    SYMBOL **names = scan->names;

    for(i=1; i<=count; i++) {
      int atom = i+offset;
      SYMBOL *sym = names[i];

      if(sym) {   /* The atom has a symbolic name */

	ATAB *other = sym->info.table;   /* Relevant piece */
	int atom2 = sym->info.atom;
	
	if(atom2 && other) {
 
          /* The name appears in the other program as well */

	  int j = atom2 - other->offset;  /* Calculate index */

	  (scan->others)[i] = sym->info.atom;

	  if(checkinput) {
	    int status1 = (scan->statuses)[i];
	    int status2 = (other->statuses)[j];

	    if((status1 & MARK_INPUT) && !(status2 & MARK_INPUT))
	      return i+(scan->offset);
	  }

	  if(checkoutput) {
	    int status1 = (scan->statuses)[i];
	    int status2 = (other->statuses)[j];

	    /* Report atoms that are defined by both programs */

	    if(!(status1 & MARK_INPUT) && !(status2 & MARK_INPUT))
	      return i+(scan->offset);

	    /* Clear input atoms that get defined by the other program */

	    if((status1 & MARK_INPUT) && !(status2 & MARK_INPUT))
	      (scan->statuses[i]) &= ~MARK_INPUT;

	    if(!(status1 & MARK_INPUT) && (status2 & MARK_INPUT))
	      (other->statuses[j]) &= ~MARK_INPUT;

	  }

	} else {

	  if(checkmissing)
	    return i+(table1->offset);

	}
      }
    }
    scan = scan->next;
  }

  return 0;
}

int compare_atom_tables(ATAB *table1, ATAB *table2, int checkinput)
{
  int rvalue = 0;

  attach_atoms_to_names(table2);

  if(rvalue = combine_atom_tables(table1, table2, -1, checkinput, 0))
    return rvalue;

  detach_atoms_from_names(table2);

  return 0;
}

int match_atom_tables(ATAB *table1, ATAB *table2, int checkoutput)
{
  int rvalue = 0;

  /* Match as far as possible; report atoms that are defined
     (output atoms) in both programs */

  attach_atoms_to_names(table2);

  if(rvalue = combine_atom_tables(table1, table2, 0, 0, checkoutput))
    return rvalue;

  detach_atoms_from_names(table2);

  return 0;
}

void transfer_compute_statement(ATAB *table1, ATAB *table2)
{
  ATAB *scan = table1;
  int i = 0;

  /* Presumes a previous call to attach_atom_to_names(table2) */

  while(scan) {
    int count = scan->count;
    int offset = scan->offset;
    SYMBOL **names = scan->names;

    for(i=1; i<=count; i++) {
      int atom = i+offset;
      SYMBOL *sym = names[i];

      if(sym) {   /* The atom has a symbolic name */

	ATAB *other = sym->info.table;    /* Relevant piece */
	int atom2 = sym->info.atom;
	
	if(atom2 && other) {
	  int j = atom2 - other->offset;  /* Calculate index */

	  (other->statuses)[j] |=
	    ((scan->statuses)[i] & MARK_TRUE_OR_FALSE);

	}
      }
    }
    scan = scan->next;
  }

  return;
}

/* --------------- Set status bit (e.g. compute statement) ----------------- */

int set_status_by_name(ATAB *table, char *name, int mask)
{
  while(table) {
    int count = table->count;
    SYMBOL **names = table->names;
    int *statuses = table->statuses;
    int i = 0;

    for(i=1; i<=count; i++) {
      SYMBOL *s = names[i];
      if(s && strcmp(s->name, name) == 0) {
	statuses[i] |= mask;
	return -1;
      }
    }

    table = table->next;
  }
  return 0;
}

/* Find the piece in which atom is stored */

ATAB *find_atom(ATAB *table, int atom)
{
  while(table) {
    int count = table->count;
    int offset = table->offset;
    if(atom > offset && atom <= offset+count)
      return table;
    table = table->next;
  }
  return NULL;
}

int find_atom_by_name(ATAB *table, char *name)
{
  while(table) {
    int count = table->count;
    int offset = table->offset;
    SYMBOL **names = table->names;
    int i = 0;

    for(i=1; i<=count; i++) {
      SYMBOL *s = names[i];
      if(s && strcmp(s->name, name) == 0)
	return i+offset;
    }
    table = table->next;
  }
  return 0;
}

SYMBOL *find_name(ATAB *table, int atom)
{
  ATAB *piece = find_atom(table, atom);

  if(piece) {
    int offset = piece->offset;
    SYMBOL **names = piece->names;
    return names[atom-offset];
  }
  return NULL;
}

int set_status(ATAB *table, int atom, int mask)
{
  ATAB *piece = find_atom(table, atom);

  if(piece) {
    int *statuses = piece->statuses;
    int offset = piece->offset;

    statuses[atom-offset] |= mask;
    return -1;
  }

  return 0;
}

int set_statuses(ATAB *table, int cnt, int *atoms, int mask)
{
  int rvalue = -1;

  while((cnt--)>0)
    rvalue &= set_status(table, *atoms++, mask);

  return rvalue;
}

int clear_status(ATAB *table, int atom, int mask)
{
  ATAB *piece = find_atom(table, atom);

  if(piece) {
    int *statuses = piece->statuses;
    int offset = piece->offset;

    statuses[atom-offset] &= ~mask;
    return -1;
  }

  return 0;
}

int get_status(ATAB *table, int atom)
{
  ATAB *piece = find_atom(table, atom);

  if(piece) {
    int *statuses = piece->statuses;
    int offset = piece->offset;

    return statuses[atom-offset];

  } else
    return -1;
}

int set_name(ATAB *table, int atom, char *name)
{
  ATAB *piece = find_atom(table, atom);

  if(piece) {
    SYMBOL **names = piece->names;
    int offset = piece->offset;

    names[atom-offset] = find_symbol(name);
    return -1;
  }

  return 0;
}

int set_module(ATAB *table, int atom, int module)
{
  ATAB *piece = find_atom(table, atom);

  if(piece) {
    SYMBOL **names = piece->names;
    int offset = piece->offset;
    SYMBOL *name = names[atom-offset];

    if(name) {
      name->info.module = module;
      return -1;
    }
  }

  return 0;
}

/* ----------------------- Denial of invisible atoms ---------------------- */

int find_invisible(ATAB *table)
{
  while(table) {
    int count = table->count;
    SYMBOL **names = table->names;
    int i = 0;

    for(i = 1; i <= count; i++) {
      SYMBOL *name = names[i];

      if(!name) return i+(table->offset);
    }
    table = table->next;
  }
  return 0;
}

/* ------------------------------ Table size ------------------------------ */

int table_size(ATAB *table)
{
  int max = 0;

  while(table) {
    int last = table->offset+table->count;
    if(last>max) max=last;
    table = table->next;
  }

  return max;
}

void set_shift(ATAB* table, int shift)
{
  while(table) {
    table->shift = shift;
    table = table->next;
  }

  return;
}

void set_prefix(ATAB* table, char *prefix)
{
  while(table) {
    table->prefix = prefix;
    table = table->next;
  }

  return;
}

void set_postfix(ATAB* table, char *postfix)
{
  while(table) {
    table->postfix = postfix;
    table = table->next;
  }

  return;
}

/* --------------------------- Stack operations --------------------------- */

ASTACK *push(int atom, int status, char *name, ASTACK *stack)
{
  ASTACK *value = (ASTACK *)malloc(sizeof(ASTACK));

  value->atom = atom;
  value->status = status;
  value->name = name;
  value->under = stack;

  return value;
}

ASTACK *pop(int *atom, int *status, char **name, ASTACK *stack)
{
  ASTACK *under = stack->under;

  *atom = stack->atom;
  if(status) *status = stack->status;
  if(name) *name = stack->name;
  free(stack);
  
  return under;
}

/* --------------------------- queue operations --------------------------- */

void add_element(AQUEUE *q, QELEM *e)
{
  if(q->last) {
    q->last->next = e;
  }
  q->last = e;
  if(!(q->first))
    q->first = e;
}

void qdef(int atom, AQUEUE *q)
{
  QELEM *e = (QELEM *)malloc(sizeof(QELEM));

  e->etype = AQ_DEF;
  e->elem.atom =  atom;
  e->next = NULL;

  add_element(q, e);

  return;
}

void qstr(char *name, AQUEUE *q)
{
  QELEM *e = (QELEM *)malloc(sizeof(QELEM));

  e->etype = AQ_STR;
  e->elem.string =  name;
  e->next = NULL;

  add_element(q, e);

  return;
}

void qidx(int idx, AQUEUE *q)
{
  QELEM *e = (QELEM *)malloc(sizeof(QELEM));

  e->etype = AQ_IDX;
  e->elem.index =  idx;
  e->next = NULL;

  add_element(q, e);

  return;
}

void qatom(int atom, ATAB *table, AQUEUE *q)
{
  AREF *a = (AREF *)malloc(sizeof(AREF));
  QELEM *e = (QELEM *)malloc(sizeof(QELEM));

  a->atom = atom;
  a->table = table;

  e->etype = AQ_ATM;
  e->elem.aref =  a;
  e->next = NULL;

  add_element(q, e);

  return;
}

QELEM *print_elem(FILE *out, QELEM *q)
{
  while(q && q->etype !=AQ_DEF) {
    switch(q->etype) {
    case AQ_STR:
      fprintf(out, "%s", q->elem.string);
      break;
    case AQ_IDX:
      fprintf(out, "%i", q->elem.index);
      break;
    case AQ_ATM:
      write_atom(STYLE_READABLE, out, q->elem.aref->atom,
		 q->elem.aref->table);
    }
    q = q->next;
  }

  return q;
}

/* ------------------ Introduce names for invisible atoms ------------------ */

int log10i(int number)
{
  int rvalue = number<0 ? 1 : 0;

  while(number != 0)
    number /= 10, rvalue++;

  return rvalue;
}

void name_invisible_atoms(char *prefix, ATAB *table)
{
  while(table) {
    int count = table->count;
    int offset = table->offset;
    int shift = table->shift;
    SYMBOL **names = table->names;
    int *statuses = table->statuses;
    int i = 0;

    for(i=1; i<=count; i++) {
      int atom = i+offset;
      SYMBOL *name = names[i];

      if(!name) {
	char *internal = (char *)malloc(strlen(prefix)+log10i(atom)+1);

	sprintf(internal, "%s%i", prefix, atom);
	names[i] = find_symbol(internal);
      }
    }
    table = table->next;
  }
  return;

}
