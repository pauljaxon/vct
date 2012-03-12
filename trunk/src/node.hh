//==========================================================================
//==========================================================================
// NODE.HH
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



#ifndef NODE_HH
#define NODE_HH

#include <string>    /* C++ strings */
#include <vector>    /* C++ dynamic arrays */

#include <set>

#include <cassert>

/* Definitions of node kinds for use in AST */

/* These are put in a separate namespace to avoid clashes with
   tokens of the same name
*/

namespace z {

enum Kind {
    // Top level
    FDL_FILE,
    DEF_TYPE,
    DECL_TYPE,  // Isabelle, SMTLIB2
    DEF_RECORD, // Isabelle
    OUTER_DECL, // Isabelle
    DEF_CONST,
    DECL_CONST, // Isabelle, SMTLIB2
    DECL_VAR,
    DECL_FUN,
    RULE_FILE,
    RULE_FAMILY,
    REQUIRES,
    MAY_BE_REPLACED_BY,
    MAY_BE_DEDUCED,
    ARE_INTERCHANGEABLE,
    VCG_FILE,
    GOAL,
    ASSERT,  // Yices & SMTLib2
    CHECK,
    GET_UNSAT_CORE,
    PUSH,
    POP,
    LABEL,
    COMMENT,
    SCRIPT,
    DECLS,  // Introduced by processing
    RULES,
    RULE,
    GOALS,
    HYPS,
    ASSUMPTION, // Isabelle, SMTLib
    CONCLS,
    CONCL,  // Isabelle
    UNIT,
    THEORY, // Isabelle

    // Types
    INT_TY,
    REAL_TY,
    BOOL_TY,
    BIT_TY,
    BITVEC_TY,
    SUBRANGE_TY,
    ENUM_TY,
    ARRAY_TY,
    RECORD_TY,
    FUN_TY,     // Yices, HOLs
    FUN_ARG_TY, // Isabelle/HOL
    TUPLE_TY,
    UNKNOWN,
    TYPE_UNIV,
    INT_OR_REAL_TY,    // Int or Real
    INT_REAL_OR_ENUM_TY, // Int or Real or Enum
    NO_TY,    // Set of no types

    // Expressions
    FORALL,
    EXISTS,
    PAT,
    IMPLIES,
    IFF,
    AND,
    OR,
    NOT,
    EQ,
    NE,
    LT,
    GT,
    LE,
    I_LT,
    I_LE,
    R_LT,
    R_LE,
    GE,
    TO_REAL,
    UMINUS,
    SUCC,
    PRED,
    PLUS,
    MINUS,
    TIMES,
    LIN_TIMES,  // Linear times:  k * e
    NL_TIMES, // Nonlinear times
    I_UMINUS,
    I_SUCC,
    I_PRED,
    I_PLUS,
    I_MINUS,
    I_TIMES,
    I_LIN_TIMES,  // Linear times:  k * e
    I_NL_TIMES, // Nonlinear times
    R_UMINUS,
    R_PLUS,
    R_MINUS,
    R_TIMES,
    R_LIN_TIMES,  // Linear times:  k * e
    R_NL_TIMES, // Nonlinear times
    RDIV,
    IDIV,
    IDIV_E,  // Euclidian IDIV
    IDIVM, // IDIV compatible with MOD
    MOD,
    MOD_E,   // Euclidian MOD
    EXP,
    ABS,
    I_ABS,
    R_ABS,
    SQR,
    I_SQR,
    R_SQR,
    I_EXP,
    R_EXP,
    I_EXP_N, // Isabelle/HOL
    R_EXP_N, // Isabelle/HOL
    I_TO_NAT, // Isabelle/HOL
    ODD,
    TUPLE,
    ARR_ELEMENT,
    ARR_UPDATE,
    ARR_BOX_UPDATE,
    MK_ARRAY,
    RCD_ELEMENT,
    RCD_UPDATE,
    MK_RECORD,
    ASSIGN,
    INDEX_AND,
    FUN_AP,
    SUBRANGE,
    ID,
    TYPE_ID,
    TYPE_PARAM,
    NATNUM,
    REALNUM,
    TRUE,
    FALSE,

