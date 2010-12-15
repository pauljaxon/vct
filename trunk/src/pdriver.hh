//==========================================================================
//==========================================================================
// PDRIVER.HH
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

// 

/* 
Driver for parser, updated for Bison 2.3.
Needs updating for Bison 2.4...

This file presents public interface of parser.

Unlike calc++ example from bison distribution, the function prototype for
yylex is moved to a separate file lexer.hh. 

Not only does this keep yylex private, but also prevents leak of
YY_DECL macro definition used by lexer implementation into public
space.

*/


#ifndef PDRIVER_HH
#define PDRIVER_HH

#include <string>
#include "node.hh"


class pdriver
{
public:
    pdriver ();
    virtual ~pdriver ();

    Node* result;

    // Handling the lexer
    void scan_begin ();
    void scan_end ();
    bool trace_scanning;

    // Handling the parser.

    int parse (const std::string& f);
    std::string file;
    bool trace_parsing;

    // Error handling.
    void error (const std::string& m);
    bool errorFlag;
};
#endif // ! PDRIVER_HH
