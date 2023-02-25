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
 * Output routines for (translated) rules
 *
 * (c) 2002-2023 Tomi Janhunen
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "version.h"
#include "symbol.h"
#include "atom.h"
#include "rule.h"
#include "io.h"

int priority = 0; /* Priority level for ASPIF translation */

/* --------------------- Print version information ------------------------- */

void _version_output_c()
{
  _version("$RCSfile: output.c,v $",
	   "$Date: 2023/02/25 13:30:11 $",
	   "$Revision: 1.37 $");
}

/* ----------------------- Output a logic program -------------------------- */

void write_name(FILE *out, SYMBOL *s, char *prefix, char *postfix)
{
  char *name = NULL;

  if(s)
    name = s->name;
  if(prefix)
    fputs(prefix, out);
  if(name) {
    while(*name != '(' && *name != '\0') {
      fputc(*name, out); name++;
    }
    if(postfix)
      fputs(postfix, out);
    while(*name != '\0') {
      fputc(*name, out); name++;
    }
  } else {
    fprintf(out, "NULL");
    if(postfix)
      fputs(postfix, out);
  }
}

int atomlen(int atom, ATAB *table)
{
  ATAB *piece = find_atom(table, atom);
  int len = 0;

  if(piece) {
    int offset = piece->offset;
    int shift = piece->shift;
    SYMBOL **names = piece->names;
    SYMBOL *name = names[atom-offset];

    /* The length is calculated as if printed in STYLE_READABLE */

    if(name) {
      char *str = name->name;
      if(str) {
	if(piece->prefix)
	  len += strlen(piece->prefix);
	len += strlen(str);
	if(piece->postfix)
	  len += strlen(piece->postfix);
      } else
	len = strlen("NULL");
    } else
      len = 1+log10i(atom+shift); /* Preceded by underscore */

  } else {
    fprintf(stderr, "%s: entry _%i out of table\n", program_name, atom);
    exit(-1);
  }

  return len;
}

void write_atom(int style, FILE *out, int atom, ATAB *table)
{
  ATAB *piece = find_atom(table, atom);

  if(piece) {
    int offset = piece->offset;
    int shift = piece->shift;
    SYMBOL **names = piece->names;
    SYMBOL *name = names[atom-offset];

    switch(style) {
    case STYLE_READABLE:
    case STYLE_GNT:
    case STYLE_DIMACS:

      if(name)
	write_name(out, name, piece->prefix, piece->postfix);
      else
	fprintf(out, "_%i", atom+shift);
      break;

    case STYLE_SMODELS:
      fputs(" ", out);
    case STYLE_ASPIF:
      fprintf(out, "%i", atom+shift);
      break;

    case STYLE_DLV:
      if(name)
	write_name(out, name, piece->prefix, piece->postfix);
      else
	fprintf(out, "int%i", atom+shift);
      break;

    default:
      fprintf(stderr, "%s: unknown style %i for _%i\n",
	      program_name, style, atom);
      exit(-1);
    }
  } else {
    fprintf(stderr, "%s: entry _%i out of table\n", program_name, atom);
    exit(-1);
  }

  return;
}

void write_atom_list(int style, FILE *out, int cnt, int *atoms, ATAB *table)
{
  int i = 0;

  for(i=0; i<cnt; i++) {
    if(cnt>1 && i>0)
      fprintf(out, ", ");
    write_atom(style, out, atoms[i], table);
  }

  return;
}

void write_other_atom(int style, FILE *out, int atom, ATAB *table)
{
  ATAB *piece = find_atom(table, atom);

  if(piece) {
    ATAB *other = piece->other;
    int *others = piece->others;
    int offset = piece->offset;

    if(!other || !others || !others[atom-offset]) {
      fprintf(stderr, "%s: missing cross reference for ",
	      program_name);
      write_atom(STYLE_READABLE, stderr, atom, table);
      fprintf(stderr, "\n");
      exit(-1);
    } else
      write_atom(style, out, others[atom-offset], other);

  } else {
    fprintf(stderr, "%s: entry _%i out of table\n", program_name, atom);
    exit(-1);
  }

  return;
}

