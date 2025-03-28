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
 * LPLIST -- Give a human-readable listing of an smodels program
 *
 * (c) 2005-2016 Tomi Janhunen
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

void _version_lplist_c()
{
  fprintf(stderr, "%s: version information:\n", program_name);
  _version("$RCSfile: lplist.c,v $",
	   "$Date: 2021/05/27 08:51:45 $",
	   "$Revision: 1.10 $");
  _version_atom_c();
  _version_rule_c();
  _version_input_c();
  _version_output_c();
}

void usage()
{
  fprintf(stderr, "\nusage:");
  fprintf(stderr, "   lplist <options> <file>\n\n");
  fprintf(stderr, "options:\n");
  fprintf(stderr, "   -h or --help -- print help message\n");
  fprintf(stderr, "   --version    -- print version information\n");
  fprintf(stderr, "   -s           -- include symbols\n");
  fprintf(stderr, "   -d           -- input is a cnf in DIMACS format\n");
  fprintf(stderr, "   --gnt        -- generate output for gnt\n");
  fprintf(stderr, "   --dlv        -- generate output for dlv\n");
  fprintf(stderr, "\n");

  return;
}

int main(int argc, char **argv)
{
  char *file = NULL;
  FILE *in = NULL;
  RULE *program = NULL;
  RULE *scan = NULL;
  ATAB *table = NULL;
  int i = 0;
  int rcnt = 0;
  int lcnt = 0;

  FILE *out = stdout;

  int option_help = 0;
  int option_version = 0;
  int option_symbols = 0;
  int option_dimacs = 0;
  int option_gnt = 0;
  int option_dlv = 0;
  char *arg = NULL;
  int which = 0;
  int style = STYLE_READABLE;
  int weighted = 0;

  program_name = argv[0];

  for(which=1; which<argc; which++) {
    arg = argv[which];

    if((strcmp(arg, "-h") == 0) || (strcmp(arg, "--help") == 0))
      option_help = -1;
    else if(strcmp(arg, "--version") == 0)
      option_version = 1;
    else if(strcmp(arg, "-s") == 0)
      option_symbols = 1;
    else if(strcmp(arg, "-d") == 0)
      option_dimacs = 1;
    else if(strcmp(arg, "--gnt") == 0)
      option_gnt = 1;
    else if(strcmp(arg, "--dlv") == 0)
      option_dlv = 1;
    else if(file == NULL)
      file = arg;
    else {
      fprintf(stderr, "%s: unknown argument %s\n", program_name, arg);
      usage();
      exit(-1);
    }
  }

  if(option_help) usage();
  if(option_version) _version_lplist_c();

  if(option_gnt && option_dlv) {
    fprintf(stderr, "%s: options --gnt and --dlv are mutually exclusive!\n",
	    program_name);
    exit(-1);
  }
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

  if(option_gnt)
    style = STYLE_GNT;
  else if(option_dlv)
    style = STYLE_DLV;

  if(option_dimacs) {
    RULE *scan = NULL;
    int clauses = 0;

    program = read_cnf(in, &table, &weighted);
    scan = program;
    while(scan) {
      clauses++;
      scan = scan->next;
    }

    fputs("% ", out);
    if(weighted)
      fputs("weighted ", out);
    fprintf(out, "CNF with %i vars %i clauses", table_size(table), clauses);
    if(max_weight)
      fprintf(out, " (max weight = %ld)", max_weight);
    fputs("\n", out);

    write_symbols(style, out, table);
    write_cnf(style, out, program, table);
  } else {
    program = read_program(in);
    table = read_symbols(in);
    read_compute_statement(in, table);

    write_program(style, out, program, table);
    if(style != STYLE_DLV)
      printf("\n#compute {");
    write_compute_statement(style, out, table, MARK_TRUE|MARK_FALSE);
    if(style != STYLE_DLV)
      printf("}.\n");

    if((style == STYLE_GNT) || (style == STYLE_DLV))
      write_compute_statement(style, out, table, MARK_INPUT);
    else
      write_input(style, out, table);

    if(option_symbols) {
      printf("\n");
      printf("%% Symbol table:\n");
      write_symbols(style, out, table);
    }
  }

  exit(0);
}
