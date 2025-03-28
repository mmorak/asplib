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
 * Input routines for a number of programs manipulating smodels programs
 *
 * (c) 2002-2023 Tomi Janhunen
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "version.h"
#include "symbol.h"
#include "atom.h"
#include "rule.h"
#include "io.h"

int max_atom = 0;
long max_weight = 0;

/* --------------------- Print version information ------------------------- */

void _version_io_h()
{
  _version(_IO_H_RCSFILE, _IO_H_DATE, _IO_H_REVISION);
}

void _version_input_c()
{
  _version_io_h();
  _version("$RCSfile: input.c,v $",
	   "$Date: 2023/02/25 13:26:44 $",
	   "$Revision: 1.33 $");
}

/* ------------------------- Low level routines ---------------------------- */

char *program_name = NULL;

void error(char *msg)
{
  if(program_name)
    fprintf(stderr, "%s: %s\n", program_name, msg);
  else
    fprintf(stderr, "error: %s\n", msg);
  exit(-1);
}

#define BUFSIZE 1024

typedef struct bufseq {
  char *buf;
  struct bufseq *next;
} BUFSEQ;

char *read_string(FILE *in)
{
  BUFSEQ *bs = (BUFSEQ *)malloc(sizeof(BUFSEQ));
  BUFSEQ *first = bs;
  int i = 0;
  int ch = 0;
  int len = 0;
  char *result = NULL;

  bs->buf = (char *)malloc(BUFSIZE);
  bs->next = NULL;

  ch = fgetc(in);
  while(isspace(ch))
    ch = fgetc(in);
  if(ch != EOF)
    ungetc(ch, in);

  while((ch = fgetc(in)) != EOF && !isspace(ch)) {
    if(i==BUFSIZE) {
      bs->next = (BUFSEQ *)malloc(sizeof(BUFSEQ));
      bs = bs->next;
      bs->buf = (char *)malloc(BUFSIZE);
      bs->next = NULL;
      i = 0;
    }
    (bs->buf)[i++] = (char) ch;
    len++;
  }
  if(isspace(ch))
    ungetc(ch, in);

  if(ch == EOF) {
    result = NULL;

    while(first != bs) {
      BUFSEQ *prev = first;

      first = first->next;
      free(prev->buf);
      free(prev);
    }
    free(first->buf);
    free(first);

  } else {
    char *scan = (char *)malloc(len+1);

    result = scan;
    while(first != bs) {
      BUFSEQ *prev = first;

      memcpy(scan, first->buf, BUFSIZE);
      scan = &scan[BUFSIZE];
      first = first->next;

      free(prev->buf);
      free(prev);
    }
    memcpy(scan, first->buf, i);
    scan[i] = '\0';
    free(first->buf);
    free(first);
  }

  return result;
}

int read_atom(FILE *in, char *msg)
{
  int atom = 0;

  if(fscanf(in, " %i", &atom) != 1) error(msg);

  if(atom>max_atom)
    max_atom = atom;

  return atom;
}

void read_atom_list(FILE *in, int cnt, int *table, char *msg)
{
  int i = 0;
  int atom = 0;

  for(i=0; i<cnt; i++) {
    if(fscanf(in, " %i", &atom) != 1) error(msg);

    table[i] = atom;
    if(atom>max_atom)
      max_atom = atom;
  }

  return;
}

void read_weight_list(FILE *in, int cnt, int *table, char *msg)
{
  int i = 0;
  int weight = 0;

  for(i=0; i<cnt; i++) {
    if(fscanf(in, " %i", &weight) != 1) error(msg);

    table[i] = weight;
  }

  return;
}

/* --------------------- Read in a smodels program ------------------------- */

