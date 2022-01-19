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
 * Various utilities related to rules and programs
 *
 * (c) 2002-2010 Tomi Janhunen
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "version.h"
#include "symbol.h"
#include "atom.h"
#include "rule.h"
#include "io.h"

/* --------------------- Print version information ------------------------- */

void _version_rule_h()
{
  _version(_RULE_H_RCSFILE, _RULE_H_DATE, _RULE_H_REVISION);
}

void _version_rule_c()
{
  _version_rule_h();
  _version("$RCSfile: rule.c,v $",
	   "$Date: 2021/05/27 07:55:38 $",
	   "$Revision: 1.17 $");
}

/* ----------------------- Access to data structures ----------------------- */

int get_head(RULE *r)
{
  switch(r->type) {
  case TYPE_BASIC:       return r->data.basic->head;
  case TYPE_CONSTRAINT:  return r->data.constraint->head;
  case TYPE_WEIGHT:      return r->data.weight->head;
  case TYPE_CHOICE:
  case TYPE_INTEGRITY:
  case TYPE_DISJUNCTIVE:
  case TYPE_CLAUSE:
  case TYPE_OPTIMIZE:
  case TYPE_ORDERED:
  default:
    return 0;
  }
}

int *get_heads(RULE *r)
{
  switch(r->type) {
  case TYPE_BASIC:       return &(r->data.basic->head);
  case TYPE_CONSTRAINT:  return &(r->data.constraint->head);
  case TYPE_WEIGHT:      return &(r->data.weight->head);
  case TYPE_CHOICE:      return r->data.choice->head;
  case TYPE_DISJUNCTIVE: return r->data.disjunctive->head;
  case TYPE_INTEGRITY:
  case TYPE_CLAUSE:
  case TYPE_OPTIMIZE:
  case TYPE_ORDERED:
  default:
    return NULL;
  }
}

int get_head_cnt(RULE *r)
{
  switch(r->type) {
  case TYPE_BASIC:       return 1;
  case TYPE_CONSTRAINT:  return 1;
  case TYPE_WEIGHT:      return 1;
  case TYPE_CHOICE:      return r->data.choice->head_cnt;
  case TYPE_DISJUNCTIVE: return r->data.choice->head_cnt;
  case TYPE_INTEGRITY:
  case TYPE_CLAUSE:
  case TYPE_OPTIMIZE:
  case TYPE_ORDERED:
  default:
    return 0;
  }
}

int *get_pos(RULE *r)
{
  switch(r->type) {
  case TYPE_BASIC:       return r->data.basic->pos;
  case TYPE_CONSTRAINT:  return r->data.constraint->pos;
  case TYPE_CHOICE:      return r->data.choice->pos;
  case TYPE_INTEGRITY:   return r->data.integrity->pos;
  case TYPE_WEIGHT:      return r->data.weight->pos;
  case TYPE_DISJUNCTIVE: return r->data.disjunctive->pos;
  case TYPE_CLAUSE:      return r->data.clause->pos;
  case TYPE_OPTIMIZE:    return r->data.optimize->pos;
  case TYPE_ORDERED:
  default:
    return NULL;
  }
}

int get_pos_cnt(RULE *r)
{
  switch(r->type) {
  case TYPE_BASIC:       return r->data.basic->pos_cnt;
  case TYPE_CONSTRAINT:  return r->data.constraint->pos_cnt;
  case TYPE_CHOICE:      return r->data.choice->pos_cnt;
  case TYPE_INTEGRITY:   return r->data.integrity->pos_cnt;
  case TYPE_WEIGHT:      return r->data.weight->pos_cnt;
  case TYPE_DISJUNCTIVE: return r->data.disjunctive->pos_cnt;
  case TYPE_CLAUSE:      return r->data.clause->pos_cnt;
  case TYPE_OPTIMIZE:    return r->data.optimize->pos_cnt;
  case TYPE_ORDERED:
  default:
    return 0;
  }
}

int *get_neg(RULE *r)
{
  switch(r->type) {
  case TYPE_BASIC:       return r->data.basic->neg;
  case TYPE_CONSTRAINT:  return r->data.constraint->neg;
  case TYPE_CHOICE:      return r->data.choice->neg;
  case TYPE_INTEGRITY:   return r->data.integrity->neg;
  case TYPE_WEIGHT:      return r->data.weight->neg;
  case TYPE_DISJUNCTIVE: return r->data.disjunctive->neg;
  case TYPE_CLAUSE:      return r->data.clause->neg;
  case TYPE_OPTIMIZE:    return r->data.optimize->neg;
  case TYPE_ORDERED:
  default:
    return NULL;
  }
}