void write_literal_list(int style, FILE *out, char *separator,
			int pos_cnt, int *pos,
			int neg_cnt, int *neg,
			int *weight, int ones, ATAB *table)
{
  int *scan = NULL;
  int *last = NULL;
  int *wscan = weight;
  int *wlast = &weight[pos_cnt+neg_cnt];

  for(scan = neg, last = &neg[neg_cnt];
      scan != last; ) {
    if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
      fprintf(out, "not ");
    if(style == STYLE_ASPIF)
      fputs("-", out);
    write_atom(style, out, *scan, table);
    if(wscan) {
      if(style == STYLE_READABLE)
	fprintf(out, "=%i", *(wscan++));
      else if(style == STYLE_ASPIF)
	fprintf(out, " %i", *(wscan++));
    }
    if(style == STYLE_ASPIF && ones)
      fputs(" 1", out);
    scan++;
    if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
      if(scan != last || pos_cnt)
	fprintf(out, "%s", separator);
    if(style == STYLE_ASPIF)
      if(scan != last || pos_cnt)
	fputs(" ", out);
  }

  for(scan = pos, last = &pos[pos_cnt];
      scan != last; ) {
    write_atom(style, out, *scan, table);
    if(wscan) {
      if (style == STYLE_READABLE)
	fprintf(out, "=%i", *(wscan++));
      else if(style == STYLE_ASPIF)
	fprintf(out, " %i", *(wscan++));
    }
    if(style == STYLE_ASPIF && ones)
      fputs(" 1", out);
    scan++;
    if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
      if(scan != last)
	fprintf(out, "%s", separator);
    if(style == STYLE_ASPIF)
      if(scan != last)
	fputs(" ", out);
  }

  if(wscan && (style == STYLE_SMODELS))
    while(wscan != wlast)
      fprintf(out, " %i", *(wscan++));

  return;
}

/* ----------------------- Different types of rules ------------------------ */

void write_basic(int style, FILE *out, RULE *rule, ATAB *table)
{
  int *scan = NULL;
  int *last = NULL;
  int pos_cnt = 0;
  int neg_cnt = 0;

  BASIC_RULE *basic = rule->data.basic;

  if(style == STYLE_SMODELS)
    fprintf(out, "1");
  else if(style == STYLE_ASPIF)
    fprintf(out, "1 0 1 ");
    
  write_atom(style, out, basic->head, table);

  pos_cnt = basic->pos_cnt;
  neg_cnt = basic->neg_cnt;

  if(style == STYLE_SMODELS)
    fprintf(out, " %i %i", (pos_cnt+neg_cnt), neg_cnt);
  else if(style == STYLE_ASPIF)
    fprintf(out, " 0 %i ", pos_cnt+neg_cnt);
    
  if(pos_cnt || neg_cnt) {
    if(style == STYLE_READABLE || style == STYLE_GNT ||
       style == STYLE_DLV)
      fprintf(out, " :- ");

    write_literal_list(style, out, ", ",
		       pos_cnt, basic->pos,
		       neg_cnt, basic->neg,
		       NULL, 0, table);
  }

  if(style == STYLE_READABLE || style == STYLE_GNT ||
     style == STYLE_DLV)
    fprintf(out, ".");
  fprintf(out, "\n");

  return;
}

void write_constraint(int style, FILE *out, RULE *rule, ATAB *table)
{
  int pos_cnt = 0;
  int neg_cnt = 0;
  int bound = 0;

  CONSTRAINT_RULE *constraint = rule->data.constraint;

  if(style == STYLE_SMODELS)
    fprintf(out, "2");
  else if(style == STYLE_ASPIF)
    fprintf(out, "1 0 1 ");
    
  write_atom(style, out, constraint->head, table);

  pos_cnt = constraint->pos_cnt;
  neg_cnt = constraint->neg_cnt;
  bound = constraint->bound;

  if(style == STYLE_SMODELS)
    fprintf(out, " %i %i %i", (pos_cnt+neg_cnt), neg_cnt, bound);
  else if(style == STYLE_ASPIF)
    fprintf(out, " 1 %i %i ", bound, pos_cnt+neg_cnt);
    
  if(style == STYLE_READABLE)
    fprintf(out, " :- %i {", bound);

  if(pos_cnt || neg_cnt)
    write_literal_list(style, out, ", ",
		       pos_cnt, constraint->pos,
		       neg_cnt, constraint->neg,
		       NULL, -1 /* Use weights = 1 */, table);

  if(style == STYLE_READABLE)
    fprintf(out, "}.");

  fprintf(out, "\n");

  return;
}

