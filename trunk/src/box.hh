//==========================================================================
//==========================================================================
// BOX.HH
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


#ifndef BOX_HH
#define BOX_HH

#include <iostream>   
using std::ostream;

#include <string>
using std::string;

#include <vector>
using std::vector;

// For holding formatted text

// Box are rectangles except that last line might be shorter.

// Box printing:
// Precond: pt is at current indent.  Indent supplied as arg
// printing routine adds indent of spaces after each linebreak
// pt left at rh side of last line output.

// Constructors defined as static members to abstract away from
// implementation choices.



  
class Box {
private:
    static vector<Box*> allocList;
    static int allocCount;

protected:         // Assume subclasses set values directly.
    int height;
    int width;
    int lastWidth; // Width of last line (maybe shorter).

    Box();
    virtual ~Box(); // Needed so delete on Box* pointers correctly frees
                    // storage used by objects from subclasses.
                    // See Stroustrup, p422.

public:
    /* constructors */

    Box* hAppendT(Box* b); // T = Top align. this->height == 1
    Box* hAppendB(Box* b); // B = Bottom align. b.height == 1
    Box* hAppend(Box* b);  
    Box* vAppend(Box* b);

    /* accessors */
    int getWidth() {return width;}
    int getHeight() {return height;}
    int getLastWidth() {return lastWidth;} 

    static int getAllocCount() {return allocCount;} 

    /* utilities */

    static bool allHeightOne(vector<Box*>& bs) {
        for (vector<Box*>::iterator i = bs.begin(); i != bs.end(); i++) {
            if ( (*i)->getHeight() > 1 ) return false;
        }
        return true;
    }
    
    static int sumWidths(vector<Box*>& bs) {
        int sum = 0;
        for (vector<Box*>::iterator i = bs.begin(); i != bs.end(); i++) {
            sum += (*i)->getWidth();
        }
        return sum;
    }

    static void deleteAll();

    // Not for public use. However compiler doesn't permit 
    // recursive invocations in subclasses if protected.

    virtual void indentPrint(ostream& os, int indent) = 0;
};


Box& box(const string& s);
Box& box(const char* cstr);


// Appending strings, cstrings and boxes.

Box& operator+ (Box& b1, Box& b2);
Box& operator+ (const string& s1, Box& b2);
Box& operator+ (Box& b1, const string& s2);
Box& operator+ (const char* cs1, Box& b2);
Box& operator+ (Box& b1, const char* cs2);
Box& operator/ (Box& b1, Box& b2);


// Printing boxes
ostream& operator<< (ostream& os, Box& b);



#endif // ! BOX_HH

