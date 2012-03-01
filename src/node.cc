//==========================================================================
//==========================================================================
// NODE.CC
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


#include "node.hh"

using namespace z;  // For Kind type, kindString function

using std::set;

#include "pprinter.hh"

// For printMessage.
#include "utility.hh"


// Tedious having to name Kinds twice.  What's best way of avoiding this?
std::string
z::kindString(z::Kind k) {
    std::string s;

    switch (k) {
        #define c(id) case id: s = #id ; break
        c(FDL_FILE);
	c(DEF_TYPE);
	c(DECL_TYPE); // Isabelle
	c(DEF_RECORD); // Isabelle
	c(OUTER_DECL); // Isabelle
	c(DEF_CONST);
	c(DECL_CONST); // Isabelle
	c(DECL_VAR);
	c(DECL_FUN);
	c(RULE_FILE);
	c(RULE_FAMILY);
	c(REQUIRES);
	c(MAY_BE_REPLACED_BY);
	c(MAY_BE_DEDUCED);
	c(ARE_INTERCHANGEABLE);
	c(VCG_FILE);
	c(GOAL);
	c(ASSERT);  // Yices extra
	c(CHECK);
	c(PUSH);
	c(POP);
	c(SCRIPT);
        c(DECLS);  // Introduced by processing
        c(RULES);
        c(RULE);
        c(GOALS);
        c(HYPS);
        c(ASSUMPTION);
        c(CONCLS);
        c(CONCL);  // Isabelle
        c(UNIT);
        c(THEORY); // Isabelle

    // Types
	c(INT_TY);
	c(REAL_TY);
	c(BOOL_TY);
	c(BIT_TY);
	c(BITVEC_TY);
	c(SUBRANGE_TY);
	c(ENUM_TY);
	c(ARRAY_TY);
	c(RECORD_TY);
        c(FUN_TY);     // Yices, HOLs
        c(FUN_ARG_TY); // Isabelle/HOL
        c(TUPLE_TY);
        c(UNKNOWN);    // Used when no constraints known on a type
        c(TYPE_UNIV);  // Type of types
        c(INT_OR_REAL_TY);    // Int or Real
        c(INT_REAL_OR_ENUM_TY); // Int or Real or Enum
        c(NO_TY);  // Set of no types.

    // Expressions
	c(FORALL);
	c(EXISTS);
        c(PAT);
	c(IMPLIES);
	c(IFF);
	c(AND);
	c(OR);
	c(NOT);
	c(EQ);
	c(NE);
	c(LT);
	c(GT);
	c(LE);
	c(I_LT);
	c(I_LE);
	c(R_LT);
	c(R_LE);
	c(GE);
	c(TO_REAL);
	c(UMINUS);
	c(SUCC);
	c(PRED);
	c(PLUS);
	c(MINUS);
	c(TIMES);
	c(LIN_TIMES);
	c(NL_TIMES);
	c(I_UMINUS);
	c(I_SUCC);
	c(I_PRED);
	c(I_PLUS);
	c(I_MINUS);
	c(I_TIMES);
	c(I_LIN_TIMES);
	c(I_NL_TIMES);
	c(R_UMINUS);
	c(R_PLUS);
	c(R_MINUS);
	c(R_TIMES);
	c(R_LIN_TIMES);
	c(R_NL_TIMES);
	c(RDIV);
	c(IDIV);
	c(IDIV_E);  // Euclidean IDIV
	c(IDIVM); // IDIV compatible with MOD
	c(MOD);
	c(MOD_E);  // Euclidean MOD
	c(EXP);
	c(I_EXP);
	c(R_EXP);
	c(I_EXP_N);// Isabelle/HOL
	c(R_EXP_N);// Isabelle/HOL
	c(I_TO_NAT);// Isabelle/HOL
        c(ODD);
	c(ABS);
	c(I_ABS);
	c(R_ABS);
	c(SQR);
	c(I_SQR);
	c(R_SQR);
	c(TUPLE);
	c(ARR_ELEMENT);
	c(ARR_UPDATE);
	c(ARR_BOX_UPDATE);
	c(MK_ARRAY);
	c(RCD_ELEMENT);
	c(RCD_UPDATE);
	c(MK_RECORD);
        c(ASSIGN);
        c(INDEX_AND);
	c(FUN_AP);
	c(SUBRANGE);
	c(ID);
	c(TYPE_ID);
	c(TYPE_PARAM);
	c(NATNUM);
	c(REALNUM);
	c(TRUE);
	c(FALSE);

	c(TERM_TRUE);
	c(TERM_FALSE);
	c(TERM_AND);
	c(TERM_OR);
	c(TERM_IFF);
	c(TERM_NOT);
	c(TERM_EQ);
	c(TERM_NE);
	c(TERM_I_LT);
	c(TERM_I_LE);

        c(TO_PROP);
        c(TO_BIT);   // prop to bit

	c(APPLY);
	c(LAMBDA);
	c(MK_TUPLE);
	c(UPDATE);
	c(SELECT);
    // Builders, misc
	c(SEQ);
	c(DECL);
	c(PENDING);
    // SMTLIB
        c(BENCHMARK);
        c(LOGIC);
        c(EXTRASORTS);
        c(EXTRAFUNS);
        c(EXTRAPREDS);
        c(FORMULA);
        c(STATUS);
        c(DECL_PRED);
        c(TCONST);
        c(PRED_AP);
        c(DISTINCT);
        c(ITE);
        c(CONST);
        c(VAR);
    // SMTLIB2
        c(SET_OPTION);
        c(TO_INT);
        c(IS_INT);

	c(SET_INFO);
	c(INFO_STR);

        #undef c
    }
    return s;

}