RULE *read_basic(FILE *in)
{
  int head = 0;
  int atom = 0;
  int lit_cnt = 0;
  int neg_cnt = 0;
  int pos_cnt = 0;
  int *table = NULL;
  int i = 0;

  RULE *new = (RULE *)malloc(sizeof(RULE));
  BASIC_RULE *basic = (BASIC_RULE *)malloc(sizeof(BASIC_RULE));

  new->type = TYPE_BASIC;
  new->data.basic = basic;
  new->next = NULL;

  basic->head = read_atom(in, "basic rule, missing head");

  if(fscanf(in, " %i", &lit_cnt) != 1)
    error("basic rule, missing literal count");
  if(fscanf(in, " %i", &neg_cnt) != 1)
    error("basic rule, missing negative count");

  pos_cnt = lit_cnt - neg_cnt;
  if(pos_cnt < 0)
    error("basic rule, invalid positive count");

  table = (int *)malloc(lit_cnt * sizeof(int));

  basic->neg_cnt = neg_cnt;
  basic->pos_cnt = pos_cnt;
  basic->neg = table;
  basic->pos = &table[neg_cnt];

  read_atom_list(in, neg_cnt, basic->neg,
		 "basic rule, missing negative literal");

  read_atom_list(in, pos_cnt, basic->pos,
		 "basic rule, missing positive literal");

  return new;
}

RULE *read_constraint(FILE *in)
{
  int head = 0;
  int bound = 0;
  int atom = 0;
  int lit_cnt = 0;
  int neg_cnt = 0;
  int pos_cnt = 0;
  int *table = NULL;
  int i = 0;

  RULE *new = (RULE *)malloc(sizeof(RULE));
  CONSTRAINT_RULE *constraint =
    (CONSTRAINT_RULE *)malloc(sizeof(CONSTRAINT_RULE));

  new->type = TYPE_CONSTRAINT;
  new->data.constraint = constraint;
  new->next = NULL;

  constraint->head = read_atom(in, "constraint rule, missing head");

  if(fscanf(in, " %i", &lit_cnt) != 1)
    error("constraint rule, missing literal count");
  if(fscanf(in, " %i", &neg_cnt) != 1)
    error("constraint rule, missing negative count");
  if(fscanf(in, " %i", &bound) != 1)
    error("constraint rule, missing bound");

  constraint->bound = bound;

  pos_cnt = lit_cnt - neg_cnt;
  if(pos_cnt < 0)
    error("constraint rule, invalid positive count");

  table = (int *)malloc(lit_cnt * sizeof(int));

  constraint->neg_cnt = neg_cnt;
  constraint->neg = table;
  constraint->pos_cnt = pos_cnt;
  constraint->pos = &table[neg_cnt];

  read_atom_list(in, neg_cnt, constraint->neg,
		 "constraint rule, missing negative literal");

  read_atom_list(in, pos_cnt, constraint->pos,
		 "constraint rule, missing positive literal");

  return new;
}

RULE *read_choice(FILE *in)
{
  int head_cnt = 0;
  int atom = 0;
  int lit_cnt = 0;
  int neg_cnt = 0;
  int pos_cnt = 0;
  int *table = NULL;
  int i = 0;

  RULE *new = (RULE *)malloc(sizeof(RULE));
  CHOICE_RULE *choice = (CHOICE_RULE *)malloc(sizeof(CHOICE_RULE));

  new->type = TYPE_CHOICE;
  new->data.choice = choice;
  new->next = NULL;

  if(fscanf(in, " %i", &head_cnt) != 1)
    error("choice rule, missing head count");

  table = (int *)malloc(head_cnt * sizeof(int));

  choice->head_cnt = head_cnt;
  choice->head = table;

  read_atom_list(in, head_cnt, choice->head,
		 "choice rule, missing head atom");

  if(fscanf(in, " %i", &lit_cnt) != 1)
    error("choice rule, missing literal count");
  if(fscanf(in, " %i", &neg_cnt) != 1)
    error("choice rule, missing negative count");

  pos_cnt = lit_cnt - neg_cnt;
  if(pos_cnt < 0)
    error("choice rule, invalid positive count");

  table = (int *)malloc(lit_cnt * sizeof(int));

  choice->neg_cnt = neg_cnt;
  choice->neg = table;
  choice->pos_cnt = pos_cnt;
  choice->pos = &table[neg_cnt];

  read_atom_list(in, neg_cnt, choice->neg,
		 "choice rule, missing negative literal");

  read_atom_list(in, pos_cnt, choice->pos,
		 "choice rule, missing positive literal");

  return new;
}

