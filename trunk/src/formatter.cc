//==========================================================================
//==========================================================================
// FORMATTER.CC
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

#include "formatter.hh"
#include "pprinter.hh"

#include <sstream>
using std::ostringstream;

Box& 
Formatter::addParens(z::Kind kind,
                     z::Kind parentKind,
                     int childNum,
                     int parentArity,
                     Box& b) {
    
    // Decide whether to parenthesise

    // Suppress parentheses if either node or parent is not an operator

    if (subParensMap.count(parentKind) == 0
        || subParensMap.count(kind) == 0)
        return b;

    string& s = subParensMap.find(parentKind)->second;

    // Decide format character, as discussed in formatter.hh

    char formatChar;

    if (s == "EL" && parentArity > 2)
        formatChar = (childNum == 0 ) ? 'E' : 'L';

    else if (s == "LE" && parentArity > 2)
        formatChar = (childNum == parentArity - 1) ? 'E' : 'L';

    else if (childNum >= (int) s.size())
        formatChar = s.at(s.size() - 1);

    else
        formatChar = s.at(childNum);

    int prec = precMap.find(kind)->second;
    int parentPrec = precMap.find(parentKind)->second;
    
    if ( formatChar == '*'
         || (formatChar == 'E' && parentPrec <= prec)
         || (formatChar == 'L' && parentPrec < prec)
         ) {
        return b; // return if parentheses around operator should be suppressed
    }
    // Do parenthesisation

    return PP::makeHVSingleton("", "(", "", ")",b);
    
    
};




// STATIC members

// See below Vanilla Formatter for initialisation of formatter field.

// Formatter*
//Formatter::formatter = VanillaFormatter::getFormatter();  // Works???


Box& 
Formatter::format(const Node& node) {

    vector<Box*> subBoxes;

    for (int i = 0; i != node.arity(); i++)
    {
        Node* subNode = node.child(i);
        Box& wrappedSubBox = formatter->addParens(subNode->kind,
                                                  node.kind,
                                                  i,
                                                  node.arity(),
                                                  format(*subNode));
        subBoxes.push_back(&wrappedSubBox);
    }
    
    return formatter->addSyntax(node.kind, node.id, subBoxes);
}


void
Formatter::addOpInfo(z::Kind k, int prec, const std::string& subParens) {
    precMap.insert(std::make_pair(k,prec));
    subParensMap.insert(std::make_pair(k,subParens));
}

Box&
Formatter::makeStringAp(const string& id,
                        std::vector<Box*>& bs) {
    return PP::makeHVSeq("(" + id, "", "", ")", bs);
}




//==========================================================================
// VANILLA FORMATTER
//==========================================================================


Box& VanillaFormatter::addSyntax(z::Kind k, const std::string& id,
                                 std::vector<Box*>& bs) {

    string header = z::kindString(k)
        + (id != ""
           ?  "{" + id + "}"
           : "");

    if (bs.size() == 0)
        return box(header);
    else
        return PP::makeHVSeq(header,
                             "[",
                             ";",
                             "]",
                             bs
            );
}

Formatter*
VanillaFormatter::getFormatter() {
    if (instance == 0) instance = new VanillaFormatter;
    return instance;
}

VanillaFormatter::VanillaFormatter() {};

Formatter*
VanillaFormatter::instance = 0;


Formatter*
Formatter::formatter = VanillaFormatter::getFormatter();  


//==========================================================================
// TEST FORMATTER
//==========================================================================


using namespace z;

