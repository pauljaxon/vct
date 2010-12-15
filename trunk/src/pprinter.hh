//==========================================================================
//==========================================================================
// PPRINTER.HH
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


#ifndef PPRINTER_HH
#define PPRINTER_HH

#include "box.hh"

// Pretty printer class

class PP {

public:
    static int basicIndent;
    static int linearLimit;

    static string padLeft(const string& s, int width);
    static string padRight(const string& s, int width);

    static Box& makeHSeq(const string& prefix,
                         const string& infix,
                         const string& suffix, 
                         vector<Box*>& bs);

    static Box& makeVSeq(const string& topPrefix,
                         const string& leftPrefix,
                         const string& infix,
                         const string& suffix, 
                         vector<Box*>& bs) ;

    static Box& makeHVSeq(const string& topPrefix,
                          const string& leftPrefix,
                          const string& infix,
                          const string& suffix, 
                          vector<Box*>& bs) ;

    static Box& simpleHSeq(vector<Box*>& bs);
    static Box& simpleVSeq(vector<Box*>& bs);
    static Box& simpleHVSeq(vector<Box*>& bs);

    // Keep infix argument.  Allows setting of indentation, even if
    // leftPrefix == ""
    static Box& makeHVSingleton(const string& topPrefix,
                                const string& leftPrefix,
                                const string& infix,
                                const string& suffix, 
                                Box& b) {
        vector<Box*> bs(1);
        bs[0] = &b;
        return makeHVSeq(topPrefix, leftPrefix, infix, suffix, bs);
    }

    static Box& makeHVPair(const string& topPrefix,
                           const string& leftPrefix,
                           const string& infix,
                           const string& suffix, 
                           Box& b1,
                           Box& b2) {
        vector<Box*> bs(2);
        bs[0] = &b1;
        bs[1] = &b2;
        return makeHVSeq(topPrefix, leftPrefix, infix, suffix, bs);
    }

    /*
    static Box* makeRInfixSeq(string infixSym, vector<Box*>* bs, PPMode m);
    static Box* makeLLLSeq(string startD, string sep, 
                           string endD, Box* b, PPMode m);
    static Box* addDelimsTB(string startD, string endD, Box* b, PPMode m);
    static Box* addDelimsLL(string startD, string endD, Box* b, PPMode m);
    */
};

#endif // ! defined PPRINTER_HH
