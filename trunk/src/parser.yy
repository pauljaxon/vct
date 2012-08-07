/*  -*- C++ -*- */

//==========================================================================
//==========================================================================
// PARSER.YY
//==========================================================================
//==========================================================================

/*
This file is part of Victor: a SPARK VC Translator and Prover Driver.

Copyright (C) 2009, 2010 University of Edinburgh

Author(s): Paul Jackson

Victor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

Victor is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

A copy of the GNU General Public License V3 can be found in file
LICENSE.txt and online at http://www.gnu.org/licenses/.
*/

// Bison parser for SPARK FDL.

// This version works with bison 2.4.  To produce a version working with
// bison 2.3, comment out lines with comments "**bison 2.4**" and uncomment
// lines with comments "**bison 2.3**".


%skeleton "lalr1.cc"
%defines             /* Enable writing of "parser.hh" file with token defs */

%require "2.4.1"  // **bison 2.4**



// %code requires: Code region for bison 2.4 for type definitions used
// in code generated from %union, %lex-param and %parse-param
// directives.

// %{            // **bison 2.3**
%code requires { // **bison 2.4**

/* First part of user declarations.  Inserted near parser.tab.hh start */

#include <string>    /* C++ strings */

#include "node.hh"

#include "pdriver.hh"

using namespace std;

}      // **bison 2.4**
// %}  // **bison 2.3**


%parse-param { pdriver& driver }
%lex-param   { pdriver& driver }

%debug           /* Enable compilation of trace facilities */
%error-verbose

%union
{
    int ival;
    string* sval;     /* C++ strings */
    Node* nval ;
    z::Kind kval;
}

// %{   // **bison 2.3**
%code { // **bison 2.4**

/* First user implementation prologue.  Inserted at start of parser.tab.cc */

#include "lexer.hh"

}      // **bison 2.4**
// %}  // **bison 2.3**

%token <sval> TITLE
%token FOR RULE_FAMILY
%token <sval> GOAL_ORIGINS
%token COLON LSB RSB LPAREN RPAREN COMMA AMPERSAND SEMIC DOT DOTDOT
%token REQUIRES
%token MAY_BE_REPLACED_BY MAY_BE_DEDUCED MAY_BE_DEDUCED_FROM
%token ARE_INTERCHANGEABLE
%token IF END FUNCTION PROCEDURE TYPE VAR CONST
%token FOR_SOME FOR_ALL
%token ARRAY RECORD ASSIGN OF
%token <sval> SUBPROG_ID CONCL_ID HYP_ID TASK_TYPE PACKAGE PACKAGE_SPEC PACKAGE_BODY ID NATNUM
%token TRIPLESTAR
%token TRIPLEBANG
%token START_FDL_FILE START_RULE_FILE START_VCG_FILE;
%token FILE_END 0

%nonassoc ASSIGN
%nonassoc AMPERSAND
%nonassoc DOTDOT

 /*

Operator precedences follow those listed in GenVCs manual.
  "Operator precedence" section (4.4.1.4 in Issue 8.12).

On precedence of **:

  Gen VCs manual, (section 4.4.1 "Operators") says:

  the SPARK exponentiation operator ** is supported by the SPADE
  Automatic Simplifier and SPADE Proof Checker as though it were an
  extension of FDL's expression syntax; it has a precedence which is
  compatible with that used in Ada, relative to the other FDL operator
  precedences.

Ada 95 Standard gives ** strongest precedence, so we do that here.


On precedence of unary minus.

  GenVCs manual places it stronger than * / div mod, but doesn't say
  where it fits in relative to **.  Following the above suggestion on
  ** being compatible with Ada where ** is *stronger* than unary
  minus, we also make ** stronger than unary minus.

  The precedence ordering we implement here agrees with that listed in
  the SPARK book, p294.

  NB: The SPARK Grammar in GenVCs manual, start of
  Section 4.4 "Expressions" places unary minus `on same level'
  with binary +/-.  I assume this is a mistake.

 */