// Memory management definitions.

Nodes Node::pool;

void
Node::deletePool() {
    for (  vector<Node*>::iterator i = pool.begin();
           i != pool.end();
           i++) {
        delete (*i);
    }
    pool.clear();
    return;
}

Node::Node(z::Kind k, Storage st) {
    kind = k;
    if (st == MANAGED) addToPool();
}

/*
Node::Node(Node* n1, Storage st) {
    kind = SEQ;
    addChild(n1);
    if (st == MANAGED) addToPool();
}
*/

Node::Node(Kind k, Node* n1, Storage st) {
    kind = k;
    addChild(n1);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, Node* n1, Node* n2, Storage st) {
    kind = k;
    addChild(n1);
    addChild(n2);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, Nodes& ns, Storage st) {
    kind = k;
    children = ns;
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const std::string& s, Storage st) {
    kind = k;
    id = s;
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const char* s, Storage st) {
    kind = k;
    id = string(s);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const std::string& s, Node* n1, Storage st) {
    kind = k;
    id = s;
    addChild(n1);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const char* s, Node* n1, Storage st) {
    kind = k;
    id = string(s);
    addChild(n1);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const std::string& s, Node* n1, Node* n2, Storage st) {
    kind = k;
    id = s;
    addChild(n1);
    addChild(n2);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const char* s, Node* n1, Node* n2, Storage st) {
    kind = k;
    id = string(s);
    addChild(n1);
    addChild(n2);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const std::string& s, Node* n1, Node* n2, Node* n3,
           Storage st) {
    kind = k;
    id = s;
    addChild(n1);
    addChild(n2);
    addChild(n3);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const char* s, Node* n1, Node* n2, Node* n3, Storage st) {
    kind = k;
    id = string(s);
    addChild(n1);
    addChild(n2);
    addChild(n3);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const std::string& s, Node* n1, Node* n2, Node* n3,
           Node* n4, Storage st) {
    kind = k;
    id = s;
    addChild(n1);
    addChild(n2);
    addChild(n3);
    addChild(n4);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const char* s, Node* n1, Node* n2, Node* n3,
           Node* n4, Storage st) {
    kind = k;
    id = string(s);
    addChild(n1);
    addChild(n2);
    addChild(n3);
    addChild(n4);
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const std::string& s, Nodes& ns, Storage st) {
    kind = k;
    id = s;
    children = ns;
    if (st == MANAGED) addToPool();
}