int get_neg_cnt(RULE *r)
{
  switch(r->type) {
  case TYPE_BASIC:       return r->data.basic->neg_cnt;
  case TYPE_CONSTRAINT:  return r->data.constraint->neg_cnt;
  case TYPE_CHOICE:      return r->data.choice->neg_cnt;
  case TYPE_INTEGRITY:   return r->data.integrity->neg_cnt;
  case TYPE_WEIGHT:      return r->data.weight->neg_cnt;
  case TYPE_DISJUNCTIVE: return r->data.disjunctive->neg_cnt;
  case TYPE_CLAUSE:      return r->data.clause->neg_cnt;
  case TYPE_OPTIMIZE:    return r->data.optimize->neg_cnt;
  case TYPE_ORDERED:
  default:
    return 0;
  }
}

/* ------------ Check negative occurrences of invisible atoms -------------- */

int neg_inv_list(int cnt, int *first, ATAB *table)
{
  int *scan = first;
  int *last = &first[cnt];
  int rvalue = 0;

  while(scan != last) {
    int atom = *scan;

    if(invisible(table, atom)) {
      rvalue = -1;
      set_status(table, atom, MARK_NEGOCC);
    }
    scan++;
  }
  return rvalue;
}

int neg_inv_basic(RULE *rule, ATAB *table)
{
  BASIC_RULE *basic = rule->data.basic;

  if(basic->neg_cnt)
    return neg_inv_list(basic->neg_cnt, basic->neg, table);
  else
    return 0;
}

int neg_inv_constraint(RULE *rule, ATAB *table)
{
  CONSTRAINT_RULE *constraint = rule->data.constraint;

  if(constraint->neg_cnt)
    return neg_inv_list(constraint->neg_cnt, constraint->neg, table);
  else
    return 0;
}

int neg_inv_choice(RULE *rule, ATAB *table)
{
  CHOICE_RULE *choice = rule->data.choice;
  int rvalue = 0;

  if(choice->head_cnt)
    rvalue = neg_inv_list(choice->head_cnt, choice->head, table);

  if(choice->neg_cnt)
    rvalue |= neg_inv_list(choice->neg_cnt, choice->neg, table);

  return rvalue;
}

int neg_inv_integrity(RULE *rule, ATAB *table)
{
  INTEGRITY_RULE *integrity = rule->data.integrity;

  if(integrity->neg_cnt)
    return neg_inv_list(integrity->neg_cnt, integrity->neg, table);
  else
    return 0;
}

int neg_inv_weight(RULE *rule, ATAB *table)
{
  WEIGHT_RULE *weight = rule->data.weight;

  if(weight->neg_cnt)
    return neg_inv_list(weight->neg_cnt, weight->neg, table);
  else
    return 0;
}

int neg_inv_optimize(RULE *rule, ATAB *table)
{
  OPTIMIZE_RULE *optimize = rule->data.optimize;

  if(optimize->neg_cnt)
    return neg_inv_list(optimize->neg_cnt, optimize->neg, table);
  else
    return 0;
}

int neg_inv_disjunctive(RULE *rule, ATAB *table)
{
  DISJUNCTIVE_RULE *disjunctive = rule->data.disjunctive;
  int rvalue = 0;

  if(disjunctive->head_cnt)
    rvalue = neg_inv_list(disjunctive->head_cnt, disjunctive->head, table);

  if(disjunctive->neg_cnt)
    rvalue |= neg_inv_list(disjunctive->neg_cnt, disjunctive->neg, table);

  return rvalue;
}

int neg_inv(RULE *rule, ATAB *table)
{
  switch(rule->type) {
  case TYPE_BASIC:
    return neg_inv_basic(rule, table);
    break;

  case TYPE_CONSTRAINT:
    return neg_inv_constraint(rule, table);
    break;

  case TYPE_CHOICE:
    return neg_inv_choice(rule, table);
    break;

  case TYPE_INTEGRITY:
    return neg_inv_integrity(rule, table);
    break;

  case TYPE_WEIGHT:
    return neg_inv_weight(rule, table);
    break;

  case TYPE_OPTIMIZE:
    return neg_inv_optimize(rule, table);
    break;

  case TYPE_DISJUNCTIVE:
    return neg_inv_disjunctive(rule, table);
    break;

  default:
    fprintf(stderr, "unknown rule type %i!\n", rule->type);
    exit(-1);
    break;
  }
}

