//========================================================================
//========================================================================
// TRANSLATION.CC
//========================================================================
//========================================================================

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



//========================================================================
/*

Relevant command-line options
=============================

[] for legacy
*  for new one, not yet supported

*/

//--------------------------------------------------------------------------
// C++ headers
//--------------------------------------------------------------------------
#include <cassert>


#include <map>
using std::map;
using std::pair;

//--------------------------------------------------------------------------
// My header files
//--------------------------------------------------------------------------

#include "translation.hh"
#include "utility.hh"

//--------------------------------------------------------------------------
// Major translation components
//--------------------------------------------------------------------------

#include "refine-types.cc"
#include "qfol-to-fol.cc"
#include "elim-bit-type.cc"
#include "elim-enums.cc"
#include "elim-arrays-records.cc"

//============================================================================
//============================================================================
// Typechecking
//============================================================================
//============================================================================
// `Type-checking' here does both syntactic checks and type checks.
// If there are problems with the syntax, then, to avoid clutter, the
// type error messages are suppressed.

class nodeTypeChecker {
public:
    string syntaxMessages;
    string tCMessages;

    void addSyntaxMessage(const string& m);
    void addTCMessage(const string& m);
    Node* operator() (FDLContext* c, Node* n);
};


void
nodeTypeChecker::addSyntaxMessage(const string& m) {
    if (syntaxMessages.size() > 0) syntaxMessages += ENDLs;
    syntaxMessages += m;
    return;
}

void
nodeTypeChecker::addTCMessage(const string& m) {
    if (tCMessages.size() > 0) tCMessages += ENDLs;
    tCMessages += m;
    return;
}

Node*
nodeTypeChecker::operator() (FDLContext* c, Node* n) {

    // Don't attempt to check interior nodes

    if (n->kind == SEQ
        || n->kind == ASSIGN
        || n->kind == SUBRANGE
        || n->kind == DECL) {
        return n;
    }
    // Flag polymorphic nodes

    if (isPolymorphicNode(n)) {
        addSyntaxMessage(  "Found unexpected polymorphic node "
                           + n->toShortString() + ENDLs +
                           "  at " + c->getPathString() + ENDLs 
                        );
        return n;  
    }
    // Flag unbound identifiers
    if (n->kind == ID) {
        addSyntaxMessage(  "Found unexpected "
                           + n->toShortString() + ENDLs +
                           "  at " + c->getPathString() + ENDLs 
                        );
        return n;
    }
    else if (n->kind == VAR && c->lookupBinding(n->id) == 0) {
        addSyntaxMessage(  "Found unbound "
                     + n->toShortString() + ENDLs +
                     "  at " + c->getPathString() + ENDLs 
                     );
        return n;
    }
    else if (n->kind == CONST
             && c->lookupVar(n->id) == 0
             && c->lookupConst(n->id) == 0
             && c->lookupEnumConst(n->id) == 0) {
        addSyntaxMessage(  "Found undeclared "
                           + n->toShortString() + ENDLs +
                           "  at " + c->getPathString() + ENDLs 
                        );
        return n;
    }

    
    vector<Node**> subNodes = n->getSubNodes();
    Node* subNodeTypes = c->getSubNodeTypes(n);

    if (subNodeTypes->arity() != (int) subNodes.size()) {

        addTCMessage("For node " + n->toShortString() + ENDLs +
                     "  at " + c->getPathString() + ENDLs + 
                     "Have " + intToString((int) subNodes.size())
                     + " subnodes and "
                     + intToString(subNodeTypes->arity()) + " subnode types"
                     + ENDLs
                     );
        return n;  // No point trying to continue typechecking of subnodes
    }


    for (int i = 0; i != (int) subNodes.size(); i++) {
        Node** subNode = subNodes.at(i);
        Node* actualType = c->normaliseType(c->getType(*subNode));
        Node* expectedType = c->normaliseType(subNodeTypes->child(i));

        printMessage(FINESTm,
                     "Typechecking child " + intToString(i)
                     + " of node " + n->toShortString() + ENDLs
                     + "Expected type:" 
                     + expectedType->toShortString() + ENDLs
                     + "Actual type:" + ENDLs
                     + actualType->toShortString());
                     
        if (c->hasFeature("prim bit type has int interp")) {
            if (actualType->kind == BIT_TY) {
                actualType = actualType->copy()->updateKind(INT_TY);
            }
            if (expectedType->kind == BIT_TY) {
                expectedType = expectedType->copy()->updateKind(INT_TY);
            }
        }

        // Pickout good cases one by one.

        if (actualType->kind != UNKNOWN && actualType->equals(expectedType)) {
            continue;

        }

        if (actualType->kind == SUBRANGE_TY) {

            if (expectedType->kind == INT_TY) continue;

            if (expectedType->kind == SUBRANGE_TY
                && actualType->child(0)->kind == NATNUM
                && actualType->child(1)->kind == NATNUM
                && expectedType->child(0)->kind == NATNUM
                && expectedType->child(1)->kind == NATNUM
                ) {
                int actualL = stringToInt(actualType->child(0)->id);
                int actualH = stringToInt(actualType->child(1)->id);
                int expectedL = stringToInt(expectedType->child(0)->id);
                int expectedH = stringToInt(expectedType->child(1)->id);

                if (expectedL <= actualL && actualH <= expectedH) continue;
            }
        }
        printMessage(FINESTm,
                     "type check failed of sub node " + (intToString(i)) 
                     + " of " + ENDLs
                     + n->toString() + ENDLs
                     + "Expected type:" + ENDLs
                     + expectedType->toString() + ENDLs
                     + "Actual type:" + ENDLs
                     + actualType->toString()
                     );
        addTCMessage("For node " + (*subNode)->toShortString() + ENDLs
                     + "  at " + c->getPathString()
                     + kindString(n->kind) + "."
                     + intToString(i) + "." + ENDLs +
                     + "Expected type:" 
                     + expectedType->toShortString() + ENDLs
                     + "Actual type:" 
                     + actualType->toShortString() + ENDLs
                     );
        /*
        addMessage(  "type check failed of "
                     + (*subNode)->toShortString() + ENDLs
                     + "which is sub node " + (intToString(i)) 
                     + " of " + n->toShortString() + ENDLs
                     + "Expected type:" 
                     + expectedType->toShortString() + ENDLs
                     + "Actual type:" 
                     + actualType->toShortString()
                     );
        */

    } // END for loop over children
    return n;
}