    TERM_TRUE,
    TERM_FALSE,
    TERM_AND,
    TERM_OR,
    TERM_IFF,
    TERM_NOT,
    TERM_EQ,
    TERM_NE,
    TERM_I_LT,
    TERM_I_LE,

    TO_PROP,  // Bit to prop
    TO_BIT,   // prop to bit

    APPLY,   // Yices, HOLs
    LAMBDA,
    MK_TUPLE,
    UPDATE,    // for arrays or functions
    SELECT,

    // Builders, misc
    SEQ,
    DECL,
    PENDING,

    // SMTLIB specific
    BENCHMARK,
    LOGIC,
    EXTRASORTS,
    EXTRAFUNS,
    EXTRAPREDS,
    FORMULA,
    STATUS,
    DECL_PRED,
    TCONST,
    PRED_AP,
    DISTINCT,
    ITE,
    CONST,
    VAR,
    // SMTLIB2 specific
    SET_OPTION,
    TO_INT,
    IS_INT,

    SET_INFO,
    INFO_STR
};


std::string kindString(z::Kind k);

} // End namespace z


class Node;

typedef std::vector< Node* > Nodes;

// By default pointers to Node objects are stored in a pool, and all
// current objects in the pool can be deleted at once by calling the
// deletePool() method.

// When node objects are statically or heap allocated, or
// are intended to outlast pool deletions, they should be allocated outside
// the pool by setting the optional storage argument to constructors to
// UNMANAGED

// (Was using bool type, but got nasty type bugs because Node* args were
// getting coerced to bool. Now using enum type.)

enum Storage { MANAGED, UNMANAGED };

class Node {

  // Memory management fields and methods
  private:

    static std::vector<Node*> pool;

    void addToPool() {
        pool.push_back(this);
        return;
    }

 public:

    static void deletePool();
    static int getPoolAllocCount() {return pool.size();}





    // Instance fields

    z::Kind kind;
    std::string id;
    Nodes children;

    // Constructors

    explicit Node(z::Kind k, Storage st = MANAGED) ;

    Node(z::Kind k, Node* n1, Storage st = MANAGED) ;
    Node(z::Kind k, Node* n1, Node* n2, Storage st = MANAGED) ;
    Node(z::Kind k, Nodes& ns, Storage st = MANAGED) ;


    // Constructors with char* needed to avoid compiler wrongly
    // inserting conversions.

    Node(z::Kind k, const std::string& s, Storage st = MANAGED) ;
    Node(z::Kind k, const char* s,        Storage st = MANAGED) ;

    Node(z::Kind k, const std::string& s, Node* n1, Storage st = MANAGED) ;
    Node(z::Kind k, const char* s,        Node* n1, Storage st = MANAGED) ;
    Node(z::Kind k,
         const std::string& s,
         Node* n1,
         Node* n2,
         Storage st = MANAGED) ;
    Node(z::Kind k,
         const char* s,
         Node* n1,
         Node* n2,
         Storage st = MANAGED) ;
    Node(z::Kind k,
         const std::string& s,
         Node* n1,
         Node* n2,
         Node* n3,
         Storage st = MANAGED) ;
    Node(z::Kind k,
         const char* s,
         Node* n1,
         Node* n2,
         Node* n3,
         Storage st = MANAGED) ;
    Node(z::Kind k,
         const std::string& s,
         Node* n1,
         Node* n2,
         Node* n3,
         Node* n4,
         Storage st = MANAGED) ;
    Node(z::Kind k,
         const char* s,
         Node* n1,
         Node* n2,
         Node* n3,
         Node* n4,
         Storage st = MANAGED) ;
    Node(z::Kind k, const std::string& s, Nodes& ns, Storage st = MANAGED) ;
    Node(z::Kind k, const char* s, Nodes& ns, Storage st = MANAGED) ;