int check_negative_invisible(RULE *rule, ATAB *table)
{
  int rvalue = 0;

  while(rule) {
    rvalue |= neg_inv(rule, table);
    rule = rule->next;
  }
  return rvalue;
}

/* -------------------- Mark atoms for various reasons --------------------- */

void mark_io_atoms(RULE *rule, ATAB *table, int module)
{
  ATAB *scan = table;

  /* Visible atoms are input atoms by default: */

  while(scan) {
    int count = scan->count;
    SYMBOL **names = scan->names;
    int *statuses = scan->statuses;
    int i = 0;

    for(i = 1; i <= count; i++)
      if(names[i])
	statuses[i] |= MARK_INPUT;

    scan = scan->next;
  }

  /* Except those who have defining rules: */

  while(rule) {
    int type = rule->type;

    switch(type) {

    case TYPE_BASIC:
      { 
	BASIC_RULE *basic = rule->data.basic;
        clear_status(table, basic->head, MARK_INPUT);
	if(module)
	  set_module(table, basic->head, module);
      }
      break;

    case TYPE_CONSTRAINT:
      {
        CONSTRAINT_RULE *constraint = rule->data.constraint;
	clear_status(table, constraint->head, MARK_INPUT);
	if(module)
	  set_module(table, constraint->head, module);
      }
      break;

    case TYPE_CHOICE:
      {
	CHOICE_RULE *choice = rule->data.choice;
	int cnt = choice->head_cnt;
	int *head = choice->head;

	while(cnt--) {
	  clear_status(table, *head, MARK_INPUT);
	  if(module)
	    set_module(table, *head, module);
	  head++;
	}
      }
      break;

    case TYPE_INTEGRITY:
      break;

    case TYPE_WEIGHT:
      {
	WEIGHT_RULE *weight = rule->data.weight;
	clear_status(table, weight->head, MARK_INPUT);
	if(module)
	  set_module(table, weight->head, module);
      }
      break;

    case TYPE_OPTIMIZE:
      break;

    case TYPE_DISJUNCTIVE:
      {
	DISJUNCTIVE_RULE *disjunctive = rule->data.disjunctive;
	int cnt = disjunctive->head_cnt;
	int *head = disjunctive->head;

	while(cnt--) {
	  clear_status(table, *head, MARK_INPUT);
	  if(module)
	    set_module(table, *head, module);
	  head++;
	}
      }
      break;

    default:
      fprintf(stderr, "unsupported rule type %i!\n", rule->type);
      exit(-1);
    }

    rule = rule->next;
  }
}

void mark_visible(ATAB *table)
{
  ATAB *scan = table;

  while(scan) {
    int count = scan->count;
    SYMBOL **names = scan->names;
    int *statuses = scan->statuses;
    int i = 0;

    for(i = 1; i <= count; i++)
      if(names[i])
	statuses[i] |= MARK_VISIBLE;

    scan = scan->next;
  }

  return;
}

void mark_occurrences(RULE *rule, ATAB *table)
{
  ATAB *scan = table;

  /* Mark occurrences in heads, positive bodies, and negative bodies
     of rules */
 
  while(rule) {
    int type = rule->type;

    switch(type) {

    case TYPE_BASIC:
    case TYPE_CONSTRAINT:
    case TYPE_WEIGHT:
      set_status(table, get_head(rule), MARK_HEADOCC);
      /* No break here */
    case TYPE_INTEGRITY:
    case TYPE_OPTIMIZE:
    case TYPE_CLAUSE:
      set_statuses(table, get_pos_cnt(rule), get_pos(rule), MARK_POSOCC);
      set_statuses(table, get_neg_cnt(rule), get_neg(rule), MARK_NEGOCC);
      break;

    case TYPE_CHOICE:
    case TYPE_DISJUNCTIVE:
      set_statuses(table, get_head_cnt(rule), get_heads(rule), MARK_HEADOCC);
      set_statuses(table, get_pos_cnt(rule), get_pos(rule), MARK_POSOCC);
      set_statuses(table, get_neg_cnt(rule), get_neg(rule), MARK_NEGOCC);
      break;

    default:
      fprintf(stderr, "unsupported rule type %i!\n", rule->type);
      exit(-1);
    }

    rule = rule->next;
  }
}