string typeCheck(FDLContext* c, Node* node, Node* expectedType) {
   
    nodeTypeChecker nTC;
    mapOverWithContext(nTC, c, node);

    Node* actualType = c->getType(node);
    if (! actualType->equals(expectedType)) {
        nTC.addTCMessage("At top node "
                         + node->toShortString() + ENDLs
                         + "Expected type:" 
                         + expectedType->toShortString() + ENDLs
                         + "Actual type:" 
                         + actualType->toShortString() + ENDLs
                        );
    }
    if (nTC.syntaxMessages.size() > 0)
        return "Possible FDL issue" + ENDLs + nTC.syntaxMessages;
    else
        return nTC.tCMessages;
}

// Return true if type check succeeds


bool typeCheckFmla(int reportingLevel, const string& fmlaName, FDLContext* c, Node* node) {

    c->strictTyping = true;

    string messages = typeCheck(c,node,new Node(BOOL_TY));

    if (messages.size() == 0) {
        return true;
    }
    else {
        printMessage
            (reportingLevel,
             "Type check failed for " + fmlaName + ":" + ENDLs
             + node->toString() + ENDLs + ENDLs
             + messages + ENDLs
             );
        return false;
    }
}

bool typeCheckUnit(const string& tcKind,
                   UnitInfo* unitInfo,
                   FDLContext* ctxt,
                   Node* unit) {

    Node* rules = unit->child(1);
    Node* goals = unit->child(2);

    bool typeCheckGood = true;

    for ( int ruleNum = 1;
          ruleNum <= rules->arity();
          ruleNum++) {

        if (unitInfo->isExcludedRule(ruleNum - 1)) continue;
        
        string ruleStr (tcKind + ", Rule " + intToString(ruleNum));
        Node* rule = rules->child(ruleNum - 1);

        if (option("delete-rules-failing-typecheck")) {

            if (!typeCheckFmla(WARNINGm, ruleStr, ctxt, rule)) {

                unitInfo->addExcludedRule(ruleNum - 1);
                rule->child(0) = nTRUE;

                string ruleName = rule->id;
                string ruleKind =
                    unitInfo->isUserRule(ruleNum - 1)
                    ? "user"
                    : "Examiner";

                printMessage(WARNINGm,
                         "Deleting " + ruleKind + " rule " + ruleName
                             + " because it fails type check");
            }
        }
        else {
            typeCheckGood =
                typeCheckGood && typeCheckFmla(ERRORm, ruleStr, ctxt, rule);
        }
    }
    
    for (int goalNum = 1; goalNum <= goals->arity(); goalNum++) {
        
        Node* goal = goals->child(goalNum-1);

        if (goal->arity() < 2) { // "*** true" goals
            continue;
        }

        string goalStr (tcKind + ", Goal " + intToString(goalNum));

        Node* hyps = goal->child(0);
        Node* concls = goal->child(1);
        
        for ( int hypNum = 1;
              hypNum <= hyps->arity();
              hypNum++) {

            Node* hyp = hyps->child(hypNum-1);

            string hypStr (goalStr + ", Hyp " + intToString(hypNum));

            typeCheckGood = typeCheckGood &
                typeCheckFmla(ERRORm, hypStr, ctxt, hyp);
            
        }

        for ( int conclNum = 1;
              conclNum <= concls->arity();
              conclNum++) {

            Node* concl = concls->child(conclNum-1);

            string conclStr (goalStr + ", Concl " + intToString(conclNum));

            typeCheckGood = typeCheckGood &
                typeCheckFmla(ERRORm, conclStr, ctxt, concl);
            
        }
    } // END for goalNum = ...

    return typeCheckGood;

}

