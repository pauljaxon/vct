//==========================================================================
//==========================================================================
// BOX.CC
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


#include <cassert>

#include <algorithm>  // for max function.

#include "box.hh"

using std::endl;
using std::ostream;
using std::string;

/* Implementation of box class */

/* Design notes:

* Disjoint union implemented using subclasses 

* Expect users to use explicit constructor methods and functions in main 
  namespace.

  Considered defining constructors for Box(string) and Box(char*) and 
  using main class Box for string boxes. However

  - implicit conversions using such constructors sometimes will use stack 
    allocation which is wrong.  Will almost invariably want lifetime of 
    object to 
    outspan the lexical scope of the point where the implicit conversion is
    inferred.
  - explicit ones verbose (need Box::new)
  - explicit ones have wrong return type for use with references (want
    ref types to allow use of overloaded * and + operators

Memory management. 

- Expect all boxes to be allocated on heap.
- Reference to each box kept. 
  Boxes reclaimed en masse by explicit function call.


*/





// ========================================================================
// Box sub-classes
// ========================================================================

class StrBox : public Box {
private:
    string str;

public:
    StrBox(const string& s) : str(s) {
        height = 1;
        width = s.size();
        lastWidth = width;
    }

    virtual void indentPrint(ostream& os, int indent) {
        os << str;
        return;
    }

};

Box& box(const string& s) { return * new StrBox(s); }

Box& box(const char* cstr) {
    string s(cstr);
    return box(s);
}





class HBoxTAlign : public Box {
public:
    Box*    lbox; // Always height 1
    Box*    rbox;

    HBoxTAlign(Box* b1, Box* b2) : lbox(b1), rbox(b2) {
        height = rbox->getHeight();
        width = lbox->getWidth() + rbox->getWidth();
        lastWidth = lbox->getWidth() + rbox->getLastWidth();
    }

    virtual void indentPrint(ostream& os, int indent) {
        lbox->indentPrint(os, indent);  // Argument irrelevant
        rbox->indentPrint(os, indent + lbox->getWidth());
        return;
    }
};


class HBoxBAlign : public Box {
public:
    Box* lbox;
    Box* rbox; // Always height 1

    HBoxBAlign(Box* b1, Box* b2) : lbox(b1), rbox(b2) {
        height = lbox->getHeight();
        lastWidth = lbox->getLastWidth() + rbox->getWidth();
        width = std::max(lastWidth, lbox->getWidth());
    }

    virtual void indentPrint(ostream& os, int indent) {
        lbox->indentPrint(os, indent);
        rbox->indentPrint(os, indent);
        return;
    }


};

class VBox : public Box {
public:
    Box* tbox;
    Box* bbox;

    VBox(Box* tb, Box* bb) : tbox(tb), bbox(bb) {
        height = tbox->getHeight() + bbox->getHeight();
        width = std::max(tbox->getWidth(), bbox->getWidth()); 
        lastWidth = bbox->getLastWidth();
    }

    virtual void indentPrint(ostream& os, int indent) {
        tbox->indentPrint(os, indent);
        os << endl;
        for (int i = 0; i < indent; i++) os << " ";
        bbox->indentPrint(os, indent);
        return;
    }

};

// ========================================================================
// Box functions and static field defs.
// ========================================================================

vector<Box*> Box::allocList;  // Static field definition.
int Box::allocCount = 0;

Box::Box() { allocList.push_back(this); allocCount++;}
Box::~Box () {}

void
Box::deleteAll() {
    for (  vector<Box*>::iterator i = allocList.begin();
           i != allocList.end();
           i++) {
        delete (*i);
    }
    allocList.clear();
    return;
}


// Ideally constructors of subboxes should not be in main namespace.
// If made private, could probably use friend declaration to allow their
// visibility in Box.


Box*
Box::hAppendT(Box* b) {
  assert(this->getHeight() == 1);
  return new HBoxTAlign(this,b);
}

Box*
Box::hAppendB(Box* b) {
  assert(b->getHeight() == 1);
  return new HBoxBAlign(this,b);
}

Box*
Box::hAppend(Box* b) {
  if (this->getHeight() == 1)
    return hAppendT(b);
  else
    return hAppendB(b);
}

Box* 
Box::vAppend(Box* b) {return new VBox(this,b);}



// Overloaded operators for appending strings, cstrings and boxes.

// const declarations here are *necessary*.

Box& operator+ (Box& b1, Box& b2) {return *(b1.hAppend(&b2));}
Box& operator+ (const string& s1, Box& b2) {
    Box& b1 = box(s1);
    return *(b1.hAppend(&b2));
}
Box& operator+ (Box& b1, const string& s2) {
    Box& b2 = box(s2);
    return *(b1.hAppend(&b2));
}

Box& operator+ (const char* cs1, Box& b2) {
    Box& b1 = box(cs1);
    return *(b1.hAppend(&b2));
}
Box& operator+ (Box& b1, const char* cs2) {
    Box& b2 = box(cs2);
    return *(b1.hAppend(&b2));
}

Box& operator/ (Box& b1, Box& b2) {return *(b1.vAppend(&b2));}




std::ostream&
operator<< (std::ostream& os, Box& b) {
    b.indentPrint(os, 0);
    return os;
}

