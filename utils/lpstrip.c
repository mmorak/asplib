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
 * STRIP -- Remove unnecessary atoms/rules from a program
 *
 * (c) 2012-2013 Tomi Janhunen
 *
 * Driver program
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "version.h"
#include "symbol.h"
#include "atom.h"
#include "rule.h"
#include "io.h"

void _version_strip_c()
{
  fprintf(stderr, "%s: version information:\n", program_name);
  _version("$RCSfile: lpstrip.c,v $",
	   "$Date: 2021/05/27 09:22:53 $",
	   "$Revision: 1.10 $");
  _version_atom_c();
  _version_rule_c();
  _version_input_c();
  _version_output_c();
}

void usage()
{
  fprintf(stderr, "\nusage:");
  fprintf(stderr, "   lpstrip <options> [<file>]\n\n");
  fprintf(stderr, "options:\n");
  fprintf(stderr, "   -h or --help -- print help message\n");
  fprintf(stderr, "   --version    -- print version information\n");
  fprintf(stderr, "\n");

  return;
}

RULE *strip_program(RULE *program, ATAB *table);
void strip_compute_statement(ATAB *table);

int main(int argc, char **argv)
{
  char *file = NULL;
  FILE *in = NULL;
  RULE *program = NULL;
  RULE *scan = NULL;
  ATAB *table = NULL;
  int i = 0;
  int number = 0;

  FILE *out = stdout;

  int option_help = 0;
  int option_version = 0;
  char *arg = NULL;
  int which = 0;
  int style = STYLE_SMODELS;

  program_name = argv[0];

  for(which=1; which<argc; which++) {
    arg = argv[which];

    if((strcmp(arg, "-h") == 0) || (strcmp(arg, "--help") == 0))
      option_help = -1;
    else if(strcmp(arg, "--version") == 0)
      option_version = 1;
    else if(file == NULL)
      file = arg;
    else {
      fprintf(stderr, "%s: unknown argument %s\n", program_name, arg);
      usage();
      exit(-1);
    }
  }

  if(option_help) usage();
  if(option_version) _version_strip_c();

  if(option_help || option_version)
    exit(0);

  if(file == NULL || strcmp("-", file) == 0) {
    in = stdin;
  } else {
    if((in = fopen(file, "r")) == NULL) {
      fprintf(stderr, "%s: cannot open file %s\n", program_name, file);
      exit(-1);
    }
  }

  program = read_program(in);
  table = read_symbols(in);
  number = read_compute_statement(in, table);

  mark_occurrences(program, table);

  program = strip_program(program, table);

  strip_compute_statement(table);

  write_program(style, out, program, table);
  fprintf(out, "0\n");

  write_symbols(STYLE_SMODELS, out, table);
  fprintf(out, "0\n");

  fprintf(out, "B+\n");
  write_compute_statement(STYLE_SMODELS, out, table, MARK_TRUE);
  fprintf(out, "0\n");

  fprintf(out, "B-\n");
  write_compute_statement(STYLE_SMODELS, out, table, MARK_FALSE);
  fprintf(out, "0\n");

  fprintf(out, "E\n");
  write_compute_statement(STYLE_SMODELS, out, table, MARK_INPUT);
  fprintf(out, "0\n");

  fprintf(out, "%i\n", number);

  exit(0);
}

/* -------------------------- Local routines ------------------------------ */

int strip_basic(RULE *rule, ATAB *table)
{
  BASIC_RULE *basic = rule->data.basic;
  int head = basic->head;
  int pos_cnt = basic->pos_cnt;
  int neg_cnt = basic->neg_cnt;
  int status = get_status(table, head);

  /* Remove hidden structure which is not needed */

  if(!visible(table, head)) {

    if(pos_cnt+neg_cnt == 0 &&
       !(status & MARK_POSOCC_OR_NEGOCC) &&
       !(status & MARK_FALSE)) {
      set_status(table, head, MARK_UNIQUE);
      return -1;
    }

    if(status & MARK_UNIQUE)
      return -1;

  } else {

    if(pos_cnt+neg_cnt == 0)
      set_status(table, head, MARK_UNIQUE);
  }

  return 0;
}

int strip_constraint(RULE *rule, ATAB *table)
{
  CONSTRAINT_RULE *constraint = rule->data.constraint;
  int head = constraint->head;
  int bound = constraint->bound;
  int status = get_status(table, head);

  /* Remove hidden structure which is not needed */

  if(!visible(table, head)) {

    if(bound == 0 &&
       !(status & MARK_POSOCC_OR_NEGOCC) &&
       !(status & MARK_FALSE)) {
      set_status(table, head, MARK_UNIQUE);
      return -1;
    }

    if(status & MARK_UNIQUE)
      return -1;
  } else {

    if(bound == 0)
      set_status(table, head, MARK_UNIQUE);
  }

  return 0;
}

int strip_choice(RULE *rule, ATAB *table)
{
  return 0;
}

int strip_weight(RULE *rule,  ATAB *table)
{
  WEIGHT_RULE *weight = rule->data.weight;
  int head = weight->head;
  int bound = weight->bound;
  int status = get_status(table, head);

  /* Remove hidden structure which is not needed */

  if(!visible(table, head)) {

    if(bound == 0 &&
       !(status & MARK_POSOCC_OR_NEGOCC) &&
       !(status & MARK_FALSE)) {
      set_status(table, head, MARK_UNIQUE);
      return -1;
    }

    if(status & MARK_UNIQUE)
      return -1;

  } else {

    if(bound == 0)
      set_status(table, head, MARK_UNIQUE);
  }

  return 0;
}

int strip_optimize(RULE *rule, ATAB *table)
{
  return 0;
}

int strip_disjunctive(RULE *rule, ATAB *table)
{
  return 0;
}

int strip_rule(RULE *rule, ATAB *table)
{
  int answer = 0;

  switch(rule->type) {
  case TYPE_BASIC:
    answer = strip_basic(rule, table);
    break;

  case TYPE_CONSTRAINT:
    answer = strip_constraint(rule, table);
    break;

  case TYPE_CHOICE:
    answer = strip_choice(rule, table);
    break;

  case TYPE_WEIGHT:
    answer = strip_weight(rule, table);
    break;

  case TYPE_OPTIMIZE:
    answer = strip_optimize(rule, table);
    break;

  case TYPE_DISJUNCTIVE:
    answer = strip_disjunctive(rule, table);
    break;

  default:
    error("unknown rule type");
  }

  return answer;
}

RULE *strip_program(RULE *program, ATAB *table)
{
  RULE *scan = program;
  RULE *previous = NULL;

  while(scan) {
    RULE *next = scan->next;
    int drop = strip_rule(scan, table);

    if(drop) {
      if(previous)
	previous->next = next;

      if(program == scan)
	program = next;

      free_rule(scan);
    } else
      previous = scan;

    scan = next;
  }

  return program;
}

void strip_compute_statement(ATAB *table)
{
  while(table) {
    int count = table->count;
    int *statuses = table->statuses;
    SYMBOL **names = table->names;
    int i = 0;

    for(i=1; i<=count; i++) {  
      int offset = table->offset;
      int atom = i+offset;
      SYMBOL *name = names[i];
      int status = statuses[i];

      if(status & MARK_UNIQUE) {

	if(status & MARK_TRUE) statuses[i] &= ~MARK_TRUE;

	if(!name && (status & MARK_FALSE))
	  statuses[i] |= MARK_TRUE; /* Retain contradiction */
      }

    }
    table = table->next;
  }
  return;
}