%nonassoc IFF IMPLIES
%left OR
%left AND
%nonassoc NOT
%nonassoc EQ NE GT GE LT LE
%left PLUS MINUS
%left STAR SLASH DIV MOD
%nonassoc UMINUS
%left STARSTAR

%type <nval>   top file

%type <nval>   fdl_file   fdl_decl
%type <nval>  fdl_decls  types

%type <nval>   rule_file rule_family typeassum rule rule_condition rule_body
%type <nval>  rule_families typeassums rules

%type <nval>   vcg_file hyp concl goal
%type <nval>  goalsets goalset goals hyps concls

%type <nval>   id typedecl expseq
%type <nval>  ids typedecls multidecl expseq1
%type <sval>   id_str

%type <nval>   type type_id
%type <nval>  recordtypedecls

%type <nval>   exp exp_id aexp
%type <nval>  indexset aexpseq



%start top

%%
/*
==========================================================================
RULES
==========================================================================
*/
/*
==========================================================================
Top level
==========================================================================
*/

// top: file FILE_END {driver.result = $1;}
top: file {driver.result = $1;}
;

file:
   START_FDL_FILE fdl_file { $$ = $2; }
 | START_RULE_FILE rule_file { $$ = $2; }
 | START_VCG_FILE vcg_file { $$ = $2; }
;

/*
==========================================================================
FDL files
==========================================================================
*/

fdl_file: TITLE program_kind id SEMIC fdl_decls END SEMIC
          {
	      // Drop recording of $2 and $3
              $$ = $5;
              $$->kind = z::FDL_FILE;
          }
;

program_kind:
 TASK_TYPE
 | FUNCTION
 | PROCEDURE
 | PACKAGE
 ;

fdl_decls:
   /* empty */           { $$ = new Node(z::SEQ); }
 | fdl_decls fdl_decl    { $$ = $1;  $$->addChild($2);}
;

fdl_decl:
   TYPE id_str EQ type SEMIC
        { if ($4->kind == z::PENDING)
              $$ = new Node(z::DEF_TYPE, * $2);
          else
              $$ = new Node(z::DEF_TYPE, * $2, $4);
          delete $2;
        }
 | CONST id_str COLON type EQ exp SEMIC
        { if ($6->kind == z::PENDING)
              $$ = new Node(z::DEF_CONST, * $2, $4);
          else
              $$ = new Node(z::DEF_CONST, * $2, $4, $6);
          delete $2;
        }
 | VAR   id_str COLON type SEMIC                 // FIXME: allow multiple IDs
        { $$ = new Node(z::DECL_VAR, * $2, $4);
          delete $2;
        }

 | FUNCTION id_str LPAREN types RPAREN COLON type SEMIC
        { $$ = new Node(z::DECL_FUN,
                        * $2,
                        $4,
                        $7);
          delete $2;
        }
 | FUNCTION id_str COLON type SEMIC
        { $$ = new Node(z::DEF_CONST, * $2, $4);
          delete $2;
        }
;

types:
   type             { $$ = new Node(z::SEQ,$1); }
 | types COMMA type { $$ = $1;
                      $$->addChild($3);
                    }
;


/*
==========================================================================
Rule files (RLS, RUL, RLU suffix)
==========================================================================
Grammar allows for initial sequence of rules without an explicit
RULE_FAMILY header.  This is observed in RUL and RLU files.
*/

rule_file:
    rules rule_families
          { $$ = $2;
            $$->addLeftChild(new Node(z::RULE_FAMILY,
                                      "implicit",
                                      new Node(z::SEQ),
                                      $1));
            $$->kind = z::RULE_FILE;
          }
;

rule_families:
   /* empty */               { $$ = new Node(z::SEQ); }
 | rule_families rule_family { $$ = $1; $$->addChild($2); }
;

rule_family:
   RULE_FAMILY id_str COLON typeassums DOT rules
                            { $$ = new Node(z::RULE_FAMILY, *$2, $4, $6); }
 | RULE_FAMILY id_str COLON DOT rules
                            { $$ = new Node(z::RULE_FAMILY,
                                            *$2,
                                            new Node(z::SEQ),
                                            $5);
                            }