/* ------------------------- Check program type ---------------------------- */

int non_basic(RULE *program)
{
  int basic[] = {TYPE_BASIC, TYPE_INTEGRITY, 0};
  return check_rule_types(program, basic);
}

int check_rule_types(RULE *program, int *types)
{
  RULE *rule = program;
  while(rule) {
    int found = 0;
    int type = rule->type;
    int *scan = types;

    while(*scan) {
      if(*scan == type) {
	found = -1;
	break;
      }
      scan++;
    }

    if(!found) return(-1);

    rule = rule->next;
  }
  return 0;
}

int non_atomic(RULE *program)
{
  while(program) {
    if(program->type != TYPE_BASIC)
      return -1;
    else {
      BASIC_RULE *basic = program->data.basic;
      if(basic->pos_cnt != 0)
	return -1;
    }
    program = program->next;
  }
  return 0;
}

/* --------------- Count length (internal format) ----------------- */

int len_basic(RULE *rule)
{
  BASIC_RULE *basic = rule->data.basic;

  /* 1 head #lits #negative <negative> <positive> */

  return basic->pos_cnt+basic->neg_cnt + 4;
}

int len_constraint(RULE *rule)
{
  CONSTRAINT_RULE *constraint = rule->data.constraint;

  /* 2 head #lits #negative bound <negative> <positive> */

  return constraint->pos_cnt + constraint->neg_cnt + 5;
}

int len_choice(RULE *rule)
{
  CHOICE_RULE *choice = rule->data.choice;

  /* 3 #heads <heads> #lits #negative <negative> <positive> */

  return choice->head_cnt + choice->pos_cnt + choice->neg_cnt + 4;
}

int len_integrity(RULE *rule)
{
  INTEGRITY_RULE *integrity = rule->data.integrity;

  /* 4 #lits #negative <negative> <positive> */

  return integrity->pos_cnt + integrity->neg_cnt + 3;
}

int len_weight(RULE *rule)
{
  WEIGHT_RULE *weight = rule->data.weight;

  /* 5 head bound #lits #negative <negative> <positive> <weights> */

  return 2*(weight->pos_cnt + weight->neg_cnt) + 5;
}

int len_optimize(RULE *rule)
{
  OPTIMIZE_RULE *optimize = rule->data.optimize;

  /* 6 0 #lits #negative <negative> <positive> <weights> */

  return 2*(optimize->pos_cnt + optimize->neg_cnt) + 4;
}

int len_disjunctive(RULE *rule)
{
  DISJUNCTIVE_RULE *disjunctive = rule->data.disjunctive;

  /* 7 head #lits #negative <negative> <positive> */

  return disjunctive->pos_cnt + disjunctive->neg_cnt + 4;
}

int len_clause(RULE *rule)
{
  CLAUSE *clause = rule->data.clause;
  int len = clause->pos_cnt + clause->neg_cnt;

  /* <potential weight> <positive lits> <negativelits> 0 */

  if(clause->weight>0)
    return  len + 2;
  else
    return len + 1;
}

int len_rule(RULE *rule)
{
  switch(rule->type) {
  case TYPE_BASIC:
    return len_basic(rule);
    break;

  case TYPE_CONSTRAINT:
    return len_constraint(rule);
    break;

  case TYPE_CHOICE:
    return len_choice(rule);
    break;

  case TYPE_INTEGRITY:
    return len_integrity(rule);
    break;

  case TYPE_WEIGHT:
    return len_weight(rule);
    break;

  case TYPE_OPTIMIZE:
    return len_optimize(rule);
    break;

  case TYPE_DISJUNCTIVE:
    return len_disjunctive(rule);
    break;

  case TYPE_CLAUSE:
    return len_clause(rule);
    break;

  default:
    fprintf(stderr, "%s: unknown rule type %i!\n", program_name, rule->type);
    exit(-1);
  }
}

int len(RULE *rule)
{
  int rvalue = 0;

  while(rule) {
    rvalue += len_rule(rule);

    rule = rule->next;
  }

  return rvalue;
}

