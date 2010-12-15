//==========================================================================
//==========================================================================
// LEXER.HH
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



#ifndef LEXER_HH
#define LEXER_HH

#include "parser.tab.hh"
#include "pdriver.hh"

// Announce to Flex the prototype we want for lexing function, ...
// 
// Use of driver arg here is not strictly necessary.  Lexer never 
// accesses driver object.  (In calc++ example, the lexer reported error
// messages to the driver)
// 
# define YY_DECL                                 \
  yy::parser::token_type                         \
  yylex (yy::parser::semantic_type* yylval,      \
         pdriver& driver)

// Declare lexing function for use in parser

YY_DECL;


#endif // ! LEXER_HH