Node::Node(Kind k, const char* s, Nodes& ns, Storage st) {
    kind = k;
    id = string(s);
    children = ns;
    if (st == MANAGED) addToPool();
}


int
Node::treeSize() const {
    int size = 1;
    for (int i = 0; i != arity(); i++) {
        size += child(i)->treeSize();
    }
    return size;
}


bool
Node::operator==(const Node& n) const {
    if (kind == n.kind
        && id == n.id
        && arity() == n.arity()) {

        for (int i = 0; i != arity(); i++) {
            if ( ! ( * child(i) == * (n.child(i))
                    )
                 )
                return false;
        }
        return true;
    }
    return false;
}



std::set<string>
Node::getIds(z::Kind k) {
    std::set<string> result;
    if (kind == k && id.size() > 0)
        result.insert(id);

    for (int i = 0; i != arity(); i++) {

        std::set<string> childIds = child(i)->getIds(k);
        result.insert(childIds.begin(), childIds.end());
    }
    return result;
}


std::string
Node::toShortString() {

    return kindString(kind) + "{" + id + "}[" + intToString(arity()) + "]";
}


Node*
Node::copy() {

    Node* result = new Node(kind, id);
    for (int i = 0; i != arity(); i++) {
        result->addChild(child(i)->copy());
    }
    return result;
}



Node*
Node::int_ty = new Node (z::INT_TY, UNMANAGED);
Node*
Node::bool_ty = new Node (z::BOOL_TY, UNMANAGED);
Node*
Node::bit_ty = new Node (z::BIT_TY, UNMANAGED);
Node*
Node::real_ty = new Node (z::REAL_TY, UNMANAGED);
Node*
Node::type_univ = new Node (z::TYPE_UNIV, UNMANAGED);
Node*
Node::unknown = new Node (z::UNKNOWN, UNMANAGED);
Node*
Node::int_or_real_ty = new Node(z::INT_OR_REAL_TY, UNMANAGED);
Node*
Node::int_real_or_enum_ty = new Node(z::INT_REAL_OR_ENUM_TY, UNMANAGED);
Node*
Node::no_ty = new Node(z::NO_TY, UNMANAGED);

bool isDivOrMod(Node* n) {
    return
        n->kind == z::IDIV
        || n->kind == z::MOD;
}

//
bool isCompoundProp(Node* n) {
    int k = n->kind;
    return
        k == z::UNIT
        || k == z::RULES
        || k == z::RULE
        || k == z::GOALS
        || k == z::GOAL
        || k == z::HYPS
        || k == z::CONCLS
        || k == z::AND
        || k == z::OR
        || k == z::NOT
        || k == z::FORALL
        || k == z::EXISTS
        || k == z::IFF
        || k == z::IMPLIES;
}

// NB: isAtomicProp() is not complete.  It will flag nodes that
// are definitely atomic propositions, but will miss those which might
// or might not be: e.g. boolean-valued array or record elements,
// boolean-valued user-defined function applications, and the
// boolean-valued order relations on enumeration types.

bool isAtomicProp(Node* n) {
    int k = n->kind;
    return
        k == z::TRUE
        || k == z::FALSE
        || k == z::EQ
        || k == z::NE
        || k == z::LT
        || k == z::I_LT
        || k == z::R_LT
        || k == z::GT
        || k == z::LE
        || k == z::I_LE
        || k == z::R_LE
        || k == z::GE;
}

bool isProp(Node* n) {return isAtomicProp(n) || isCompoundProp(n); }