    Node*& child(int i) {
        return children.at(i);
    }

    Node*& lastChild() {
        return children.at(children.size() - 1);
    }


    // This definition needed for use in correctly typed def of ==.
    Node* const & child(int i) const {
        return children.at(i);
    }

    // Add n as new (rightmost) child
    void addChild(Node* n) { children.push_back(n); }

    void addLeftChild(Node* n) {
        children.insert(children.begin(), n);
    }

    void popChild() { children.pop_back(); }
    void popLeftChild() { children.erase(children.begin()); }
    void clearChildren() { children.clear(); }

    // Add all children of n onto right of this's children.
    void appendChildren(Node* n) {
        children.insert(children.end(),
                        n->children.begin(),
                        n->children.end());
    }

    int arity() const {return children.size(); }

    int treeSize() const;

    Node* updateKind(z::Kind k) {
        kind = k;
        return this;
    }

    Node* updateId(const std::string& s) {
        id = s;
        return this;
    }

    Node* updateKindAndId(z::Kind k, const std::string& s) {
        kind = k;
        id = s;
        return this;
    }

    bool operator==(const Node& n) const;

    bool equals(Node* n) { return * this == * n;}

    std::string toString();

    std::string toShortString();

    std::set<std::string> getIds(z::Kind k);

    // Deep copy of node tree.

    Node* copy();

    // Map f over Node tree in single bottom up pass.
    // f can be pointer to function or unary function object.

    template<class UnaryFun> void mapOver(UnaryFun& f);
    template<class UnaryFun> Node* mapOver1(UnaryFun& f);

    // Compute Or of result of f applied to each node of Node tree.

    // template<typename UnaryBoolFun> bool orOver(UnaryBoolFun& f);
    template<typename UnaryBoolFun> bool orOver(UnaryBoolFun f);

    // Get `logical' children.  Usually are the same as children,
    // except for operators built using more than one node.

    std::vector<Node**> getSubNodes();

    //
    Node* expandSubranges() {
        if (kind == z::SUBRANGE_TY)
            return Node::int_ty;
        else
            return this;
    }
    // Some common atomic nodes.

    static Node* int_ty;
    static Node* bool_ty;
    static Node* bit_ty;
    static Node* real_ty;
    static Node* int_or_real_ty;
    static Node* int_real_or_enum_ty;
    static Node* type_univ;
    static Node* unknown;
    static Node* no_ty;


};


// g++ doesn't support the "export" keyword which allows
// this definition to be pushed down into node.cc.

template<class UnaryFun>
void
Node::mapOver(UnaryFun& f) {

    for (int i = 0; i != arity(); i++) {
        child(i)->mapOver(f);
    }
    f(this);
}

// Variation on mapOver, if want to destructively modify
// node tree structure.

template<class UnaryFun>
Node*
Node::mapOver1(UnaryFun& f) {

    for (int i = 0; i != arity(); i++) {
        child(i) = child(i)->mapOver1(f);
    }
    return f(this);
}


// A function/function object adaptor.
// Permits application of bool-valued operator a number of times.
// reports if exist application that returns true.

// UnaryBoolFun expected to be either
// 1. Type of functions T->bool
// 2. A class which overloads operator() with function returning bool on arg
//    of type T.

// Version with references is not compiling properly.  Not sure why.

template<typename T, typename UnaryBoolFun>
class IteratedOr {
private:
    bool result;
    //    UnaryBoolFun& fun;
    UnaryBoolFun fun;

public:
    //    IteratedOr(UnaryBoolFun& f) {
    IteratedOr(UnaryBoolFun f) {
        result = false;
        fun = f;
    }

    void operator() (T t) {
        if ( fun(t) ) result = true;
        return;
    }

    bool value() {return result;}

};

