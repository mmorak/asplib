/* utils -- Basic utilities related to the Smodels file format

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
 * LEN -- Compute the length of a given program
 *
 * (c) 2002-2017 Tomi Janhunen
 *
 * Main program
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

void _version_len_c()
{
  fprintf(stderr, "%s: version information:\n", program_name);
  _version("$RCSfile: len.c,v $",
	   "$Date: 2021/05/27 08:46:34 $",
	   "$Revision: 1.17 $");
  _version_atom_c();
  _version_rule_c();
  _version_input_c();
  _version_output_c();
}

void usage()
{
  fprintf(stderr, "\nusage:");
  fprintf(stderr, "   len <options> <file>\n\n");
  fprintf(stderr, "options:\n");
  fprintf(stderr, "   -h or --help -- print help message\n");
  fprintf(stderr, "   --version -- print version information\n");
  fprintf(stderr, "   -a -- count atoms\n");
  fprintf(stderr, "   -r -- count rules\n");
  fprintf(stderr, "   -rn - count normal rules\n");
  fprintf(stderr, "   -rg - count choice rules\n");
  fprintf(stderr, "   -rc - count cardinality rules\n");
  fprintf(stderr, "   -rw - count weight rules\n");
  fprintf(stderr, "   -rd - count disjunctive rules\n");
  fprintf(stderr, "   -ro - count optimization statements\n");
  fprintf(stderr, "   -l -- count length (integers in internal format)\n");
  fprintf(stderr, "   -d -- input is a cnf/wcnf in DIMACS format\n");
  fprintf(stderr, "\n");

  return;
}



int main(int argc, char **argv)
{
  char *file = NULL;
  FILE *in = NULL;
  RULE *rule = NULL, *cnf = NULL, *clause = NULL;
  ATAB *table = NULL;
  int i = 0;
  int rcnt = 0;
  int
    rcnt_norm = 0, rcnt_gen = 0,
    rcnt_card = 0, rcnt_wght = 0, 
    rcnt_disj = 0, rcnt_opt = 0;
  int lcnt = 0;

  FILE *out = stdout;

  int option_help = 0;
  int option_version = 0;
  int option_all = -1;
  int option_atoms = 0;
  int option_rules = 0;
  int option_rules_by_type = 0;
  int option_literals = 0;
  int option_dimacs = 0;
  char *arg = NULL;
  int which = 0;

  program_name = argv[0];

  for(which=1; which<argc; which++) {
    arg = argv[which];

    if((strcmp(arg, "-h") == 0) || (strcmp(arg, "--help") == 0))
      option_help = -1;
    else if(strcmp(arg, "--version") == 0)
      option_version = 1;
    else if(strcmp(arg, "-d") == 0)
      option_dimacs = 1;
    else if(strcmp(arg, "-a") == 0) {
      option_all = 0;
      option_atoms = -1;
    } else if(strcmp(arg, "-r") == 0) {
      option_all = 0;
      option_rules = -1;
    } else if(strcmp(arg, "-rn") == 0) {
      option_all = 0;
      option_rules_by_type = TYPE_BASIC;
    } else if(strcmp(arg, "-rg") == 0) {
      option_all = 0;
      option_rules_by_type = TYPE_CHOICE;
    } else if(strcmp(arg, "-rc") == 0) {
      option_all = 0;
      option_rules_by_type = TYPE_CONSTRAINT;
    } else if(strcmp(arg, "-rw") == 0) {
      option_all = 0;
      option_rules_by_type = TYPE_WEIGHT;
    } else if(strcmp(arg, "-rd") == 0) {
      option_all = 0;
      option_rules_by_type = TYPE_DISJUNCTIVE;
    } else if(strcmp(arg, "-ro") == 0) {
      option_all = 0;
      option_rules_by_type = TYPE_OPTIMIZE;
    } else if(strcmp(arg, "-l") == 0) {
      option_all = 0;
      option_literals = -1;
    } else if(file == NULL)
      file = arg;
    else {
      fprintf(stderr, "%s: unknown argument %s\n", program_name, arg);
      usage();
      exit(-1);
    }
  }

  if(option_help) usage();
  if(option_version) _version_len_c();

  if(option_help || option_version)
    exit(0);

  if(file == NULL || strcmp("-", file) == 0) {
    in = stdin;
  } else {
    in = fopen(file, "r");
    if(in == NULL) {
      fprintf(stderr, "%s: cannot open file %s\n", program_name, file);
      exit(-1);
    }
  }
  
  if(option_dimacs) {
    int clauses = 0;
    int weighted = 0;

    table = initialize_cnf(in, &clauses, &weighted);
    rcnt = clauses;

    /* Read clauses in one by one and make calculations */
    while(clauses) {
      clause = read_clause(in, weighted);
      lcnt += len_clause(clause);
      free_rule(clause);
      clauses--;
    }

  } else {
    initialize_program();
    rule = read_rule(in);

    /* Read rules in one by one and make calculations */
    while(rule) {
      if(option_all)
	switch(rule->type) {
	case TYPE_BASIC:
	  rcnt_norm++;
	  break;
	case TYPE_CHOICE:
	  rcnt_gen++;
	  break;
	case TYPE_CONSTRAINT:
	  rcnt_card++;
	  break;
	case TYPE_WEIGHT:
	  rcnt_wght++;
	  break;
	case TYPE_DISJUNCTIVE:
	  rcnt_disj++;
	  break;
	case TYPE_OPTIMIZE:
	  rcnt_opt++;
	  break;
	default:
	  break;
	}
      if(option_rules_by_type) {
	if(rule->type == option_rules_by_type)
	  rcnt++;
      } else
	rcnt++;
      lcnt += len_rule(rule);
      free_rule(rule);
      rule = read_rule(in);
    }
    table = read_symbols(in);
    read_compute_statement(in, table);
  }

  if(option_all || option_literals) {
    if(!option_dimacs)
      lcnt += compute_statement_len(table);
  }

  if(option_all) {
    if(option_dimacs)
      fprintf(stdout, "CNF %s in numbers:\n", file);
    else
      fprintf(stdout, "Program %s in numbers:\n", file);
  }

  if(option_all)
    fprintf(stdout, " - %i atom(s)\n", table_size(table));
  if(option_atoms) {
    fprintf(stdout, "%i", table_size(table));
  }
  if(option_atoms && (option_rules || option_rules_by_type))
    fputs(" ", stdout);

  if(option_all) {
    fprintf(stdout, " - %i rule(s) \n", rcnt);
    if(rcnt_norm)
      fprintf(stdout, "   %i normal\n", rcnt_norm);
    if(rcnt_gen)
      fprintf(stdout, "   %i choice\n", rcnt_gen);
    if(rcnt_card)
      fprintf(stdout, "   %i cardinality\n", rcnt_card);
    if(rcnt_wght)
      fprintf(stdout, "   %i weight\n", rcnt_wght);
    if(rcnt_disj)
      fprintf(stdout, "   %i disjunctive\n", rcnt_disj);
    if(rcnt_opt)
      fprintf(stdout, "   %i optimization\n", rcnt_opt);
  }
  if(option_rules || option_rules_by_type)
    fprintf(stdout, "%i", rcnt);
  if(option_rules && option_literals)
    fputs(" ", stdout);

  if(option_all)
    fprintf(stdout, " - %i integer(s) in internal format\n", lcnt);
  if(option_literals)
    fprintf(stdout, "%i", lcnt);

  if(!option_all)
    fputs("\n", stdout);

  exit(0);
}
