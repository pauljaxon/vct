//==========================================================================
//==========================================================================
// NODE-UTILS.CC
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



#include <stdexcept>

using std::string;

#include <iostream>
using std::endl;

#include "node-utils.hh"
#include "utility.hh"

#include "formatter.hh"

using namespace z;


//========================================================================
// Translator base class
//========================================================================
//
// Provides error reporting and handling for Node->Node translation functions.

Node*
Translator::translate (Node* oldN) {
    errorFlag = false;

    errorMessages << name + " translator error(s): " << endl;
    
    Node* result = translateAux(oldN);
    if (errorFlag) {
        
        Formatter::setFormatter(VanillaFormatter::getFormatter());

        // Printing whole parse trees is too verbose.  Was OK at one
        // point when decls, rules, hyps or concls were translated one
        // at a time, but now whole units translated at once.

        /*
        errorMessages << "translation input: " << endl;
        errorMessages << *oldN << endl;
        errorMessages << "translation result: " << endl;
        errorMessages << *result << endl;
        */

        throw std::runtime_error(errorMessages.str());
    }
    return result;
}

 
void
Translator::error (const string& message) {
    errorFlag = true;
    errorMessages << message << endl;
    return;
}



