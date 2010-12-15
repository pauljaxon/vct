//==========================================================================
//==========================================================================
// NODE-UTILS.HH
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



#ifndef NODE_UTILS_HH
#define NODE_UTILS_HH

#include <sstream>

#include "node.hh"


//========================================================================
// Translator base class
//========================================================================
//
// To create Node to Node translator for application X:
//
// 1. Subclass Translator with XTranslator.

// 2. Define virtual function translateAux in subclass.
//    As needed, this can call the error function to log errors.

// 3. Define constructor for XTranslator that sets the name string to "X".

// 4. Define function xTranslate which constructs an XTranslator
//    object and calls the translate virtual function on it.
//  
//    translate will accumulate errors and throw an exception if
//    any errors have been logged.
// 
//    Doesn't matter whether xTranslate is a static function of XTranslator
//    or a file level function.

//    [ Used to be that error message always included printouts of
//    before and after translation trees.  Then it was required that
//    xTranslate should set Formatter appropriately for application X.
//    Now error messages do not include these printouts, so setting
//    formatter is no longer necessary ]


class Translator {

public:

    Node* translate (Node* oldN);

    virtual ~Translator() {};
private:
    bool errorFlag;   
    std::ostringstream errorMessages;

protected:
    std::string name;
    Translator(string s) : name(s) {};

    virtual Node* translateAux (Node* oldN) = 0;
    void error(const std::string& s);


};


#endif // ! NODE_UTILS_HH