RULE *read_integrity(FILE *in)
{
  int atom = 0;
  int lit_cnt = 0;
  int neg_cnt = 0;
  int pos_cnt = 0;
  int *table = NULL;
  int i = 0;

  RULE *new = (RULE *)malloc(sizeof(RULE));
  INTEGRITY_RULE *integrity = (INTEGRITY_RULE *)malloc(sizeof(INTEGRITY_RULE));

  new->type = TYPE_INTEGRITY;
  new->data.integrity = integrity;
  new->next = NULL;

  if(fscanf(in, " %i", &lit_cnt) != 1)
    error("integrity rule, missing literal count");
  if(fscanf(in, " %i", &neg_cnt) != 1)
    error("integrity rule, missing negative count");

  pos_cnt = lit_cnt - neg_cnt;
  if(pos_cnt < 0)
    error("integrity rule, invalid positive count");

  table = (int *)malloc(lit_cnt * sizeof(int));

  integrity->neg_cnt = neg_cnt;
  integrity->neg = table;
  integrity->pos_cnt = pos_cnt;
  integrity->pos = &table[neg_cnt];

  read_atom_list(in, neg_cnt, integrity->neg,
		 "integrity rule, missing negative literal");

  read_atom_list(in, pos_cnt, integrity->pos,
		 "integrity rule, missing positive literal");

  return new;
}

RULE *read_weight(FILE *in)
{
  int head = 0;
  int bound = 0;
  int atom = 0;
  int lit_cnt = 0;
  int neg_cnt = 0;
  int pos_cnt = 0;
  int *table = NULL;
  int i = 0;

  RULE *new = (RULE *)malloc(sizeof(RULE));
  WEIGHT_RULE *weight =
    (WEIGHT_RULE *)malloc(sizeof(WEIGHT_RULE));

  new->type = TYPE_WEIGHT;
  new->data.weight = weight;
  new->next = NULL;

  weight->head = read_atom(in, "weight rule, missing head");

  if(fscanf(in, " %i", &bound) != 1)
    error("weight rule, missing bound");

  weight->bound = bound;

  if(fscanf(in, " %i", &lit_cnt) != 1)
    error("weight rule, missing literal count");
  if(fscanf(in, " %i", &neg_cnt) != 1)
    error("weight rule, missing negative count");

  pos_cnt = lit_cnt - neg_cnt;
  if(pos_cnt < 0)
    error("weight rule, invalid positive count");

  table = (int *)malloc(2 * lit_cnt * sizeof(int));

  weight->neg_cnt = neg_cnt;
  weight->neg = table;
  weight->pos_cnt = pos_cnt;
  weight->pos = &table[neg_cnt];
  weight->weight = &table[neg_cnt+pos_cnt];

  read_atom_list(in, neg_cnt, weight->neg,
		 "weight rule, missing negative literal");

  read_atom_list(in, pos_cnt, weight->pos,
		 "weight rule, missing positive literal");

  read_weight_list(in, lit_cnt, weight->weight,
		   "weight rule, missing weight");

  return new;
}