void write_choice(int style, FILE *out, RULE *rule, ATAB *table)
{
  int head_cnt = 0;
  int pos_cnt = 0;
  int neg_cnt = 0;
  char *separator = ", ";

  CHOICE_RULE *choice = rule->data.choice;
  head_cnt = choice->head_cnt;
  pos_cnt = choice->pos_cnt;
  neg_cnt = choice->neg_cnt;

  if(style == STYLE_SMODELS)
    fprintf(out, "3 %i", head_cnt);
  else if(style == STYLE_READABLE)
    fprintf(out, "{");
  else if(style == STYLE_ASPIF)
    fprintf(out, "1 1 %i ", head_cnt);
    
  if(style == STYLE_GNT)
    separator = " | ";
  else if(style == STYLE_DLV)
    separator = " v ";

  write_literal_list(style, out, separator,
		     head_cnt, choice->head,
		     0, NULL,
		     NULL, 0, table);

  if(style == STYLE_READABLE)
    fprintf(out, "}");

  if(style == STYLE_SMODELS)
    fprintf(out, " %i %i", (pos_cnt+neg_cnt), neg_cnt);
  else if(style == STYLE_ASPIF)
    fprintf(out, " 0 %i ", pos_cnt+neg_cnt);
    
  if(pos_cnt || neg_cnt) {
    if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
      fprintf(out, " :- ");

    write_literal_list(style, out, ", ",
		       pos_cnt, choice->pos,
		       neg_cnt, choice->neg,
		       NULL, 0, table);
  }

  if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
    fprintf(out, ".");
  fprintf(out, "\n");

  return;
}

void write_integrity(int style, FILE *out, RULE *rule, ATAB *table)
{
  int *scan = NULL;
  int *last = NULL;
  int pos_cnt = 0;
  int neg_cnt = 0;

  INTEGRITY_RULE *integrity = rule->data.integrity;

  if(style == STYLE_SMODELS)
    fprintf(out, "4");
  else if(style == STYLE_ASPIF)
    fprintf(out, "1 0 0 ");
  
  pos_cnt = integrity->pos_cnt;
  neg_cnt = integrity->neg_cnt;

  if(style == STYLE_SMODELS)
    fprintf(out, " %i %i", (pos_cnt+neg_cnt), neg_cnt);
  else if(style == STYLE_ASPIF)
    fprintf(out, " 0 %i ", pos_cnt+neg_cnt);
    
  if(pos_cnt || neg_cnt) {
    if(style == STYLE_READABLE || style == STYLE_GNT ||
       style == STYLE_DLV)
      fprintf(out, " :- ");

    write_literal_list(style, out, ", ",
		       pos_cnt, integrity->pos,
		       neg_cnt, integrity->neg,
		       NULL, 0, table);
  }

  if(style == STYLE_READABLE || style == STYLE_GNT ||
     style == STYLE_DLV)
    fprintf(out, ".");
  fprintf(out, "\n");

  return;
}

void write_weight(int style, FILE *out, RULE *rule, ATAB *table)
{
  int pos_cnt = 0;
  int neg_cnt = 0;
  int bound = 0;

  WEIGHT_RULE *weight = rule->data.weight;

  if(style == STYLE_SMODELS)
    fprintf(out, "5");
  else if(style == STYLE_ASPIF)
    fprintf(out, "1 0 1 ");
    
  write_atom(style, out, weight->head, table);

  pos_cnt = weight->pos_cnt;
  neg_cnt = weight->neg_cnt;
  bound = weight->bound;

  if(style == STYLE_SMODELS)
    fprintf(out, " %i %i %i", bound, (pos_cnt+neg_cnt), neg_cnt);
  else if(style == STYLE_ASPIF)
    fprintf(out, " 1 %i %i ", bound, pos_cnt+neg_cnt);
    
  if(style == STYLE_READABLE)
    fprintf(out, " :- %i [", bound);

  if(pos_cnt || neg_cnt)
    write_literal_list(style, out, ", ",
		       pos_cnt, weight->pos,
		       neg_cnt, weight->neg,
		       weight->weight, 0, table);

  if(style == STYLE_READABLE)
    fprintf(out, "].");
  fprintf(out, "\n");

  return;
}

