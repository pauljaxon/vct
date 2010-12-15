//========================================================================
//========================================================================
// CONTEXT.HH
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


// Class for unit declarations.

/*
Context includes a notion of Feature.  Currently supported features are:

SI = standard interpretation

QFOL    Logic is quasi-first order.  Formulas are just terms of type Bool.
FOL     Logic is properly first order.  Formulas distinct from individual 
        type of Bits which replaces Bool type.  
        ( Predicates declarations are distinct from formula declarations.
          Hoping for this, but not yet implemented)

term_ITE     ITE operator supported in term language

Features in use:  A = added, Q = queried

AQ "prim bit type has int interp"
AQ "FOL"
A  "Error" 

Proposed features:

        ctxt->addFeature("prim bit type has int interp");

"Bit type has Bool interp"
"Bit type has Bool interp"
"Bit ops have std Bool interp"   (bit_and, bit_or, bit_not, bit_iff)
"Bit consts have std Bool interp" (bit_true, bit_false)
"Bit rels have std Bool interp"  (bit_eq[T] bit_le[real] bit_le[int])

Bit refinement changes these...
A
*/



#ifndef CONTEXT_HH
#define CONTEXT_HH

#include <set>
using std::set;

#include <map>
using std::map;
using std::make_pair;
using std::pair;

#include <vector>
using std::vector;

#include "node.hh"


using std::string;
using namespace z;


//============================================================================
// Context class
//============================================================================

/*
  Split declarations and definitions from FDL file into distinct maps for:

   o types
   o constants
   o enumeration constants from enumeration types
   o vars
   o functions.

  Maps are from strings to syntax trees for declarations and
  definitions.  Range for enumeration constant map is constant
  declarations that have the type field set to a type id using the
  declared name for the enumeration type.

  Generally, the assumption is made that there is no overlap between
  constant, enumeration constant and variable names and that a
  particular enumeration constant never gets used in more than one
  enumeration type.

  SPARK Examiner tool seems to add prefixes to names that probably help
  in ensuring this.

  No checks yet done to confirm this assumption.

*/

// Class is only partially abstract. 

// Have need to access copies of several components and access iterators.
// A hassle to provide these all via interface so just allow public member
// access.


class FDLContext {

    

  public:


    Node typeSeq;   // Mirrors typeMap, but maintains types in
                     // definition before use order

    Node termSeq; // All const, var, and fun declarations (not enum consts)
 
    map<string, Node*> typeMap;
    map<string, Node*> constMap;
    map<string, Node*> enumConstMap; // Constants introduced for enum types
    map<string, Node*> varMap;
    map<string, Node*> funMap;

    vector<z::Kind> pathKinds;
    vector<int> pathAddr;
    
    Node bindings;

    bool isFormula; // 
    bool isEmbeddedFormula; // 

    bool strictTyping;  // Affects behaviour of type inference
                        // If true, expect all polymorphism to be resolved
                        // so type of any operator immediately resolvable
                        // without consulting wider context.


    FDLContext(Node* FDL_AST);

    void insert(Node* decl, bool atEnd = true);

    void insertTypeAtStart(Node* decl) {
        insert(decl, false);
    }
    void insertTypeAtEnd(Node* decl) {
        insert(decl, true);
    }
    
    Node* lookupType(const string& s);
    Node* lookupConst(const string& s);
    Node* lookupVar(const string& s);
    Node* lookupFun(const string& s);
    Node* lookupEnumConst(const string& s);

    Node* normaliseType(Node* n);      // Expand all top-level type defs 
    Node* canoniseType(Node* n);      // Expand top-level type defs down to 
                                      // TYPE_ID, REAL_TY, INT_TY, BOOL_TY,
                                         // or BIT_TY

    void pushBinding(Node* decl); // Binding from FORALL or EXISTS expression.
    void popBinding();            // Pop most recently pushed.

    void pushPathStep(z::Kind k, int i); // Step from node kind k to child i
    void popPathStep();
    string getPathString();

    Node* lookupBinding(const string& s);

    Node* lookupId(const string& s);    // Id = bvar, const, var, enum const
    Node* lookupConstId(const string& s);  // Id = const, var, enum const

    Node* getType(Node* n);

    Node* getSubNodeTypes (Node* n);

    Node* extractDecls();

    set<string> features;

    void addFeature(const string& s);
    void removeFeature(const string& s);
    bool hasFeature(const string& s);

};


//============================================================================
// Template functions on contexts
//============================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// mapOverWithContext
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Expect typing for f:
//
//   Node* f(FDLContext* c, Node* n) ;
//
// Applies f in single bottom up pass over n, updating context with
// bindings when descending through expr arg of quantifiers.
// If f doesn't change node, it should just return it.

// - isEmbeddedFormulaCtxt coding is probably incomplete
// - 
// 
template<class BinFun> 
Node*
mapOverWithContextAux(BinFun& f, FDLContext* c, Node* n) {



    bool isFormulaCtxt = c->isFormula;
    bool isEmbeddedFormulaCtxt = c->isEmbeddedFormula;

    // [FORALL|EXISTS] (SEQ decl+) exp   where decl ::= DECL{id} type 

    if (n->kind == FORALL || n->kind == EXISTS) {

        c->isFormula = false;
        c->pushPathStep(n->kind, 0);
        n->child(0) = mapOverWithContextAux(f, c, n->child(0));
        c->popPathStep();
        c->isFormula = isFormulaCtxt;

        for ( int i = 0; 
              i != n->child(0)->arity();
              i++) {

            Node* decl = n->child(0)->child(i);
            c->pushBinding(decl);
        }

        c->pushPathStep(n->kind, 1);
        n->child(1) = mapOverWithContextAux(f, c, n->child(1));
        c->popPathStep();

        for ( int i = 0; 
              i != n->child(0)->arity();
              i++) {

            c->popBinding();
        }

    } else {

        if (! isCompoundProp(n)) c->isFormula = false;

        for (int i = 0; i != n->arity(); i++) {

            c->pushPathStep(n->kind, i);
            n->child(i) = mapOverWithContextAux(f, c, n->child(i));
            c->popPathStep();
        }
    }
    // Restore formula status of n.

    c->isFormula = isFormulaCtxt;
    c->isEmbeddedFormula = isEmbeddedFormulaCtxt;

    return f(c,n);


}


template<class BinFun> 
Node*
mapOverWithContext(BinFun& f, FDLContext* c, Node* n) {

    //    if (n->kind == UNIT || n->kind == RULES || n->kind == GOALS) {
    //        c->isFormula = true;
    //    }

    c->isFormula = true;

    return mapOverWithContextAux(f, c, n);
}




#endif // ! defined CONTEXT_HH
