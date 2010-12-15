//==========================================================================
//==========================================================================
// BIGNUM.HH
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


#ifndef BIGNUM_HH
#define BIGNUM_HH


#include "node.hh"

#include <string.h>  // Put string decls into global namespace.
                     // Already in std namespace because <string>
                     // included in node.hh.  Apparently need this in
                     // some cases
#include <gmpxx.h>

//========================================================================
// MyInt
//========================================================================
// Wrapper for GMP multiple precision integers.
//

class MyInt {
private:

    mpz_class num;

public:


    MyInt(Node* n); // Assume n satisfies isInt().
    MyInt(const std::string& s); 

    MyInt(const mpz_class& z) : num(z) {};
    MyInt() : num(mpz_class(0)) {};
    
    Node* toNode();

    bool operator==(const MyInt& a) const;
    bool operator<(const MyInt& a) const;

    std::string toString() const;

    static bool isInt(Node* n);

    static MyInt uminus(const MyInt& a);
    static MyInt plus(const MyInt& a, const MyInt& b);
    static MyInt minus(const MyInt& a, const MyInt& b);
    static MyInt times(const MyInt& a, const MyInt& b);
    static MyInt div(const MyInt& a, const MyInt& b);
    static MyInt rem(const MyInt& a, const MyInt& b);
    static MyInt mod(const MyInt& a, const MyInt& b);
    static MyInt exp(const MyInt& a, const MyInt& b);
};

//========================================================================
// MyInt-related utility functions
//========================================================================

bool isIntNode(Node* n);
void groundEval(Node* n);
void constArithEval(Node* n);




#endif // ! BIGNUM_HH
