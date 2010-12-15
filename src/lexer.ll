
/*
=============================================================================
=============================================================================
LEXER.LL
=============================================================================
=============================================================================

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


/* Lexer for language used in fdl, rls and vcg files. */

/* Structure influenced by calc++ example distributed with Bison. */




%{
#include <cstdlib>
#include <string>
#include "parser.tab.hh" /* For token type */
#include "lexer.hh"      /* For YY_DECL macro giving lexing function type */

// #include "pdriver.hh"

#define yyterminate() return tok::FILE_END

typedef yy::parser::token tok;
%}

%option noyywrap nounput batch debug

DIGIT      [0-9]
ID         [A-Za-z][A-Za-z0-9_]*[~]?
COMMENT    "/*"([^*]+|[*]+[^*/])*"*"+"/"
WHITESPACE [ \t\n\r]+

%x body goalorigins

%%

<*>{
^"rule_family"     { BEGIN(body); return tok::RULE_FAMILY; }
^"title"           { BEGIN(body); return tok::TITLE; }  
^"For"             { BEGIN(goalorigins); return tok::FOR; }  

{WHITESPACE}       /* eat up whitespace */

}

<goalorigins>[^:]+ { BEGIN(body); 
                     yylval->sval = new std::string(yytext);
                     return tok::GOAL_ORIGINS; 
                   }

<INITIAL>.         /* ignore non-trigger characters in header */

<body>{

":"                { return tok::COLON; }
"["                { return tok::LSB; }
"]"                { return tok::RSB; }
"("                { return tok::LPAREN; }
")"                { return tok::RPAREN; }
","                { return tok::COMMA; }
"&"                { return tok::AMPERSAND; }
";"                { return tok::SEMIC; }
"="                { return tok::EQ; }
"."                { return tok::DOT; }
".."               { return tok::DOTDOT; }
requires           { return tok::REQUIRES; }
may_be_replaced_by  { return tok::MAY_BE_REPLACED_BY; }
may_be_deduced      { return tok::MAY_BE_DEDUCED; }
may_be_deduced_from { return tok::MAY_BE_DEDUCED_FROM; }
are_interchangeable { return tok::ARE_INTERCHANGEABLE; }
if                 {return tok::IF; }
end                {return tok::END; }
task_type          { // task_type can also be a valid identifier, so we need to
                     // have a useful sval.
                     yylval->sval = new std::string(yytext);
		     return tok::TASK_TYPE; }
function           { return tok::FUNCTION; }
procedure          { return tok::PROCEDURE; }
type               { return tok::TYPE; }
var                { return tok::VAR; }
const              { return tok::CONST; }
array              { return tok::ARRAY; }
record             { return tok::RECORD; }
":="               { return tok::ASSIGN; }
of                 { return tok::OF; }
"**"               { return tok::STARSTAR; }
"*"                { return tok::STAR; }
"/"                { return tok::SLASH; }
div                { return tok::DIV; }
mod                { return tok::MOD; }
"+"                { return tok::PLUS; }
"-"                { return tok::MINUS; }
"<>"               { return tok::NE; }
"<"                { return tok::LT; }
">"                { return tok::GT; }
"<="               { return tok::LE; }
">="               { return tok::GE; }
"<->"              { return tok::IFF; }
"->"               { return tok::IMPLIES; }
not                { return tok::NOT; }
and                { return tok::AND; }
or                 { return tok::OR; }
for_some           { return tok::FOR_SOME; }
for_all            { return tok::FOR_ALL; }
"***"              { return tok::TRIPLESTAR; }
"!!!"              { return tok::TRIPLEBANG; }


"task_type_"{ID} |
"function_"{ID}  |
"procedure_"{ID}    { yylval->sval = new std::string(yytext);
                      return tok::SUBPROG_ID; 
                    }

"C"{DIGIT}+         { yylval->sval = new std::string(yytext);
                      return tok::CONCL_ID; 
                    }

"H"{DIGIT}+         { yylval->sval = new std::string(yytext);
                      return tok::HYP_ID; 
                    }

{ID}                { yylval->sval = new std::string(yytext); return tok::ID; }
{DIGIT}+            { yylval->sval = new std::string(yytext); 
                      return tok::NATNUM; 
                    }

{COMMENT}          {}     /* Eat up any C-style comments */

}

     
%%

void
pdriver::scan_begin ()
{
  yy_flex_debug = trace_scanning;
  if (!(yyin = fopen (file.c_str (), "r")))
    error (std::string ("cannot open ") + file);

  // flex doesn't reset start condition on 2nd and subsequent scans, 
  // so here we reset it explicitly.
  BEGIN(INITIAL); 
}

void
pdriver::scan_end ()
{
  fclose (yyin);
}