;

typeassums:
   typeassum                         { $$ = new Node(z::SEQ,$1); }
 | typeassums AMPERSAND typeassum    { $$ = $1;  $$->addChild($3); }
;

typeassum:
  exp REQUIRES LSB typedecls RSB   { $$ = new Node(z::SEQ); } // Placeholder
;

rules:
   /* empty */  { $$ = new Node(z::SEQ);  }
 | rules rule   { $$ = $1;   $$->addChild($2); }
;

rule:
   id_str LPAREN NATNUM RPAREN COLON rule_body DOT
                      { $$ = new Node(z::RULE, (*$1) + "(" + (*$3) + ")", $6);
                        delete $1;
                        delete $3;}

rule_body:
   exp MAY_BE_REPLACED_BY exp rule_condition
                      { $$ = new Node(z::MAY_BE_REPLACED_BY, $1, $3);
                        if ($4->kind != z::TRUE) $$->addChild($4);
                      }
 | exp MAY_BE_DEDUCED
                      { $$ = new Node(z::MAY_BE_DEDUCED, $1); }
 | exp MAY_BE_DEDUCED_FROM expseq
                      { $$ = new Node(z::MAY_BE_DEDUCED, $1, $3); }
 | exp AMPERSAND exp ARE_INTERCHANGEABLE rule_condition
                      { $$ = new Node(z::ARE_INTERCHANGEABLE, $1, $3);
                        if ($5->kind != z::TRUE) $$->addChild($5);
                      }
;

rule_condition:
   /* empty */        { $$ = new Node(z::TRUE); }
 | IF expseq          { $$ = $2; }
;


/*
==========================================================================
VCG files
==========================================================================
*/

vcg_file: goalsets    { $$ = $1; $$->kind = z::VCG_FILE; }
;

/* Run all goalsets together into one vector of goals */

goalsets:
   goalset            { $$ = $1; }

 | goalsets goalset   { $$ = $1;
                        $$->appendChildren($2);
                      }
;

goalset: FOR GOAL_ORIGINS COLON goals
   {
       $$ = $4;
       for (int i = 0; i != $$->arity(); i++) {
           Node* c = $$->child(i);
           (c->id).append(" " + (* $2));
       }
       delete $2;
   }
;

goals:
   goal         {$$ = new Node(z::SEQ,$1); }
 | goals goal   {$$ = $1; $$->addChild($2); }
;

goal:
   SUBPROG_ID DOT hyps IMPLIES concls
            {
             $3->kind = z::HYPS;
             $5->kind = z::CONCLS;
             $$ = new Node(z::GOAL, * $1, $3, $5);
             delete $1;
            }
 | SUBPROG_ID DOT TRIPLESTAR id_str DOT
            {
             $$ = new Node (z::GOAL, * $1);
             delete $1;
            }
;

hyps:
   /* empty */          {$$ = new Node(z::SEQ); }
 | hyps hyp             {$$ = $1; $$->addChild($2); }
;

hyp: HYP_ID COLON exp DOT  {$$ = $3; delete $1;}
;

concls:
   /* empty */          {$$ = new Node(z::SEQ); }
 | concls concl         {$$ = $1; $$->addChild($2); }
;

concl: CONCL_ID COLON exp DOT  {$$ = $3; delete $1;}
;


/*
==========================================================================
Multi-use non-terminals
==========================================================================
IDs, type declarations and sequence expressions.
*/

// Note that the tokens "task_type", "title", "package_spec" and
// "package_body" can also be a valid identifiers.
id_str:
   ID           { $$ = $1;}
 | HYP_ID       { $$ = $1;}
 | CONCL_ID     { $$ = $1;}
 | SUBPROG_ID   { $$ = $1;}
 | TASK_TYPE    { $$ = $1;}
 | PACKAGE      { $$ = $1;}
 | PACKAGE_SPEC { $$ = $1;}
 | PACKAGE_BODY { $$ = $1;}
 | TITLE        { $$ = $1;}
;

