
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
PROLOGBLOCKCOMMENT  "/*"([^*]+|[*]+[^*/])*"*"+"/"
PROLOGLINECOMMENT   "%".*$
FDLCOMMENT          "{"[^}]*"}"
WHITESPACE [ \t\n\r]+
VCGHEADER  (.*\n){13}

%x VCGInitial PrologBody FDLBody GoalOrigins

%%

%{
  // Code for start of yylex to inject alternate start tokens into token
  // stream for different file types
  if (driver.at_start)  {

      driver.at_start = false;
      if (driver.currentFileType == pdriver::FDL)
          return tok::START_FDL_FILE;
      if (driver.currentFileType == pdriver::RULE)
          return tok::START_RULE_FILE;
      else // driver.currentFileType == pdriver::VCG
          return tok::START_VCG_FILE;
  }
%}


<*>{WHITESPACE}       /* eat up whitespace */

<VCGInitial>.         /* ignore non-trigger characters in VCG file header */

<VCGInitial,PrologBody>^"For" { BEGIN(GoalOrigins); return tok::FOR; }

<GoalOrigins>[^:]+ { BEGIN(PrologBody);
                     yylval->sval = new std::string(yytext);
                     return tok::GOAL_ORIGINS;
                   }

<PrologBody>^rule_family/" "   { return tok::RULE_FAMILY; }

<FDLBody,PrologBody>{

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
package            { // ditto
                     yylval->sval = new std::string(yytext);
		     return tok::PACKAGE; }
package_spec       { // ditto
                     yylval->sval = new std::string(yytext);
		     return tok::PACKAGE_SPEC; }
package_body       { // ditto
                     yylval->sval = new std::string(yytext);
		     return tok::PACKAGE_BODY; }
title              { // Like task_type, both a keyword and an identifier
                     yylval->sval = new std::string(yytext);
		     return tok::TITLE; }
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


"package_spec_"{ID}  |
"package_body_"{ID}  |
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


}

<PrologBody>{
{PROLOGBLOCKCOMMENT}          {}  /* Skip comment */
{PROLOGLINECOMMENT}           {}  /* Skip comment */
}

<FDLBody>{FDLCOMMENT}         {}  /* Skip comment */


%%

void
pdriver::scan_begin ()
{
  yy_flex_debug = trace_scanning;
  if (!(yyin = fopen (file.c_str (), "r")))
    error (std::string ("cannot open ") + file);

  // flex doesn't reset start condition on 2nd and subsequent scans,
  // so here we reset it explicitly.

  // Set start condition appropriate for file.
  // Doing this also addresses issue that flex doesn't reset start
  // condition on 2nd and subsequent scans,

  if (currentFileType == FDL) { BEGIN(FDLBody); }
  else if (currentFileType == RULE) { BEGIN(PrologBody);  }
  else { // currentFileType == VCG
      BEGIN(VCGInitial);
  }

  // Set flag for triggering return of file-type-specific start token
  at_start = true;
}

void
pdriver::scan_end ()
{
  fclose (yyin);
}