void write_optimize(int style, FILE *out, RULE *rule, ATAB *table)
{
  int pos_cnt = 0;
  int neg_cnt = 0;

  OPTIMIZE_RULE *optimize = rule->data.optimize;

  if(style == STYLE_SMODELS)
    fprintf(out, "6 0");
  else if(style == STYLE_ASPIF)
    fprintf(out, "2 %i", priority++);
    
  pos_cnt = optimize->pos_cnt;
  neg_cnt = optimize->neg_cnt;

  if(style == STYLE_SMODELS)
    fprintf(out, " %i %i", (pos_cnt+neg_cnt), neg_cnt);
  else if(style == STYLE_ASPIF)
    fprintf(out, " %i ", pos_cnt+neg_cnt);
    
  if(pos_cnt || neg_cnt) {
    if(style == STYLE_READABLE)
      fprintf(out, "minimize [");

    write_literal_list(style, out, ", ",
		       pos_cnt, optimize->pos,
		       neg_cnt, optimize->neg,
		       optimize->weight, 0, table);

    if(style == STYLE_READABLE)
      fprintf(out, "].");
  }
  fprintf(out, "\n");

  return;
}

void write_disjunctive(int style, FILE *out, RULE *rule, ATAB *table)
{
  int head_cnt = 0;
  int pos_cnt = 0;
  int neg_cnt = 0;
  char *separator = " | ";

  DISJUNCTIVE_RULE *disjunctive = rule->data.disjunctive;
  head_cnt = disjunctive->head_cnt;
  pos_cnt = disjunctive->pos_cnt;
  neg_cnt = disjunctive->neg_cnt;

  if(style == STYLE_SMODELS)
    fprintf(out, "8 %i", head_cnt);
  else if(style == STYLE_ASPIF)
    fprintf(out, "1 0 %i ", head_cnt);
    
  if(style == STYLE_DLV)
    separator = " v ";

  write_literal_list(style, out, separator,
		     head_cnt, disjunctive->head,
		     0, NULL,
		     NULL, 0, table);

  if(style == STYLE_SMODELS)
    fprintf(out, " %i %i", (pos_cnt+neg_cnt), neg_cnt);
  else if(style == STYLE_ASPIF)
    fprintf(out, " 0 %i ", pos_cnt+neg_cnt);
    
  if(pos_cnt || neg_cnt) {
    if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
      fprintf(out, " :- ");

    write_literal_list(style, out, ", ",
		       pos_cnt, disjunctive->pos,
		       neg_cnt, disjunctive->neg,
		       NULL, 0, table);
  }

  if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
    fprintf(out, ".");
  fprintf(out, "\n");

  return;
}

void write_clause_as_rule(int style, FILE *out, RULE *rule, ATAB *table1,
			  int contradiction, ATAB *table2)
{
  int pos_cnt = 0;
  int neg_cnt = 0;
  CLAUSE *clause = rule->data.clause;

  /* NOTE: the roles of positive/negative literals are interchanged:
   *       a clause 'a | -b' is expressed as a rule ":- b, not a". */

  if(style == STYLE_SMODELS)
    fprintf(out, "1");
  else if(style == STYLE_ASPIF)
    fprintf(out, "1 0 1 ");
    
  write_atom(style, out, contradiction, table2);

  pos_cnt = clause->pos_cnt;
  neg_cnt = clause->neg_cnt;

  if(style == STYLE_SMODELS)
    fprintf(out, " %i %i", (pos_cnt+neg_cnt), pos_cnt);
  else if(style == STYLE_ASPIF)
    fprintf(out, " 0 %i ", pos_cnt+neg_cnt);
    
  if(pos_cnt || neg_cnt) {
    if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
      fprintf(out, " :- ");

    write_literal_list(style, out, ", ",
		       neg_cnt, clause->neg,
		       pos_cnt, clause->pos,
		       NULL, 0, table1);
      
  }
  if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
    fprintf(out, ".");
  fprintf(out, "\n");

}

