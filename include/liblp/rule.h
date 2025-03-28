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
 * Data structures related to rules used in the smodels system
 *
 * (c) 2005-2021 Tomi Janhunen
 */

#define _RULE_H_RCSFILE  "$RCSfile: rule.h,v $"
#define _RULE_H_DATE     "$Date: 2021/05/27 08:46:04 $"
#define _RULE_H_REVISION "$Revision: 1.12 $"

extern void _version_rule_c();

/* Rule types supported by smodels */

#define TYPE_BASIC       1
#define TYPE_CONSTRAINT  2
#define TYPE_CHOICE      3
#define TYPE_INTEGRITY   4
#define TYPE_WEIGHT      5
#define TYPE_OPTIMIZE    6

/* Other expressions of interest */

#define TYPE_ORDERED     7   /* lparse / not supported by smodels */
#define TYPE_DISJUNCTIVE 8   /* lparse / cmodels extension */
#define TYPE_CLAUSE      9   /* Own extension */

/* Various kinds of rules */

typedef struct basic_rule {
  int head;
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} BASIC_RULE;

typedef struct constraint_rule {
  int head;
  int bound;
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} CONSTRAINT_RULE;

typedef struct choice_rule {
  int head_cnt;
  int *head;
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} CHOICE_RULE;

typedef struct integrity_rule {
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} INTEGRITY_RULE;

typedef struct weight_rule {
  int head;
  int bound;
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
  int *weight;
} WEIGHT_RULE;

typedef struct optimize_rule {
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
  int *weight;
} OPTIMIZE_RULE;

typedef struct disjunctive_rule {
  int head_cnt;
  int *head;
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} DISJUNCTIVE_RULE;

typedef struct clause {
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
  long weight; /* For MaxSAT */
} CLAUSE;

typedef union any_rule {
  struct basic_rule *basic;
  struct constraint_rule *constraint;
  struct choice_rule *choice;
  struct integrity_rule *integrity;
  struct weight_rule *weight;
  struct optimize_rule *optimize;
  struct disjunctive_rule *disjunctive;
  struct clause *clause;
} ANY_RULE;

typedef struct rule {
  int type;
  ANY_RULE data;
  struct rule *next;
} RULE;

extern void _version_rule_c();

extern int get_head(RULE *r);
extern int *get_heads(RULE *r);
extern int get_head_cnt(RULE *r);
extern int *get_pos(RULE *r);
extern int get_pos_cnt(RULE *r);
extern int *get_neg(RULE *r);
extern int get_neg_cnt(RULE *r);

extern int check_negative_invisible(RULE *program, ATAB* table);
extern void mark_io_atoms(RULE *program, ATAB *table, int module);
extern void mark_visible(ATAB *table);
extern void mark_occurrences(RULE *program, ATAB *table);
extern int non_basic(RULE *program);
extern int check_rule_types(RULE *program, int *types);
extern int non_atomic(RULE *program);
extern int len_clause(RULE *clause);
extern int len_rule(RULE *clause);
extern int len(RULE *program);
extern int compute_statement_len(ATAB *table);
extern int number_of_rules(RULE *program);
extern RULE *append_rules(RULE *program, RULE *rules);
extern RULE *copy_rule(RULE *rule);