//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// getSubNodes
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Use `sub-nodes' to refer to `logical' children.  Usually are the
// same as children, except for operators built using more than one
// node.

// Need vector<Node**> since might need to destructively update.


void appendChildPtrsToVector(Node* n, vector<Node**>& v) {
    for (int i = 0; i != n->arity(); i++) {
        v.push_back(&(n->child(i)));
    }
    return;
}

vector<Node**>
Node::getSubNodes() {

    vector<Node**> result;

    switch(kind) {

    case FORALL:
    case EXISTS: {
        // FORALL (SEQ d1 ... nk) body [pat]
        // EXISTS (SEQ d1 ... nk) body [pat]
        //   where di = DECL{id} type

        // Ignore pattern subterms.

        for (int i = 0; i != child(0)->arity(); i++) {
            Node* decl = child(0)->child(i);
            result.push_back(&(decl->child(0)));
        }
        result.push_back(&child(1));
        break;
    }

    case ARR_ELEMENT: {
        // ARR_ELEMENT arr (SEQ i1 ... in),  n >= 1
        result.push_back(&(child(0)));
        appendChildPtrsToVector(child(1), result);
        break;
    }
    case ARR_UPDATE: {
        // ARR_UPDATE arr (SEQ i1 ... in) val,  n >= 1
        result.push_back(&(child(0)));
        appendChildPtrsToVector(child(1), result);
        result.push_back(&(child(2)));
        break;
    }
    case ARR_BOX_UPDATE: {
        // ARR_BOX_UPDATE arr (SEQ r1 ... rn) val,  n >= 1
        //   where rj = SUBRANGE i1 i2

        result.push_back(&(child(0)));
        Node* rangeTuple = child(1);
        for (int j = 0; j != rangeTuple->arity(); j++) {
            Node* range = rangeTuple->child(j);
            appendChildPtrsToVector(range, result);
        }
        result.push_back(&(child(2)));
        break;
    }
    case MK_ARRAY: {
        // MK_ARRAY{arrname} default a1 ... an, n >= 0
        // MK_ARRAY{arrname} a1 ... an,         n >= 1
        //   where ai = ASSIGN (SEQ i1 ... im) val,  m >= 1
        //         ij = e  |  SUBRANGE e1 e2

        for (int i = 0; i != arity(); i++) {
            Node* c = child(i);
            if (c->kind != ASSIGN) {
                result.push_back(&child(i));
                continue;
            }
            Node* indexList = c->child(0);

            for (int j = 0; j != indexList->arity(); j++) {
                if (indexList->child(j)->kind == SUBRANGE) {

                    result.push_back(& indexList->child(j)->child(0));
                    result.push_back(& indexList->child(j)->child(1));
                }
                else {
                    result.push_back(& indexList->child(j));
                }
            }
            result.push_back(& c->child(1));
        }
        break;
    }
    case MK_RECORD: {
        // MK_RECORD{rcdname} a1 ... an, n >= 1
        //   where ai = ASSIGN{fldname} val

        for (int i = 0; i < arity(); i++) {
            Node* assign = child(i);
            result.push_back(&(assign->child(0)));
        }
        break;
    }
    case RECORD_TY: {
        // RECORD_TY{rcdname} d1 ... dn, n >= 1
        //   where di = DECL{fldname} Ti

        for (int i = 0; i < arity(); i++) {
            Node* decl = child(i);
            result.push_back(&(decl->child(0)));
        }
        break;
    }
    default: {
        appendChildPtrsToVector(this, result);
        break;
    }
    }
    return result;
}




//========================================================================
// Term analysis.
//========================================================================


class GatherOps {
public:
    set<string> OpSet;

    void operator() (Node* n);
};

void
GatherOps::operator() (Node* n) {
    string s (kindString(n->kind));
    if (s != "ID" && s != "NATNUM" && n->id.size() != 0) {
        s.append("{");
        s.append(n->id);
        s.append("}");
    }
    OpSet.insert(s);
    return;
}