RULE *read_optimize(FILE *in)
{
  int bound = -1;
  int atom = 0;
  int lit_cnt = 0;
  int neg_cnt = 0;
  int pos_cnt = 0;
  int *table = NULL;
  int i = 0;

  RULE *new = (RULE *)malloc(sizeof(RULE));
  OPTIMIZE_RULE *optimize =
    (OPTIMIZE_RULE *)malloc(sizeof(OPTIMIZE_RULE));

  new->type = TYPE_OPTIMIZE;
  new->data.optimize = optimize;
  new->next = NULL;

  if(fscanf(in, " %i", &bound) != 1 || bound != 0)
    error("optimize statement, missing 0 field");

  if(fscanf(in, " %i", &lit_cnt) != 1)
    error("optimize statement, missing literal count");
  if(fscanf(in, " %i", &neg_cnt) != 1)
    error("optimize statement, missing negative count");

  pos_cnt = lit_cnt - neg_cnt;
  if(pos_cnt < 0)
    error("optimize statement, invalid positive count");

  table = (int *)malloc(2 * lit_cnt * sizeof(int));

  optimize->neg_cnt = neg_cnt;
  optimize->neg = table;
  optimize->pos_cnt = pos_cnt;
  optimize->pos = &table[neg_cnt];
  optimize->weight = &table[neg_cnt+pos_cnt];

  read_atom_list(in, neg_cnt, optimize->neg,
		 "optimize statement, missing negative literal");

  read_atom_list(in, pos_cnt, optimize->pos,
		 "optimize statement, missing positive literal");

  read_weight_list(in, lit_cnt, optimize->weight,
		   "optimize statement, missing weight");

  return new;
}

RULE *read_disjunctive(FILE *in)
{
  int head_cnt = 0;
  int atom = 0;
  int lit_cnt = 0;
  int neg_cnt = 0;
  int pos_cnt = 0;
  int *table = NULL;
  int i = 0;

  RULE *new = (RULE *)malloc(sizeof(RULE));
  DISJUNCTIVE_RULE *disjunctive =
    (DISJUNCTIVE_RULE *)malloc(sizeof(DISJUNCTIVE_RULE));

  new->type = TYPE_DISJUNCTIVE;
  new->data.disjunctive = disjunctive;
  new->next = NULL;

  if(fscanf(in, " %i", &head_cnt) != 1)
    error("disjunctive rule, missing head count");

  table = (int *)malloc(head_cnt * sizeof(int));

  disjunctive->head_cnt = head_cnt;
  disjunctive->head = table;

  read_atom_list(in, head_cnt, disjunctive->head,
		 "disjunctive rule, missing head atom");

  if(fscanf(in, " %i", &lit_cnt) != 1)
    error("disjunctive rule, missing literal count");
  if(fscanf(in, " %i", &neg_cnt) != 1)
    error("disjunctive rule, missing negative count");

  pos_cnt = lit_cnt - neg_cnt;
  if(pos_cnt < 0)
    error("disjunctive rule, invalid positive count");

  table = (int *)malloc(lit_cnt * sizeof(int));

  disjunctive->neg_cnt = neg_cnt;
  disjunctive->neg = table;
  disjunctive->pos_cnt = pos_cnt;
  disjunctive->pos = &table[neg_cnt];

  read_atom_list(in, neg_cnt, disjunctive->neg,
		 "disjunctive rule, missing negative literal");

  read_atom_list(in, pos_cnt, disjunctive->pos,
		 "disjunctive rule, missing positive literal");

  return new;
}

void initialize_program()
{
  max_atom = 0;
}

RULE *read_rule(FILE *in)
{
  int type = 0;
  RULE *rule = NULL;

  if(fscanf(in, " %i", &type) != 1)
    error("unknown rule type");

  switch(type) {
  case TYPE_BASIC:
    rule = read_basic(in);
    if(!rule)
      error("erroneous basic rule");
    break;

  case TYPE_CONSTRAINT:
    rule = read_constraint(in);
    if(!rule)
      error("erroneous constraint rule");
    break;

  case TYPE_CHOICE:
    rule = read_choice(in);
    if(!rule)
      error("erroneous choice rule");
    break;

  case TYPE_INTEGRITY:
    rule = read_integrity(in);
    if(!rule)
      error("erroneous integrity rule");
    break;

  case TYPE_WEIGHT:
    rule = read_weight(in);
    if(!rule)
      error("erroneous weight rule");
    break;

  case TYPE_OPTIMIZE:
    rule = read_optimize(in);
    if(!rule)
      error("erroneous optimize statement");
    break;

  case TYPE_ORDERED:
    error("ordered disjunctive rules are not supported");
    break;

  case TYPE_DISJUNCTIVE:
    rule = read_disjunctive(in);
    if(!rule)
      error("erroneous disjunctive rule");
    break;

  case 0:
    break;

  default:
    error("unknown rule type");
    break;
  }

  return rule;
}

