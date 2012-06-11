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

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <cassert>
#include <stdexcept>

#include "utility.hh"

int
main (int argc, char *argv[]) {

    assert(stringMatch("abc","abc"));
    assert(!stringMatch("abc","abcd"));
    assert(!stringMatch("abcd","abc"));
    assert(stringMatch("",""));
    assert(!stringMatch("ab",""));
    assert(!stringMatch("","ab"));

    assert(stringMatch("a?c","abc"));
    assert(stringMatch("?bc","abc"));
    assert(stringMatch("ab?","abc"));
    assert(!stringMatch("ab?d","abc"));

    assert(stringMatch("a*bc","abc"));
    assert(stringMatch("a*bc","axbc"));
    assert(stringMatch("a*bc","axxbc"));

    assert(stringMatch("ab*","ab"));
    assert(stringMatch("ab*","abc"));
    assert(stringMatch("ab*","abcde"));

    assert(!stringMatch("ab*?","ab"));

    assert(!stringMatch("ab*?","abc"));  // NB: violation of naive expectation
    assert(!stringMatch("ab*?","abcd")); 
    assert(!stringMatch("ab*?","abcde"));

    assert(!stringMatch("ab?*","ab"));
    assert(stringMatch("ab?*","abc"));
    assert(stringMatch("ab?*","abcd"));
    assert(stringMatch("ab?*","abcde"));

    cout << "All tests passed" << endl;
    return 0;
}