id:
   id_str { $$ = new Node(z::ID, * $1); delete $1;}

ids:
   id                { $$ = new Node(z::SEQ,$1); }
 | ids COMMA id       { $$ = $1;  $$->addChild($3); }
;

typedecls:
typedecl                     { $$ = new Node(z::SEQ,$1); }
 | typedecls COMMA typedecl    { $$ = $1; $$->addChild($3); }
;

typedecl:
   id_str COLON type        { $$ = new Node(z::DECL, * $1, $3);
                          delete $1;
                        }
;

multidecl:
   ids COLON type
   { $$ = new Node(z::SEQ);
     for (int i = 0; i != $1->arity(); i++) {
         $$->addChild(new Node(z::DECL, $1->child(i)->id, $3) );
     }
   }
;

expseq:
   LSB expseq1 RSB    { $$ = $2; }
;

expseq1:
   exp                { $$ = new Node(z::SEQ, $1); }
 | expseq1 COMMA exp  { $$ = $1;           $$->addChild($3); }
;


/*
==========================================================================
Types
==========================================================================
*/

type:
   type_id                     { $$ = $1; }
 | LPAREN ids RPAREN           { $$ = $2; $$->kind = z::ENUM_TY; }
 | ARRAY LSB types RSB OF type { $$ = new Node(z::ARRAY_TY,
                                               $3,
                                               $6);
                               }
 | RECORD recordtypedecls END  { $$ = $2; $$->kind = z::RECORD_TY; }
;

recordtypedecls:
   multidecl                         { $$ = $1; }
 | recordtypedecls SEMIC multidecl
          { $$ = $1;
            $$->appendChildren($3);
          }
;

type_id:
   id    { const string& idstr = $1->id;
           if      (idstr == "pending")  $$ = new Node(z::PENDING);
           else if (idstr == "integer")  $$ = Node::int_ty;
           else if (idstr == "real")     $$ = Node::real_ty;
           else if (idstr == "boolean")  $$ = Node::bool_ty;
           else  {
               $1->kind = z::TYPE_ID;
               $$ = $1;
           }
         }
;

/*
==========================================================================
Value expressions
==========================================================================
*/