int compute_statement_len(ATAB *table)
{
  int rvalue = 0;

  while(table) {
    int count = table->count;
    int *statuses = table->statuses;
    int i = 0;

    for(i=1; i<=count; i++) {
      int status = statuses[i];
      if(status & MARK_TRUE) rvalue++;
      if(status & MARK_FALSE) rvalue++;
      if(status & MARK_INPUT) rvalue++;
    }
    table = table->next;
  }
  return rvalue;
}

int number_of_rules(RULE *program)
{
  int rvalue = 0;
  while(program) {
    rvalue++;
    program = program->next;
  }
  return rvalue;
}

RULE *append_rules(RULE *program, RULE *rules)
{
  RULE *scan = program;

  if(scan) {
    while(scan->next) scan=scan->next;

    scan->next = rules;
  } else
    program = rules;

  return program;
}

RULE *copy_rule(RULE *rule)
{
  RULE *new = (RULE *)malloc(sizeof(RULE));
  int type = rule->type;

  new->type = type;
  switch(type) {
  case TYPE_BASIC:
    { BASIC_RULE *basic = (BASIC_RULE *)malloc(sizeof(BASIC_RULE));
      int pos_cnt = get_pos_cnt(rule);
      int neg_cnt = get_neg_cnt(rule);

      new->data.basic = basic;
      basic->head = get_head(rule);
      basic->neg_cnt = neg_cnt;
      basic->neg = (int *)malloc((neg_cnt+pos_cnt)*sizeof(int));
      basic->pos_cnt = pos_cnt;
      basic->pos = &((basic->neg)[neg_cnt]);
      memcpy(basic->neg, get_neg(rule), neg_cnt*sizeof(int));
      memcpy(basic->pos, get_pos(rule), pos_cnt*sizeof(int));
    }
    break;

  case TYPE_CONSTRAINT:
    { CONSTRAINT_RULE *constraint =
	(CONSTRAINT_RULE *)malloc(sizeof(CONSTRAINT_RULE));
      int pos_cnt = get_pos_cnt(rule);
      int neg_cnt = get_neg_cnt(rule);

      new->data.constraint = constraint;
      
      constraint->neg_cnt = neg_cnt;
      constraint->head = get_head(rule);
      constraint->neg = (int *)malloc((neg_cnt+pos_cnt)*sizeof(int));
      constraint->pos_cnt = pos_cnt;
      constraint->pos = &((constraint->neg)[neg_cnt]);
      memcpy(constraint->neg, get_neg(rule), neg_cnt*sizeof(int));
      memcpy(constraint->pos, get_pos(rule), pos_cnt*sizeof(int));
    }
    break;

  case TYPE_CHOICE:
    { CHOICE_RULE *choice =
	(CHOICE_RULE *)malloc(sizeof(CHOICE_RULE));
      int head_cnt = get_head_cnt(rule);
      int pos_cnt = get_pos_cnt(rule);
      int neg_cnt = get_neg_cnt(rule);

      new->data.choice = choice;
      
      choice->head_cnt = head_cnt;
      choice->head = (int *)malloc(head_cnt*sizeof(int));
      memcpy(choice->head, get_heads(rule), head_cnt*sizeof(int));

      choice->neg_cnt = neg_cnt;
      choice->neg = (int *)malloc((neg_cnt+pos_cnt)*sizeof(int));
      choice->pos_cnt = pos_cnt;
      choice->pos = &((choice->neg)[neg_cnt]);
      memcpy(choice->neg, get_neg(rule), neg_cnt*sizeof(int));
      memcpy(choice->pos, get_pos(rule), pos_cnt*sizeof(int));
    }
    break;

  case TYPE_INTEGRITY:
    { INTEGRITY_RULE *integrity =
	(INTEGRITY_RULE *)malloc(sizeof(INTEGRITY_RULE));
      int head_cnt = get_head_cnt(rule);
      int pos_cnt = get_pos_cnt(rule);
      int neg_cnt = get_neg_cnt(rule);

      new->data.integrity = integrity;
      
      integrity->neg_cnt = neg_cnt;
      integrity->neg = (int *)malloc((neg_cnt+pos_cnt)*sizeof(int));
      integrity->pos_cnt = pos_cnt;
      integrity->pos = &((integrity->neg)[neg_cnt]);
      memcpy(integrity->neg, get_neg(rule), neg_cnt*sizeof(int));
      memcpy(integrity->pos, get_pos(rule), pos_cnt*sizeof(int));
    }
    break;

  case TYPE_WEIGHT:
    { WEIGHT_RULE *weight =
	(WEIGHT_RULE *)malloc(sizeof(WEIGHT_RULE));
      int pos_cnt = get_pos_cnt(rule);
      int neg_cnt = get_neg_cnt(rule);
      int *weights = rule->data.weight->weight;

      new->data.weight = weight;
      
      weight->head = get_head(rule);
      weight->neg_cnt = neg_cnt;
      weight->neg = (int *)malloc((neg_cnt+pos_cnt)*sizeof(int));
      weight->pos_cnt = pos_cnt;
      weight->pos = &((weight->neg)[neg_cnt]);
      memcpy(weight->neg, get_neg(rule), neg_cnt*sizeof(int));
      memcpy(weight->pos, get_pos(rule), pos_cnt*sizeof(int));

      weight->weight = (int *)malloc((neg_cnt+pos_cnt)*sizeof(int));
      memcpy(weight->weight, weights, (neg_cnt+pos_cnt)*sizeof(int));

    }
    break;

  case TYPE_OPTIMIZE:
    { OPTIMIZE_RULE *optimize =
	(OPTIMIZE_RULE *)malloc(sizeof(OPTIMIZE_RULE));
      int pos_cnt = get_pos_cnt(rule);
      int neg_cnt = get_neg_cnt(rule);
      int *weight = rule->data.optimize->weight;

      new->data.optimize = optimize;
      
      optimize->neg_cnt = neg_cnt;
      optimize->neg = (int *)malloc((neg_cnt+pos_cnt)*sizeof(int));
      optimize->pos_cnt = pos_cnt;
      optimize->pos = &((optimize->neg)[neg_cnt]);
      memcpy(optimize->neg, get_neg(rule), neg_cnt*sizeof(int));
      memcpy(optimize->pos, get_pos(rule), pos_cnt*sizeof(int));

      optimize->weight = (int *)malloc((neg_cnt+pos_cnt)*sizeof(int));
      memcpy(optimize->weight, weight, (neg_cnt+pos_cnt)*sizeof(int));
    }
    break;

  case TYPE_DISJUNCTIVE:
    { DISJUNCTIVE_RULE *disjunctive =
	(DISJUNCTIVE_RULE *)malloc(sizeof(DISJUNCTIVE_RULE));
      int head_cnt = get_head_cnt(rule);
      int pos_cnt = get_pos_cnt(rule);
      int neg_cnt = get_neg_cnt(rule);

      new->data.disjunctive = disjunctive;
      
      disjunctive->head_cnt = head_cnt;
      disjunctive->head = (int *)malloc(head_cnt*sizeof(int));
      memcpy(disjunctive->head, get_heads(rule), head_cnt*sizeof(int));

      disjunctive->neg_cnt = neg_cnt;
      disjunctive->neg = (int *)malloc((neg_cnt+pos_cnt)*sizeof(int));
      disjunctive->pos_cnt = pos_cnt;
      disjunctive->pos = &((disjunctive->neg)[neg_cnt]);
      memcpy(disjunctive->neg, get_neg(rule), neg_cnt*sizeof(int));
      memcpy(disjunctive->pos, get_pos(rule), pos_cnt*sizeof(int));
    }
    break;

  case TYPE_CLAUSE:
    { CLAUSE *clause = (CLAUSE *)malloc(sizeof(CLAUSE));
      int head_cnt = get_head_cnt(rule);
      int pos_cnt = get_pos_cnt(rule);
      int neg_cnt = get_neg_cnt(rule);
      int weight = (rule->data.clause)->weight;

      new->data.clause = clause;
      
      clause->neg_cnt = neg_cnt;
      clause->neg = (int *)malloc((neg_cnt+pos_cnt)*sizeof(int));
      clause->pos_cnt = pos_cnt;
      clause->pos = &((clause->neg)[neg_cnt]);
      memcpy(clause->neg, get_neg(rule), neg_cnt*sizeof(int));
      memcpy(clause->pos, get_pos(rule), pos_cnt*sizeof(int));
      clause->weight=weight;
    }
    break;

  }
  new->next = NULL;

  return new;
}