RULE *read_program(FILE *in)
{
  int type = 0;
  RULE *program = NULL;
  RULE *new = NULL;
  RULE *last = NULL;

  initialize_program();

  if(fscanf(in, " %i", &type) != 1)
    error("unknown rule type");

  while(type != 0) {
    switch(type) {
    case TYPE_BASIC:
      new = read_basic(in);
      if(!new)
	error("erroneous basic rule");
      if(last)
	last->next = new;
      else
	program = new;
      last=new;
      break;

    case TYPE_CONSTRAINT:
      new = read_constraint(in);
      if(!new)
	error("erroneous constraint rule");
      if(last)
	last->next = new;
      else
	program = new;
      last=new;
      break;

    case TYPE_CHOICE:
      new = read_choice(in);
      if(!new)
	error("erroneous choice rule");
      if(last)
	last->next = new;
      else
	program = new;
      last=new;
      break;

    case TYPE_INTEGRITY:
      new = read_integrity(in);
      if(!new)
	error("erroneous integrity rule");
      if(last)
	last->next = new;
      else
	program = new;
      last=new;
      break;

    case TYPE_WEIGHT:
      new = read_weight(in);
      if(!new)
	error("erroneous weight rule");
      if(last)
	last->next = new;
      else
	program = new;
      last=new;
      break;

    case TYPE_OPTIMIZE:
      new = read_optimize(in);
      if(!new)
	error("erroneous optimize statement");
      if(last)
	last->next = new;
      else
	program = new;
      last=new;
      break;

    case TYPE_ORDERED:
      error("ordered disjunctive rules are not supported");
      break;

    case TYPE_DISJUNCTIVE:
      new = read_disjunctive(in);
      if(!new)
	error("erroneous disjunctive rule");
      if(last)
	last->next = new;
      else
	program = new;
      last=new;
      break;

    default:
    case 0:
      break;
    }

    if(fscanf(in, " %i", &type) != 1)
      error("unknown rule type");
  }
  
  return program;
}

/* ---------------------------- Read in symbols --------------------------- */

ATAB *read_symbols(FILE *in)
{
  int offset = 0;
  ATAB *table = new_table(max_atom, offset);
  ASTACK *missing = NULL;
  int atom = 0;

  if(fscanf(in, " %i", &atom) != 1)
    error("missing symbol table entry");

  while(atom) {
    char *name = NULL;

    if((name = read_string(in)) == NULL || strlen(name) == 0)
      error("missing symbol name");

    if(!set_name(table, atom, name))
      missing = push(atom, 0, name, missing);

    if(fscanf(in, " %i", &atom) != 1)
      error("missing symbol table entry");
  }

  /* Extend symbol table to cover missing atoms (a patch) */

  if(missing) { 
    ATAB *piece = NULL;
    int min = missing->atom;
    int max = missing->atom;
    ASTACK *scan = missing->under;

    while(scan) {
      int scanned = scan->atom;

      if(scanned<min) min=scanned;
      if(scanned>max) max=scanned;
      scan = scan->under;
    }

    piece = extend_table(table, max-min+1, min-1);

    while(missing) {
      int atom = 0;
      char *name = NULL;
      SYMBOL **names = piece->names;

      missing = pop(&atom, NULL, &name, missing);
      names[atom-(min-1)] = find_symbol(name);
    }
  }

  return table;
}

/* ----------------------- Read in compute statement ---------------------- */