Box& TestFormatter::addSyntax(z::Kind k, const std::string& id,
                              std::vector<Box*>& bs) {

    switch (k) {
    case(VCG_FILE):
        return PP::makeVSeq("vcgfile", " ", " ", "", bs); // indent all goals
    case(GOAL):
        return PP::makeVSeq("goal", "  ", "->", "", bs);

    case(SEQ):
        return PP::makeHVSeq("", "", ",", "", bs);

    case(DECL):
        return PP::makeHVSeq("", id + ":", "", "", bs);
        
    // TYPES

    case(INT_TY): return box("INT");
    case(REAL_TY): return box("REAL");
    case(BOOL_TY): return box("BOOL");
    case(SUBRANGE_TY): return PP::makeHVSeq("", "{", "..", "}", bs);
    case(ENUM_TY): return PP::makeHVSeq("",
                                        id + "{",
                                        ",",
                                        "}",
                                        bs);
    case(ARRAY_TY):
        return PP::makeHVSeq("array{" + id + "}",
                            "[",
                            "] of ",
                            "",
                            bs);
    case(RECORD_TY):
        return PP::makeHVSeq("record{" + id + "}",
                            "(",
                            ",",
                            ")",
                            bs);
    case(TYPE_ID):    
    case(TYPE_PARAM):
        return box(id);

    // EXPRESSIONS

    case(FORALL):
        return PP::makeHVSeq("", "All", ". ", "", bs);
    case(EXISTS):
        return PP::makeHVSeq("", "Exists", ". ", "", bs);

    case(IMPLIES):
        return PP::makeHVSeq("", "", "=>", "", bs);
    case(IFF):
        return PP::makeHVSeq("", "", "<=>", "", bs);
    case(OR):
        return PP::makeHVSeq("", "", "\\/", "", bs);
    case(AND):
        return PP::makeHVSeq("", "", "/\\", "", bs);
    case(NOT):
        return PP::makeHVSeq("!", "", "", "", bs);

    case(TERM_IFF):
        return PP::makeHVSeq("", "", "<=>b", "", bs);
    case(TERM_OR):
        return PP::makeHVSeq("", "", "\\/b", "", bs);
    case(TERM_AND):
        return PP::makeHVSeq("", "", "/\\b", "", bs);
    case(TERM_NOT):
        return PP::makeHVSeq("!b", "", "", "", bs);

    case(I_LE):
        return PP::makeHVSeq("", "", "<=", "", bs);
    case(I_LT):
        return PP::makeHVSeq("", "", "<", "", bs);

    case(EQ): {
        std::vector<Box*> newBs;
        newBs.push_back(bs[0]);
        newBs.push_back( &(box("= ") + *(bs[1])));
        if (bs.size() == 3) {
            newBs.push_back(&(box("in ") + *(bs[2])));
        }
        return PP::simpleHVSeq(newBs);

    }
    case(I_PLUS): 
        return PP::makeHVSeq("", "", "+", "", bs);
    case(I_MINUS):
        return PP::makeHVSeq("", "", "-", "", bs);
    case(I_TIMES):
        return PP::makeHVSeq("", "", "*", "", bs);
    case(IDIV):
        return PP::makeHVSeq("", "", "div", "", bs);
    case(MOD):
        return PP::makeHVSeq("", "", "mod", "", bs);
    case(EXP):
        return PP::makeHVSeq("", "", "**", "", bs);

    case(I_UMINUS):
        return PP::makeHVSeq("", "-", "", "", bs);

    case(ID):
        return box(id);
    case(VAR):
        return box("?" + id);
    case(CONST):
        return box(id);
    case(PENDING):
        return box("pending");
    case(NATNUM):
        return box(id);

    case(FUN_AP):
        return PP::makeHVSeq(id + "(", "", ",", ")", bs);
    case(PRED_AP):
        return PP::makeHVSeq(id + "[", "", ",", "]", bs);

    default: {
        if (id.size() == 0) 
            return PP::makeHVSeq(kindString(k),
                                 "[",
                                 ";",
                                 "]",
                                 bs
                                 );
        else
            return PP::makeHVSeq(kindString(k) + "{" + id + "}",
                                 "[",
                                 ";",
                                 "]",
                                 bs
                                 );
    }
    }
}

TestFormatter::TestFormatter() {

    addOpInfo(EXISTS, 100, "*L");
    addOpInfo(FORALL, 100, "*L");
    addOpInfo(IMPLIES, 110, "LL");
    addOpInfo(IFF, 120, "LL");
    addOpInfo(OR,  130, "LLL");  // Ensure n-ary operators easily visible 
    addOpInfo(AND, 140, "LLL");
    addOpInfo(NOT, 150, "E");
    addOpInfo(TERM_IFF, 120, "LL");
    addOpInfo(TERM_OR,  130, "LL");
    addOpInfo(TERM_AND, 140, "LL");
    addOpInfo(TERM_NOT, 150, "E");

    addOpInfo(I_LE, 200, "LL");
    addOpInfo(I_LT, 200, "LL");
    
    addOpInfo(I_MINUS, 300, "LL");
    addOpInfo(I_PLUS, 300, "EL");
    addOpInfo(I_TIMES, 320, "EL");
    addOpInfo(IDIV, 320, "EL");
    addOpInfo(MOD, 340, "EL");
    addOpInfo(EXP, 350, "EL");
    addOpInfo(I_UMINUS, 360, "E");
    return;
};

Formatter* 
TestFormatter::instance = 0;

Formatter*
TestFormatter::getFormatter() {
    if (instance == 0) instance = new TestFormatter;
    return instance;
}


//==========================================================================
// PRINTING NODES 
//==========================================================================

void
Formatter::setFormatter(Formatter* f) {
    formatter = f;
}


std::ostream& 
operator<<(std::ostream& os, const Node& n) {

    os << Formatter::format(n);

    Box::deleteAll();

    return os;
}

std::string
Node::toString() {
    ostringstream oss;
    oss << *this;
    return oss.str();
}


// End of file