void write_rule(int style, FILE *out, RULE *rule, ATAB *table)
{
  switch(rule->type) {
  case TYPE_BASIC:
    write_basic(style, out, rule, table);
    break;

  case TYPE_CONSTRAINT:
    if(style == STYLE_GNT || style == STYLE_DLV) {
      fprintf(stderr,
	      "%s: constraint rules are not supported by gnt nor dlv!\n",
              program_name);
      exit(-1);
    }
    write_constraint(style, out, rule, table);
    break;

  case TYPE_CHOICE:
    write_choice(style, out, rule, table);
    break;

  case TYPE_INTEGRITY:
    write_integrity(style, out, rule, table);
    break;

  case TYPE_WEIGHT:
    if(style == STYLE_GNT || style == STYLE_DLV) {
      fprintf(stderr,
	      "%s: weight rules are not supported by gnt nor dlv!\n",
              program_name);
      exit(-1);
    }
    write_weight(style, out, rule, table);
    break;

  case TYPE_OPTIMIZE:
    if(style == STYLE_GNT || style == STYLE_DLV) {
      fprintf(stderr,
	      "%s: optimize statements are not supported by gnt nor dlv!\n",
              program_name);
      exit(-1);
    }
    write_optimize(style, out, rule, table);
    break;

  case TYPE_DISJUNCTIVE:
    write_disjunctive(style, out, rule, table);
    break;

  default:
    error("unknown rule type");
  }
}

void write_program(int style, FILE *out, RULE *rule, ATAB *table)
{
  while(rule) {
    write_rule(style, out, rule, table);
    rule = rule->next;
  }
  return;
}

void write_status(FILE *out, int flags)
{
  if(flags & MARK_TRUE)
    fprintf(out, "T");
  if(flags & MARK_FALSE)
    fprintf(out, "F");
  if(flags & MARK_HEADOCC)
    fprintf(out, "H");
  if(flags & MARK_POSOCC)
    fprintf(out, "P");
  if(flags & MARK_NEGOCC)
    fprintf(out, "N");
  if(flags & MARK_VISIBLE)
    fprintf(out, "V");
  if(flags & MARK_INPUT)
    fprintf(out, "I");

  return;
}

void write_symbols(int style, FILE *out, ATAB *table)
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
      
      switch(style) {
      case STYLE_READABLE:
      case STYLE_GNT:
      case STYLE_DLV:
	fprintf(out, "%% _%i = ", atom+shift);
	write_atom(style, out, atom, table);
	if(statuses[i]) {
	  fprintf(out, ": ");
	  write_status(out, statuses[i]);
	}
	fprintf(out, "\n");
	break;

      case STYLE_SMODELS:
	/* Only atoms having a symbolic name are printed */

	if(name) {
	  fprintf(out, "%i ", atom+shift);
	  write_atom(STYLE_READABLE, out, atom, table);
	  fprintf(out, "\n");
	}
	break;
	
      case STYLE_ASPIF:
	/* Only atoms having a symbolic name are printed */

	if(name)
	  fprintf(out, "4 %i %s 1 %i\n",
		  (int)strlen(name->name), name->name, atom+shift);
	break;

      case STYLE_DIMACS:
	/* Only atoms having a symbolic name are printed in comments */
	
	if(name) {
	  fprintf(out, "c %i ", atom+shift);
	  write_atom(style, out, atom, table);
	  fprintf(out, "\n");
	}
	break;
      default:
	break;
      }
    }
    table = table->next;
  }
  return;
}

void write_compute_statement(int style, FILE *out, ATAB *table, int mask)
{
  int first = -1;
  
  while(table) {
    int count = table->count;
    int offset = table->offset;
    int shift = table->shift;
    int *statuses = table->statuses;
    int i = 0;

    for(i=1; i<=count; i++) {
      int atom = i+offset;
      int status = statuses[i];

      if(status & mask)
	switch(style) {
	case STYLE_READABLE:
	case STYLE_GNT:
	  if((mask & MARK_TRUE) & status) {
	    if(first) first = 0; else fprintf(out, ", ");
	    write_atom(style, out, atom, table);
	  }
	  if((mask & MARK_FALSE) & status) {
	    if(first) first = 0; else fprintf(out, ", ");
	    fprintf(out, "not ");
	    write_atom(style, out, atom, table);
	  }
	  if((mask & MARK_INPUT) & status) {
	    if(style == STYLE_READABLE) {
	      if(first) first = 0; else fprintf(out, ", ");
	      write_atom(style, out, atom, table);
	    } else {
	      fprintf(out, "{");
	      write_atom(style, out, atom, table);
	      fprintf(out, ",");
	      write_atom(style, out, atom, table);
	      fprintf(out, "'}.\n");
	    }
	  }
	  break;

	case STYLE_SMODELS:
	  fprintf(out, "%i\n", atom+shift);
	  break;

	case STYLE_ASPIF:
	  if((mask & MARK_TRUE) & status)
	    fprintf(out, "6 1 %i\n", atom+shift);
	  if((mask & MARK_FALSE) & status)
	    fprintf(out, "6 1 -%i\n", atom+shift);
	  if((mask & MARK_INPUT) & status)
	    fprintf(out, "5 %i 0\n", atom+shift);
	  break;
	  
	case STYLE_DLV:
	  if((mask & MARK_TRUE) & status) {
	    fprintf(out, ":- not ");
	    write_atom(style, out, atom, table);
	    fprintf(out, ".\n");
	  }
	  if((mask & MARK_FALSE) & status) {
	    fprintf(out, ":- ");
	    write_atom(style, out, atom, table);
	    fprintf(out, ".\n");
	  }
	  if((mask & MARK_INPUT) & status) {
	    write_atom(style, out, atom, table);
	    fprintf(out, " v int");
	    write_atom(style, out, atom, table);
	    fprintf(out, ".\n");
	  }
	  break;

	}
    }
    table = table->next;
  }
  return;
}