int read_compute_statement(FILE *in, ATAB *table)
{
  int count = table->count;
  ASTACK *missing = NULL;
  int atom = 0;
  int number = 0;
  int ch = 0;

  /* Read in the positive part (must exist) */

  while((ch = fgetc(in)) != 'B' && ch != EOF);
  while((ch = fgetc(in)) != '+' && ch != EOF);

  if(ch == EOF)
    error("missing (positive) compute statement");

  if(fscanf(in, " %i", &atom) != 1)
    error("incomplete (positive) compute statement");
  
  while(atom) {
    if(!set_status(table, atom, MARK_TRUE))
      missing = push(atom, MARK_TRUE, NULL, missing);

    if(fscanf(in, " %i", &atom) != 1)
      error("incomplete (positive) compute statement");
  }

  /* Read in the negative part (must exist) */

  while((ch = fgetc(in)) != 'B' && ch != EOF);
  while((ch = fgetc(in)) != '-' && ch != EOF);

  if(ch == EOF)
    error("missing (negative) compute statement");

  if(fscanf(in, " %i", &atom) != 1)
    error("incomplete (negative) compute statement");
  
  while(atom) {
    if(!set_status(table, atom, MARK_FALSE))
      missing = push(atom, MARK_FALSE, NULL, missing);

    if(fscanf(in, " %i", &atom) != 1)
      error("incomplete (negative) compute statement");
  }

  /* Check for a declaration of input atoms (optional) */

  while(!isalnum(ch = fgetc(in)) && ch != EOF);

  if(ch == 'E') {

    if(fscanf(in, " %i", &atom) != 1)
      error("incomplete input specification");

    while(atom) {
      if(!set_status(table, atom, MARK_INPUT))
	missing = push(atom, MARK_INPUT, NULL, missing);

      if(fscanf(in, " %i", &atom) != 1)
	error("incomplete input specification");
    }

  } else if(ch != EOF)
    ungetc(ch, in);

  /* Read in the number of models to be computed */

  if(fscanf(in, " %i\n", &number) != 1)
    error("missing number of models");

  /* Extend symbol table to cover missing atoms (a patch) */

  if(missing) { 
    ATAB *piece = NULL;
    int min = missing->atom;
    int max = missing->atom;
    ASTACK *scan = missing->under;

    while(scan) {
      if(scan->atom<min) min=atom;
      if(scan->atom>max) max=atom;
      scan = scan->under;
    }

    piece = extend_table(table, max-min+1, min-1);

    while(missing) {
      int atom = 0;
      int status = 0;
      int *statuses = piece->statuses;

      missing = pop(&atom, &status, NULL, missing);
      statuses[atom-(min-1)] |= status;
    }
  }

  return number;
}

/* --------------------- Support for DIMACS cnf format --------------------- */

void read_literals(FILE *in, CLAUSE *clause, int weighted)
{
  int literal = 0;

  if(weighted) {
    long weight = 0;

    if(fscanf(in, " %li", &weight) !=1)
      error("clause, missing weight");

    if(weight<=0)
      error("clause, non-positive weight");
    else
      clause->weight = weight;
  }

  if(fscanf(in, " %i", &literal) != 1)
    error("clause, missing literal");

  /* The number of literals is not known in advance;
     thus we make recursive calls until end of clause "0" is encountered */

  if(literal == 0) {
    int pos_cnt = clause->pos_cnt;
    int neg_cnt = clause->neg_cnt;

    if(pos_cnt) {
      clause->pos = (int *)malloc(sizeof(int)*pos_cnt);
    }

    if(neg_cnt) {
      clause->neg = (int *)malloc(sizeof(int)*neg_cnt);
    }

  } else if(literal > 0) {
    int i = clause->pos_cnt++;
    
    read_literals(in, clause, 0);
    (clause->pos)[i] = literal;

  } else { /* literal < 0 */
    int i = clause->neg_cnt++;
    
    read_literals(in, clause, 0);
    (clause->neg)[i] = -literal;

  }

  return;
}