template<typename UnaryBoolFun>
bool
Node::orOver(UnaryBoolFun f) {

    IteratedOr<Node*, UnaryBoolFun> itOr(f);
    mapOver(itOr);
    return itOr.value();

    // return f(this);
}



// Used in both yices.cc and cvc.cc.  But perhaps here is too generic a place.
//========================================================================
// Node tree analysis.
//========================================================================

bool isDivOrMod(Node* n);

bool isAtomicProp(Node* n);
bool isCompoundProp(Node* n);

bool isProp(Node* n);  // Is a propositional node.  Counts top level
                       // constructors UNIT, RULES and GOALS as propositional
                       // (effectively ANDs)


std::string gatherKinds(Node* n);

std::set<std::string> gatherBoundVars(Node* n);

// Expects hyps to be AND(h1 ... hk) where k >= 0.
// If k = 0   returns  concl
// If k = 1   return   h1 => concl
// If k > 1   return   AND(h1 ... hk) => concl

Node* mkGeneralImplies(Node* hyps, Node* concl);


Node* nameToType(const std::string& s);
std::string typeToName(Node* n);

bool isPolymorphicNode(Node* n);

//========================================================================
// Node constructor Macros
//========================================================================

#define nRULE(id,f) new Node(RULE,(id),(f))

#define nDECL(id,T) new Node(DECL,(id),(T))
#define nSEQ1(x) new Node(SEQ,(x))
#define nSEQ2(x,y) new Node(SEQ,(x),(y))
#define nSEQ3(x,y,z) new Node(SEQ,"",(x),(y),(z))

#define nFUNAP1(f,a1) new Node(FUN_AP,(f),(a1))
#define nFUNAP2(f,a1,a2) new Node(FUN_AP,(f),(a1),(a2))
#define nFUNAP3(f,a1,a2,a3) new Node(FUN_AP,(f),(a1),(a2))

#define nNATNUM(n) new Node(NATNUM,(n))
#define nCONST(c) new Node(CONST,(c))
#define nVAR(v) new Node(VAR,(v))

#define nINT_TY new Node(INT_TY)
#define nTYPE_ID(id) new Node(TYPE_ID,(id))

#define nFORALL(ds,p) new Node(FORALL, (ds),(p))
#define nFORALL1(v,T,p) nFORALL(nSEQ1(nDECL((v),(T))), (p))
#define nFORALL2(v1,T1,v2,T2,p) \
    nFORALL( nSEQ2( nDECL((v1),(T1)), nDECL((v2),(T2))), (p))
#define nIMPLIES(p,q) new Node(IMPLIES,(p),(q))
#define nNOT(p) new Node(NOT,(p))
#define nAND(p,q) new Node(AND,(p),(q))
#define nIFF(p,q) new Node(IFF,(p),(q))
#define nTRUE new Node(TRUE);
#define nFALSE new Node(FALSE);

#define nI_SUCC(i) new Node(I_SUCC,(i))
#define nI_PRED(i) new Node(I_PRED,(i))
#define nI_PLUS(i,j) new Node(I_PLUS,(i),(j))
#define nI_MINUS(i,j) new Node(I_MINUS,(i),(j))
#define nIDIV(i,j) new Node(IDIV,(i),(j))
#define nIDIV_E(i,j) new Node(IDIV_E,(i),(j))
#define nMOD(i,j) new Node(MOD,(i),(j))
#define nMOD_E(i,j) new Node(MOD_E,(i),(j))
#define nI_LE(i,j) new Node(I_LE,(i),(j))
#define nI_LT(i,j) new Node(I_LT,(i),(j))
#define nI_GE(i,j) new Node(I_LE,(j),(i))
#define nI_GT(i,j) new Node(I_LT,(j),(i))

#define nEQ(a,b,T) new Node(EQ,"",(a),(b),(T))
#define nI_EQ(a,b) new Node(EQ,"",(a),(b),nINT_TY)
#define nNE(a,b,T) nNOT(nEQ((a),(b),(T)))




#endif // ! NODE_HH