//========================================================================
//========================================================================
// Miscellaneous
//========================================================================
//========================================================================


//------------------------------------------------------------------------
// Strip quantifier patterns
//------------------------------------------------------------------------
void
stripQuantPat(Node* n) {

    if ((n->kind == FORALL || n->kind == EXISTS) && n->arity() == 3) {
        n->popChild();
    }
}

void
stripQuantPats(Node* unit) {
    unit->mapOver(stripQuantPat);
}

//------------------------------------------------------------------------
// Normalise all type expressions
//------------------------------------------------------------------------
// Expand any type aliases.

// Can ignore TYPE_PARAMs since they already are normalised.

Node* expandTypeAlias(FDLContext* ctxt, Node* n) {
    if (n->kind != TYPE_ID) return n;

    Node* normType = ctxt->normaliseType(n);

    if (normType->kind == RECORD_TY
        || normType->kind == ENUM_TY
        || normType->kind == ARRAY_TY ) {

        return new Node(TYPE_ID, normType->id);
    } else {
        // Expect here normType->kind = BOOL_TY, INT_TY, REAL_TY or BIT_TY,
        // SUBRANGE_TY, TYPE_ID 

        return normType->copy();
    }
}

void elimTypeAliases(FDLContext* ctxt, Node* unit) {

    mapOverWithContext(expandTypeAlias, ctxt, &(ctxt->typeSeq));
    mapOverWithContext(expandTypeAlias, ctxt, &(ctxt->termSeq));
    mapOverWithContext(expandTypeAlias, ctxt, unit);

    // Turn all alias type definitions into abstract type declarations.

    // Eventually could do work of completely removing the unused
    // alias type declarations.

    for (int i = 0; i != ctxt->typeSeq.arity(); i++) {
        Node* typeDecl = ctxt->typeSeq.child(i);
        if (typeDecl->arity() == 1) {
            Node* declRHS = typeDecl->child(0);
            Kind k = declRHS->kind;
            if (! (k == RECORD_TY || k == ENUM_TY || k == ARRAY_TY) ) {
                typeDecl->popChild();
            }
        }
    }
    return;
}
