//==========================================================================
//==========================================================================
// FORMATTER.HH
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


#ifndef FORMATTER_HH
#define FORMATTER_HH

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <map>
using std::map;

#include <iostream>
using std::ostream;
using std::endl;

#include "box.hh"
#include "node.hh"

/* 
A formatter specifies how to create a box tree from a node tree.

It provides to other formatting code:

1. a parenthesisation function
   Input:
      kind of current node
      kind of parent of current node
      number indicating which subterm of the parent the current node is
      a formatted box for the current node

   Output:
      a box for the formatted node, possibly with parentheses added

2. A Box constructor, taking as input:
     a node kind
     a node id
     boxes for each of children
   and returns
     a box for term.


To initialise a formatter, one provides

1. For each node kind used for an operator 
   (a term with open left and/or right sides)

   a) a precedence number
   b) a list of subnode paren control constants, one for each of subterms.
      Options are:
        L  Suppress parens if parent prec < subnode prec
        E  Suppress parens if parent prec <= subnode prec 
        *  Suppress parens always

     E.g. a left associative operator + would use "EL", allowing 
     (a + b) + c to be displayed as a + b + c.

     List provided simply as string: e.g. "L*E"

     This provides the information for controlling the parenthesisation 
     function.

     In special cases, the control constant for the subterm i is not
     the character i in the list:

     - List is "EL" (left assoc operator) & operator arity n > 2:
       subterm gets E if i = 0. o/w gets L

     - List is "LE" (right assoc operator) & operator arity n > 2:
       subterm gets E if i for last child. o/w gets L

     - i >= length of list:
       subterm gets last constant in list.

     The idea of the first 2 is to treat an n-ary version of a
     normally binary operator as an iterated version of the binary
     operator, associated the standard parenthesis-avoiding way for
     the operator.


2. The box constructor function.


Parenthesisation controlled entirely by first map.


Formatters provided for 

a) Vanilla node tree syntax

*/


class Formatter {
public:
    // functions for formatting code

    Box& addParens(z::Kind currentK,
                   z::Kind parentK,
                   int childNum,
                   int parentArity,
                   Box& currentBox);

    virtual Box& addSyntax(z::Kind k, const string& id,
                           vector<Box*>& bs) = 0;

    virtual ~Formatter() {}
protected:
    // For use in constructors of concrete subclasses.

    void addOpInfo(z::Kind k, int prec, const string& subParens);

private:
    map<z::Kind, int> precMap;
    map<z::Kind, string> subParensMap;

// Static members
private:
    static Formatter* formatter;
public:
    static void setFormatter(Formatter* f);

    static Box& format(const Node& n);

protected:    
    static Box& makeStringAp(const std::string& id,  std::vector<Box*>& bs);
    
};


ostream& operator<<(ostream& os, const Node& n);


// These classes included here to provide access to getFormatter() methods.

class VanillaFormatter : public Formatter {
public:
    virtual Box& addSyntax(z::Kind k, const string& id,
                           vector<Box*>& bs);


    static Formatter* getFormatter();

private:
    VanillaFormatter();
    static Formatter* instance;
};


class TestFormatter : public Formatter {
public:
    virtual Box& addSyntax(z::Kind k, const string& id,
                           vector<Box*>& bs);

    static Formatter* getFormatter();

private:
    static Formatter* instance;
    TestFormatter();
};




#endif // ! defined FORMATTER_HH