RULE *read_clause(FILE *in, int weighted)
{
  RULE *new = (RULE *)malloc(sizeof(RULE));
  CLAUSE *clause = (CLAUSE *)malloc(sizeof(CLAUSE));

  new->type = TYPE_CLAUSE;
  new->data.clause = clause;
  new->next = NULL;

  clause->pos_cnt = 0;
  clause->pos = NULL;

  clause->neg_cnt = 0;
  clause->neg = NULL;

  clause->weight = 0;

  read_literals(in, clause, weighted);

  return new;
}

ATAB *initialize_cnf(FILE *in, int *clauses, int *weighted)
{
  int ch = 0;
  int vars = 0;
  ATAB *table = NULL;
  int failed = 0;
  long max = 0;
  int items = 0;

  while((ch = getc(in)) == 'c')
    while(ch != '\n')
      ch = getc(in);

  if(ch != 'p')
    failed = -1;
  else
    ch = getc(in);

  if(ch != ' ')
    failed = -1;
  else
    ch = getc(in);

  if(ch == 'w')
    *weighted = -1;
  else if(ch == 'c')
    ungetc(ch, in);
  else
    failed = -1;

  if(!failed) {
    if(*weighted)
      items = fscanf(in, "cnf %i %i %li\n", &vars, clauses, &max);
    else
      items = fscanf(in, "cnf %i %i\n", &vars, clauses);
  }

  if((!*weighted && items !=2) || (*weighted && (items<2 || items>3)) || failed)
    error("DIMACS cnf/wcnf format: missing/invalid problem line");

  table = new_table(vars, 0);
  if(*weighted && items == 3)
    max_weight = max;

  while((ch = getc(in)) == 'c') {
    int atom = 0;
    char *name = NULL;

    if((ch = getc(in)) == ' ')
      if(fscanf(in, " %i", &atom) == 1)
	if((name = read_string(in)) != NULL)
	  set_name(table, atom, name);

    while(ch != '\n')
      ch = getc(in);
  }

  ungetc(ch, in);

  /* Ready to read in clauses one by one using read_clause */
  
  return table;
}

RULE *read_cnf(FILE *in, ATAB **table, int *weighted)
{
  int ch = 0;
  int vars = 0, clauses = 0;
  int atom = 0;
  int log = 1;
  int failed = 0;
  long max = 0;
  int items = 0;

  RULE *cnf = NULL;
  RULE *new = NULL;
  RULE *last = NULL;

  SYMBOL **names = NULL;
  int *statuses = NULL;

  while((ch = getc(in)) == 'c')
    while(ch != '\n')
      ch = getc(in);

  if(ch != 'p')
    failed = -1;
  else
    ch = getc(in);

  if(ch != ' ')
    failed = -1;
  else
    ch = getc(in);

  if(ch == 'w')
    *weighted = -1;
  else if(ch == 'c')
    ungetc(ch, in);
  else
    failed = -1;

  if(!failed) {
    if(*weighted)
      items = fscanf(in, "cnf %i %i %li\n", &vars, &clauses, &max);
    else
      items = fscanf(in, "cnf %i %i\n", &vars, &clauses);
  }

  if((!*weighted && items !=2) || (*weighted && (items<2 || items>3)) || failed)
    error("DIMACS cnf/wcnf format: missing/invalid problem line");
    
  *table = new_table(vars, 0);
  names = (*table)->names;
  statuses = (*table)->statuses;
  if(*weighted && items == 3)
    max_weight = max;

  while((ch = getc(in)) == 'c') {
    int atom = 0;
    char *name = NULL;

    if((ch = getc(in)) == ' ')
      if(fscanf(in, " %i", &atom) == 1)
	if((name = read_string(in)) != NULL)
	  set_name(*table, atom, name);

    while(ch != '\n')
      ch = getc(in);
  }

  ungetc(ch, in);

  while((clauses--)>0) {
    new = read_clause(in, *weighted);
    if(cnf == NULL) {
      cnf = new;
      last = cnf;
    } else {
      last->next = new;
      last = new;
    }
  }  

  return cnf;
}