void write_input(int style, FILE *out, ATAB *table)
{
  ATAB *scan = table;
  int head_count = 0;
  int i = 0;

  /* Count head atoms */

  while(scan) {
    int count = scan->count;
    int offset = scan->offset;
    int shift = scan->shift;
    SYMBOL **names = scan->names;
    int *statuses = scan->statuses;

    for(i=1; i<=count; i++) 
      if(names[i] && (statuses[i] & MARK_INPUT))
	head_count++;

    scan = scan->next;
  }

  if(head_count) {

    if(style == STYLE_SMODELS)
      fprintf(out, "3 %i", head_count);
    else if(style == STYLE_ASPIF)
      fprintf(out, "1 1 %i", head_count);
    else
      fprintf(out, "{ ");

    scan = table;

    while(scan) {
      int count = scan->count;
      int offset = scan->offset;
      int shift = scan->shift;
      SYMBOL **names = scan->names;
      int *statuses = scan->statuses;
      int i = 0;

      for(i=1; i<=count; i++) {
	int atom = i+offset;

	if(names[i] && (statuses[i] & MARK_INPUT)) {
	  if(style == STYLE_ASPIF)
	    fputs(" ", out);
	  write_atom(style, out, atom, table);
	  if(style == STYLE_READABLE && (--head_count))
	    fprintf(out, ", ");
	}
      }

      scan = scan->next;
    }

    if(style == STYLE_READABLE)
      fprintf(out, " }.\n");
    else if(style == STYLE_SMODELS || style == STYLE_ASPIF)
      fprintf(out, " 0 0\n");
  }

  return;

}

/* ------------------ Support for DIMACS cnf/wcnf format ------------------- */

void write_classical_atom(int style, FILE *out, int atom, ATAB *table)
{
  ATAB *piece = find_atom(table, atom);

  if(piece) {
    int offset = piece->offset;
    int shift = piece->shift;
    SYMBOL **names = piece->names;
    SYMBOL *name = names[atom-offset];

    switch(style) {
    case STYLE_READABLE:
    case STYLE_GNT:
    case STYLE_DLV:

      if(name)
	write_name(out, name, piece->prefix, piece->postfix);
      else
	fprintf(out, "_%i", atom+shift);
      break;

    case STYLE_DIMACS:
      fprintf(out, "%i", atom+shift);
      break;

    default:
      fprintf(stderr, "%s: unknown style %i for _%i\n",
	      program_name, style, atom);
      exit(-1);
    }
  } else {
    fprintf(stderr, "%s: entry #%i out of table\n", program_name, atom);
    exit(-1);
  }

  return;
}

void write_other_classical_atom(int style, FILE *out, int atom, ATAB *table)
{
  ATAB *piece = find_atom(table, atom);

  if(piece) {
    int offset = piece->offset;
    ATAB *other = piece->other;
    int *others = piece->others;

    if(!other || !others || !others[atom-offset]) {
      fprintf(stderr, "%s: missing cross reference for ", program_name);
      write_classical_atom(style, stderr, atom, table);
      fprintf(stderr, "\n");
      exit(-1);
    } else
      write_classical_atom(style, out, others[atom-offset], other);

  } else {
    fprintf(stderr, "%s: entry #%i out of table\n", program_name, atom);
    exit(-1);
  }

  return;
}


