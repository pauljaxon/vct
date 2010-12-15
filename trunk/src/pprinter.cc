//==========================================================================
//==========================================================================
// PPRINTER.CC
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



#include "pprinter.hh"


#include <algorithm>  // for max function.
using std::max;

int PP::basicIndent = 2;
int PP::linearLimit = 60;


string PP::padLeft(const string& s, int width) {
    if ((int) s.size() < width) 
        return string(width - s.size(), ' ') + s;
    else
        return s;
}

string PP::padRight(const string& s, int width) {
    if ((int) s.size() < width)
        return s + string(width - s.size(), ' ');
    else
        return s;
}

/*
Glue boxes horizontally 

  P Box0 I Box1 I ... I Boxn S

Expect that all Boxes height 1, though 1 with greater height should be OK.

Whitespace insertion is on safe side here.  Could pass in additional
spaces as extra parameters with default values for convenience.
*/

Box& 
PP::makeHSeq(const string& prefix, const string& infix, const string& suffix, 
             vector<Box*>& bs) {

    Box* accum;
    string paddedInfix(
	infix.size() == 0 ? " " : " " + infix + " "
	);

    if (bs.size() == 0) {

        return box(prefix + suffix);  // Assume no whitespace needed 

    } else {

        accum = bs[0];
        
        for (vector<Box*>::iterator i = ++ bs.begin();
             i != bs.end();
             i++) {
        
            accum = & (*accum + paddedInfix + **i );
        }
    }
    if (prefix.size() != 0) accum = & (prefix + " " + *accum) ;
    if (suffix.size() != 0) accum = & (*accum + " " + suffix) ;
    return *accum;
}


/*
Make vertical sequence of form

T
L  Box0
I  Box1
...
I  Boxn
S

As needed, L, I are padded on right to make them equal length, to
ensure that at least 1 space after each, and to ensure indent is at
least basicIndent.

Line for T only added if T not ""
Line for S only added if S not ""

If both T and L are "", then Box0 is not indented at all.

*/

Box&
PP::makeVSeq(const string& topPrefix, const string& leftPrefix,
             const string& infix, const string& suffix, 
             vector<Box*>& bs) {

    if (bs.size() == 0) {
        return  box(topPrefix + leftPrefix) / box(suffix);
    }
    // bs.size != 0 from now on

    // Indent at least basicIndent and ensure at least one space after
    // leftPrefix and infix tokens.
    int leaderWidth = max((int) max(leftPrefix.size() + 1, infix.size() + 1),
			  PP::basicIndent);
    
    // leading strings on lines
    
    string firstLeader = padRight(leftPrefix, leaderWidth);
    string restLeader = padRight(infix, leaderWidth);

    // Special case. If no prefix, first box must start right away

    if (topPrefix.size() == 0 && leftPrefix.size() == 0)
        firstLeader = "";

    Box* accum = & (firstLeader + *bs[0] );

    for (vector<Box*>::iterator i = ++bs.begin();
         i != bs.end();
         i++) {
        
        accum = & ( *accum / (restLeader + **i)  );
    }

    if (topPrefix.size() != 0)
        accum = & (box(topPrefix) / *accum );

    if (suffix.size() != 0)
        accum = & (*accum / box(suffix) );

    return *accum;
}


Box& 
PP::makeHVSeq(const string& topPrefix, const string& leftPrefix,
             const string& infix, const string& suffix, 
             vector<Box*>& bs) {

    if (Box::allHeightOne(bs) ) {

        string prefix (topPrefix + leftPrefix); // Assume no space needed if
                                                // Both not "".
        
        Box& hbox = makeHSeq(prefix, infix, suffix, bs);

        if (hbox.getWidth() <= linearLimit)
            return hbox;
    }
    // If get here, only vertical layout is acceptable.

    return makeVSeq(topPrefix, leftPrefix, infix, suffix, bs);
}



Box& 
PP::simpleHSeq(vector<Box*>& bs) {

    Box* accum;

    if (bs.size() == 0) {

        return box("");

    } else {

        accum = bs[0];
        
        for (vector<Box*>::iterator i = ++ bs.begin();
             i != bs.end();
             i++) {
        
            accum = & (*accum + box(" ") + **i );
        }
    }
    return *accum;
}


/*
Make vertical sequence of form

Box0
Box1
...
Boxn


*/

Box&
PP::simpleVSeq(vector<Box*>& bs) {

    if (bs.size() == 0) {
        return  box("");
    }
    // bs.size != 0 from now on

    Box* accum = bs[0];

    for (vector<Box*>::iterator i = ++bs.begin();
         i != bs.end();
         i++) {
        
        accum = & ( *accum / (**i)  );
    }
    return *accum;
}


Box& 
PP::simpleHVSeq(vector<Box*>& bs) {

    if (Box::allHeightOne(bs) ) {
        
        Box& hbox = simpleHSeq(bs);

        if (hbox.getWidth() <= linearLimit)
            return hbox;
    }
    // If get here, only vertical layout is acceptable.

    return simpleVSeq(bs);
}


/*
Box* 
PP::makeLLLSeq(string startD, string sep, string endD, Box* b, PPMode m);

Box* 
PP::addDelimsTB(string startD, string endD, Box* b, PPMode m);
    
Box* 
PP::addDelimsLL(string startD, string endD, Box* b, PPMode m);
*/