string
gatherKinds(Node* n) {
    if (n == 0) return "";

    GatherOps gFun;
    n->mapOver(gFun);

    set<string> kinds = gFun.OpSet;

    kinds.erase("HYPS");
    string result;
    for (set<string>::iterator i = kinds.begin(); i!= kinds.end(); i++ ) {
        result.append(" ");
        result.append(*i);
    }
    return result;
}

// Collect names of bound variables

class GatherBVs {
public:
    set<string> vSet;
    void operator() (Node* n);
};

void
GatherBVs::operator() (Node* n) {
    if (n->kind == FORALL || n->kind == EXISTS) {
        Node* decls = n->child(0);
        for (int i = 0; i != decls->arity(); i++) {
            Node* decl = decls->child(i);
            vSet.insert(decl->id);
        }
    }
    return;
}

set<string>
gatherBoundVars(Node* n) {

    GatherBVs gFun;
    n->mapOver(gFun);
    return gFun.vSet;
}




Node*
nameToType(const std::string& s) {
    if (s == "integer") return new Node(INT_TY);
    if (s == "real") return new Node(REAL_TY);
    if (s == "boolean") return new Node(BOOL_TY);
    if (s == "bit___type") return new Node(BIT_TY);
    else
        return new Node(TYPE_ID, s);
}


std::string
typeToName(Node* n) {
    if (n->kind == INT_TY) return std::string("integer");
    if (n->kind == REAL_TY) return std::string("real");
    if (n->kind == BOOL_TY) return std::string("boolean");
    if (n->kind == BIT_TY) return std::string("bit___type");
    if (n->kind == TYPE_ID) return n->id;
    else {
        printMessage(ERRORm,
                     "typeToName: bad arg " + kindString(n->kind));
        return std::string("unknown___type");
    }
}


// Expects hyps to be AND(h1 ... hk) or NOT AND(h1 ... hk) where k >= 0.

// If k = 0   returns  concl
// If k = 1   return   h1 => concl
// If k > 1   return   AND(h1 ... hk) => concl

Node* mkGeneralImplies(Node* hyps, Node* concl) {
    if (hyps->kind == NOT) {

        Node* conj = hyps->child(0);

        if (conj->arity() == 0) {
            return new Node(TRUE);
        } else if (conj->arity() == 1) {
            return new Node(IMPLIES, new Node(NOT, conj->child(0)), concl);
        } else {
            return new Node(IMPLIES, hyps, concl);
        }

    } else {

        if (hyps->arity() == 0) {
            return concl;
        } else if (hyps->arity() == 1) {
            return new Node(IMPLIES, hyps->child(0), concl);
        } else {
            return new Node(IMPLIES, hyps, concl);
        }

    }

}

// Polymorphic nodes used in FDL.  Expectation is that all polymorphism is
// resolved in normalisation to Standard Form.

bool isPolymorphicNode(Node* n) {

    switch(n->kind) {

    case LT :
    case GT:
    case LE:
    case GE:
    case UMINUS:
    case SUCC:
    case PRED:
    case PLUS:
    case MINUS:
    case TIMES:
    case EXP:
    case SQR:
    case ABS:
        return true;

    case ARR_ELEMENT:
    case ARR_UPDATE:
        return n->id.size() == 0;

    case RCD_ELEMENT:
        // RCD_ELEMENT{rcd-id} exp
        // RCD_ELEMENT{rcd-id} exp (TYPE_PARAM{type-id})
        return n->arity() == 1;
    case TERM_EQ:
    case TERM_NE:
    case EQ:
    case NE:
    case RCD_UPDATE:
        // RCD_UPDATE{rcd-id} exp val
        // RCD_UPDATE{rcd-id} exp val (TYPE_PARAM{type-id})
        return n->arity() == 2;
    case ITE:
        return n->arity() == 3;
    default:
        return false;
    }
}