void write_classical_literal_list(int style, FILE *out,
				  int pos_cnt, int *pos,
				  int neg_cnt, int *neg,
				  ATAB *table)
{
  int *scan = NULL;
  int *last = NULL;

  for(scan = neg, last = &neg[neg_cnt];
      scan != last; ) {
    fprintf(out, "-");
    write_classical_atom(style, out, *scan, table);
    scan++;
    if(scan != last || pos_cnt) {
      if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
	fputs(" |", out);
      fputs(" ", out);
    }
  }

  for(scan = pos, last = &pos[pos_cnt];
      scan != last; ) {
    write_classical_atom(style, out, *scan, table);
    scan++;
    if(scan != last) {
      if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
	fputs(" |", out);
      fputs(" ", out);
    }
  }

  return;
}

void write_clause(int style, FILE *out, RULE *cnf, ATAB *table)
{
  int type = cnf->type;
  CLAUSE *clause = cnf->data.clause;
  int pos_cnt = clause->pos_cnt;
  int neg_cnt = clause->neg_cnt;
  long weight = clause->weight;

  if(type != TYPE_CLAUSE) {
    fprintf(stderr, "%s: only clauses are supported by cnf routines!\n",
	    program_name);
    exit(-1);
  }

  if(weight && style == STYLE_DIMACS)
    fprintf(out, "%li ", weight);

  write_classical_literal_list(style, out,
			       pos_cnt, clause->pos,
			       neg_cnt, clause->neg,
			       table);

  if(weight && style == STYLE_READABLE)
    fprintf(out, " = %li", weight);

  if(style == STYLE_READABLE || style == STYLE_GNT || style == STYLE_DLV)
    fprintf(out, ".\n");
  else if(style == STYLE_DIMACS)
    fprintf(out, " 0\n");

  return;
}

void write_cnf(int style, FILE *out, RULE *cnf, ATAB *table)
{
  while(cnf) {
    write_clause(style, out, cnf, table);
    cnf = cnf->next;
  }
  return;
}

/* ------------------ Free the memory taken by a program ------------------ */

/* See input.c for to understand how memory was allocated */

void free_basic(BASIC_RULE *basic)
{
  free(basic->neg);
  free(basic);

  return;
}

void free_constraint(CONSTRAINT_RULE *constraint)
{
  free(constraint->neg);
  free(constraint);

  return;
}

void free_choice(CHOICE_RULE *choice)
{
  free(choice->neg);
  free(choice->head);
  free(choice);

  return;
}

void free_integrity(INTEGRITY_RULE *integrity)
{
  free(integrity->neg);
  free(integrity);

  return;
}

void free_weight(WEIGHT_RULE *weight)
{
  free(weight->neg);
  free(weight);

  return;
}

void free_optimize(OPTIMIZE_RULE *optimize)
{
  free(optimize->neg);
  free(optimize);

  return;
}

void free_disjunctive(DISJUNCTIVE_RULE *disjunctive)
{
  free(disjunctive->neg);
  free(disjunctive->head);
  free(disjunctive);

  return;
}

void free_clause(CLAUSE *clause)
{
  if(clause->neg)
    free(clause->neg);
  if(clause->pos)
    free(clause->pos);
  free(clause);

  return;
}

void free_rule(RULE *rule)
{
  switch(rule->type) {
  case TYPE_BASIC:
    free_basic(rule->data.basic);
    break;

  case TYPE_CONSTRAINT:
    free_constraint(rule->data.constraint);
    break;

  case TYPE_CHOICE:
    free_choice(rule->data.choice);
    break;

  case TYPE_INTEGRITY:
    free_integrity(rule->data.integrity);
    break;

  case TYPE_WEIGHT:
    free_weight(rule->data.weight);
    break;

  case TYPE_OPTIMIZE:
    free_optimize(rule->data.optimize);
    break;

  case TYPE_DISJUNCTIVE:
    free_disjunctive(rule->data.disjunctive);
    break;

  case TYPE_CLAUSE:
    free_clause(rule->data.clause);
    break;
  }

  free((void *)rule);

  return;
}

void free_program(RULE *program)
{
  while(program) {

    RULE *rule = program;

    program = rule->next;
    rule->next = NULL;

    free_rule(rule);
  }

  return;
}
