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
 * Definitions related with input and output routines
 *
 * (c) 2005-2021 Tomi Janhunen
 */

#define _IO_H_RCSFILE  "$RCSfile: io.h,v $"
#define _IO_H_DATE     "$Date: 2021/05/27 10:55:02 $"
#define _IO_H_REVISION "$Revision: 1.14 $"

extern void _version_input_c();
extern void _version_output_c();

/* Declarations related with input.c */

extern void _version_input_c();

extern char *program_name;
extern void error(char *msg);
extern char *read_string(FILE *in);

extern void initialize_program();
extern RULE *read_rule(FILE *in);
extern RULE *read_program(FILE *in);
extern ATAB *read_symbols(FILE *in);
extern int read_compute_statement(FILE *in, ATAB *table);

extern ATAB *initialize_cnf(FILE *in, int *clauses, int *weighted);
extern RULE *read_clause(FILE *in, int weighted);
extern RULE *read_cnf(FILE *in, ATAB **table, int *weighted);

/* Declarations related with output.c */

#define STYLE_READABLE 1  /* Symbolic smodels/dimacs format */
#define STYLE_SMODELS  2  /* Internal smodels format */
#define STYLE_DIMACS   3  /* Internal dimacs format */
#define STYLE_GNT      4  /* Symbolic gnt format */
#define STYLE_DLV      5  /* Symbolic dlv format */

extern void _version_output_c();

extern void write_name(FILE *out, SYMBOL *s, char *prefix, char *postfix);

extern void write_atom(int style, FILE *out, int atom, ATAB *table);
extern void write_atom_if_possible(int style, FILE *out, int atom, ATAB *table);
extern void write_atom_list(int style, FILE *out,
			    int cnt, int *atoms, ATAB *table);
extern void write_other_atom(int style, FILE *out, int atom, ATAB *table);
extern void write_clause_as_rule(int style, FILE *out,
                                 RULE *clause, ATAB *table1,
				 int contradiction, ATAB *table2);
extern void write_rule(int style, FILE *out, RULE *rule, ATAB *table);
extern void write_program(int style, FILE *out, RULE *program, ATAB *table);
extern void write_status(FILE *out, int flags);

extern void write_symbols(int style, FILE *out, ATAB *table);
extern void write_compute_statement(int style, FILE *out, ATAB *table,
				    int mask);
extern void write_input(int style, FILE *out, ATAB *table);

extern void write_classical_atom(int style, FILE *out, int atom,
				 ATAB *table);
extern void write_other_classical_atom(int style, FILE *out, int atom,
				       ATAB *table);
extern void write_cnf(int style, FILE *out, RULE *cnf, ATAB *table);

extern void free_rule(RULE *rule);
extern void free_program(RULE *program);

extern long max_weight;