exp:
   FOR_ALL LPAREN multidecl COMMA exp RPAREN
         { $$ = new Node(z::FORALL, $3, $5); }
 | FOR_SOME LPAREN multidecl COMMA exp RPAREN
         { $$ = new Node(z::EXISTS, $3, $5); }
 | exp IMPLIES exp {$$ = new Node(z::IMPLIES, $1, $3); }
 | exp IFF exp     {$$ = new Node(z::IFF, $1, $3); }
 | exp AND exp     {$$ = new Node(z::AND, $1, $3); }
 | exp OR exp      {$$ = new Node(z::OR, $1, $3); }
 | NOT exp         {$$ = new Node(z::NOT, $2); }
 | exp EQ exp      {$$ = new Node(z::EQ, $1, $3); }
 | exp NE exp      {$$ = new Node(z::NOT, new Node(z::EQ, $1, $3)); }
 | exp LT exp      {$$ = new Node(z::LT, $1, $3); }
 | exp GT exp      {$$ = new Node(z::GT, $1, $3); }
 | exp LE exp      {$$ = new Node(z::LE, $1, $3); }
 | exp GE exp      {$$ = new Node(z::GE, $1, $3); }
 | PLUS exp  %prec UMINUS
                   {$$ = $2; }
 | MINUS exp %prec UMINUS
                   {$$ = new Node(z::UMINUS, $2); }
 | exp PLUS exp    {$$ = new Node(z::PLUS, $1, $3); }
 | exp MINUS exp   {$$ = new Node(z::MINUS, $1, $3); }
 | exp STAR exp    {$$ = new Node(z::TIMES, $1, $3); }
 | exp SLASH exp   {$$ = new Node(z::RDIV, $1, $3); }
 | exp MOD exp     {$$ = new Node(z::MOD, $1, $3); }
 | exp DIV exp     {$$ = new Node(z::IDIV, $1, $3); }
 | exp STARSTAR exp{$$ = new Node(z::EXP, $1, $3); }
 | exp DOTDOT exp  {$$ = new Node(z::SUBRANGE, $1, $3); }
 | LPAREN indexset RPAREN  {$$ = $2; } // exp should be adequate here,
                                       // but have seen output of
                                       // Examiner that parenthesise
                                       // indexsets
 | NATNUM          {$$ = new Node(z::NATNUM, * $1); delete $1; }
 | expseq          {$$ = $1;}
 | exp_id          {$$ = $1;}
 | id_str LPAREN aexpseq RPAREN
      { string& idstr = * $1;
        int nargs = $3->arity(); // nargs > 0 by def of aexpseq
        if      (idstr == "element" && nargs == 2) {
            $$ = $3;
            $$->kind = z::ARR_ELEMENT;
        }
        else if (idstr == "update" && nargs == 3) {
             $$ = $3;
             $$->kind = z::ARR_UPDATE;
        }
        else if (idstr.size() >= 4
                 && string(idstr, 0, 4) == "mk__") {

            // array or record constructor
            if ($3->child(0)->kind == z::ASSIGN
                && $3->child(0)->arity() == 1) {
                $$ = $3;
                $$->kind = z::MK_RECORD;
                $$->id = idstr.erase(0,4);
            }
            else {
                $$ = $3;
                $$->kind = z::MK_ARRAY;
                $$->id = idstr.erase(0,4);
            }
        }
        else if (idstr.size() >= 4
                 && string(idstr, 0, 4) == "fld_"
                 && nargs == 1)
        {
            $$ = $3;
            $$->kind = z::RCD_ELEMENT;
            $$->id = idstr.erase(0,4);
        }
        else if (idstr.size() >= 4
                 && string(idstr, 0, 4) == "upf_"
                 && nargs == 2) {
            $$ = $3;
            $$->kind = z::RCD_UPDATE;
            $$->id = idstr.erase(0,4);
        }
        else if (idstr == "succ") {
            $$ = $3;
            $$->kind = z::SUCC;
        }
        else if (idstr == "pred") {
            $$ = $3;
            $$->kind = z::PRED;
        }
        else if (idstr == "odd") {
            $$ = $3;
            $$->kind = z::ODD;
        }
        else if (idstr == "abs") {
            $$ = $3;
            $$->kind = z::ABS;
        }
        else if (idstr == "sqr") {
            $$ = $3;
            $$->kind = z::SQR;
        }
        else {
            $$ = $3;
            $$->kind = z::FUN_AP;
            $$->id = idstr;
        }

        delete $1;
      }
;

aexpseq:
   aexp                { $$ = new Node(z::SEQ,$1); }
 | aexpseq COMMA aexp  { $$ = $1;           $$->addChild($3); }
;

aexp:
   exp             {$$ = $1; }
 | indexset ASSIGN exp
       // Expect indexset outermost operator to be ID, SEQ or INDEX_AND
       // Only get ID when have record field assignment
       {  if ($1->kind == z::ID)
               $$ = new Node(z::ASSIGN, $1->id, $3);
           else
               $$ = new Node(z::ASSIGN, $1, $3);
       }
;

indexset:
   exp   {$$ = $1; }
 | indexset AMPERSAND exp          // Used only on lhs of array assignments
         {
             if ($1->kind == z::INDEX_AND) {
                 $$ = $1;
                 $$->addChild($3);
             } else {
                 $$ = new Node(z::INDEX_AND,$1,$3);
             }
         }
;
exp_id:
   id    { const string& idstr = $1->id;
           if      (idstr == "pending") $$ = new Node(z::PENDING);
           else if (idstr == "true")    $$ = new Node(z::TRUE);
           else if (idstr == "false")   $$ = new Node(z::FALSE);
           else                         $$ = $1;
         }
;


%%
/*
==========================================================================
Additional Code
==========================================================================
*/
void
yy::parser::error (const yy::parser::location_type& l,
                   const std::string& m)
{
    driver.error (m);  // Ignore location, since not maintained in lexer
}



