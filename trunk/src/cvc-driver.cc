//==========================================================================
//==========================================================================
// CVC-DRIVER.CC
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

//==========================================================================
// CVC3 specific code.

//--------------------------------------------------------------------------
// C++ headers
//--------------------------------------------------------------------------

#include <cassert>
#include <map>
using std::map;
using std::pair;

#include <vector>
using std::vector;

#include<iostream>
using std::cout;
using std::endl;

#include <sstream>
using std::ostringstream;

#include <stdexcept>

//--------------------------------------------------------------------------
// My header files
//--------------------------------------------------------------------------

#include "cvc-driver.hh"
#include "utility.hh"
#include "formatter.hh"

//--------------------------------------------------------------------------
// CVC header files.
//--------------------------------------------------------------------------

#include "vc.h"

using CVC3::ValidityChecker;
using CVC3::Type;
using CVC3::Expr;
using CVC3::Op;
using CVC3::QueryResult;


#include "theory_arith.h" // for arith kinds and expressions

#include "command_line_flags.h"
using CVC3::CLFlags;

#include "exception.h" 

#include "expr_map.h"
using CVC3::ExprMap;


#include "theory_core.h"
#include "vcl.h" // ValidityChecker Implementation


using namespace z;



//==========================================================================
// CVCState class and member functions
//==========================================================================

// Allocate storage for flags in case create(flag) doesn't make its own
// copy, but continually refers back. (Seems like a sensible behaviour for
// it to have)

class CVCState {
private:
    CLFlags flags;   
    ValidityChecker* vc;

    map<string, Expr> varMap;
    map<string, Type> typeMap;
    map<string, Op> funMap;

    vector<pair<string, Expr> > bVarAlist;

    int bVarUId;  // Unique ID number for bound variables.


public:
    CVCState(string fileRoot);

    ~CVCState() {

        varMap.clear();
        typeMap.clear();
        funMap.clear();
        bVarAlist.clear();
        
        delete vc;
    }

    Expr pushBinding(Node* decl); // Returns bound var Expr
    void popBinding();
    bool isBound(const string& s);
    Expr lookupBinding(const string& s); // Finds bound var nearest in scope

    Expr translateExpr(Node* n);
    Type translateType(Node* n);


    // Main functions for external use

    void processDecl(Node* decl);
    void assertFormula(Node* n);
    void outputCounterExample();
    QueryResult doCheck();

    bool outOfResources();
    bool timeLimitReached();
    bool incomplete();
    bool incomplete(vector<string>& reasons);

};

//==========================================================================
// CVCDriver Class
//==========================================================================
class CVCDriver : public SMTDriver {
private:

    CVCState* state;
    Status status;

public:

    CVCDriver() {}
    
    // virtual void initSession();

protected:

    virtual Node* translateUnit(Node* unit);

    virtual void initGoal(const string& unitName,
                          int goalNum,
                          int ConclNum);


    virtual void addDecl(Node* n);
    virtual void addHyp(Node* h, const string& hId, string& remarks);
    virtual void addConcl(Node* n, string& format); 

    //    virtual void finishSetup();

    virtual bool checkGoal(string& format);
    
    virtual Status getResults(string& remarks);

    virtual void finaliseGoal();

    // virtual void finaliseSession()
};



SMTDriver* newCVCDriver() {
    return new CVCDriver();
}



//==========================================================================
// CVCState constructor and virtual functions
//==========================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CVCState()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CVCState::CVCState(string fileRoot) : flags(ValidityChecker::createFlags()) { 

    //    flags = ValidityChecker::createFlags();

    if (option("cvc-category"))
        flags.setFlag("category", optionVal("cvc-category"));

    string echoFileSuffix =
        option("cvc-echo-suffix") ? optionVal("cvc-echo-suffix") : "cvc";

    string outputFileSuffix =
        option("cvc-echo-suffix") ? optionVal("cvc-echo-suffix") : "clog";
    
    if (option("cvc-loginput"))
        flags.setFlag("dump-log", fileRoot + "." + echoFileSuffix);

    if (option("cvc-logoutput"))
        flags.setFlag("dump-trace", fileRoot + "." + outputFileSuffix);

    if (option("cvc-translate"))
        flags.setFlag("translate", true);
    
    if (option("cvc-echo-lang"))
        flags.setFlag("output-lang", optionVal("cvc-echo-lang"));

    if (option("cvc-echo-expected"))
        flags.setFlag("expResult", optionVal("cvc-echo-expected"));

    if (option("cvc-indent"))
        flags.setFlag("indent", optionVal("cvc-indent"));
    
    if (option("cnewarith")) flags.setFlag("arith-new", true);

    if (option("cvc-old-quant-inst")) flags.setFlag("quant-new", false);

    vc = ValidityChecker::create(flags);

    if (option("resourcelimit"))
        vc->setResourceLimit(intOptionVal("resourcelimit"));

    if (option("timeout"))
        vc->setTimeLimit(intOptionVal("timeout"));
        
    bVarUId = 0;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// pushBinding
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Expr 
CVCState::pushBinding(Node* decl) { // Returns bound var Expr
    Type ty = translateType(decl->child(0));
    Expr bv = vc->boundVarExpr(decl->id,
                               intToString(bVarUId),
                               ty);
    bVarUId++;
    bVarAlist.push_back(pair<string, Expr>(decl->id, bv));
    return bv;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// popBinding
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void 
CVCState::popBinding() {
    bVarAlist.pop_back();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// isBound
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool 
CVCState::isBound(const string& s) {
    for (int i = bVarAlist.size(); i > 0;) {
        i--;
        if (s == bVarAlist.at(i).first) return true;
    }
    return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// lookupBinding
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Expr 
CVCState::lookupBinding(const string& s) { // Finds bound var nearest in scope
    for (int i = bVarAlist.size(); i > 0;) {
        i--;
        if (s == bVarAlist.at(i).first) return bVarAlist.at(i).second;
    }
    throw std::runtime_error
        ("CVCState::lookupBinding: binding not found for: " + s);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// translateType
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


Type
CVCState::translateType(Node* n) {

    switch(n->kind) {
    case INT_TY:
        return vc->intType();
    case REAL_TY:
        return vc->realType();
    case BOOL_TY:
        return vc->boolType();
    case BITVEC_TY:
        return vc->bitvecType(stringToInt(n->id));
    case BIT_TY:
        return typeMap.find("bit__ty")->second;
    case SUBRANGE_TY: {
        return vc->subrangeType(translateExpr(n->child(0)),
                                translateExpr(n->child(1))
                                );
    }
    // RECORD_TY decl+
    case RECORD_TY: { 
        vector<string> ids;
        vector<Type>  tys;
        for (int i = 0; i != n->arity(); i++) {
            Node* decl = n->child(i);
            ids.push_back(decl->id);
            tys.push_back(translateType(decl->child(0)));
        }
        return vc->recordType(ids, tys);
    }

    //  ARRAY_TY (SEQ ts) t 
    case ARRAY_TY:  {
        if (n->child(0)->arity() == 1) {
            return vc->arrayType(translateType(n->child(0)->child(0)),
                                 translateType(n->child(1))
                                 );
        } else {
            return vc->arrayType(translateType(n->child(0)),
                                 translateType(n->child(1))
                                 );
        }
    }

    case SEQ: {
        vector<Type>  tys;
        for (int i = 0; i != n->arity(); i++) {
            tys.push_back(translateType(n->child(i)));
        }
        return vc->tupleType(tys);
    }
    case TYPE_ID: {
        if (typeMap.count(n->id) > 0)
            return typeMap.find(n->id)->second;
        else {
            throw std::runtime_error
                ("CVCState::translateType: unexpected TYPE_ID: "
                 + (n->id));
        }
    }
    case ENUM_TY:
    default: {
        throw std::runtime_error("CVCState::translateType: unrecognised kind "
                                 + kindString(n->kind));
    }
    } // END switch

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// translateExpr
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Expr
CVCState::translateExpr(Node* n) {

    // Special cases first.
    if (n->kind == FORALL || n->kind == EXISTS) {

        vector<Expr> bvars;
        
        for ( int i = 0;
              i != n->child(0)->arity();
              i++) {
            Node* decl = n->child(0)->child(i);
            Expr bvar = pushBinding(decl);
            bvars.push_back(bvar);
        }

        Expr eChild = translateExpr(n->child(1));

        for ( int i = 0;
              i != n->child(0)->arity();
              i++) {
            popBinding();
        }

        if (n->kind == FORALL)
            return vc->forallExpr(bvars, eChild);
        else
            return vc->existsExpr(bvars, eChild);
    }

    //    RCD_ELEMENT{rcd-id} exp [ TYPE_PARAM{t-id} ]
    if (n->kind == RCD_ELEMENT) 
        return vc->recSelectExpr( translateExpr(n->child(0)),
                                  n->id);

    //    RCD_UPDATE{rcd-id} exp exp [ TYPE_PARAM{t-id} ]
    if (n->kind == RCD_UPDATE) 
        return vc->recUpdateExpr(translateExpr(n->child(0)),
                                 n->id,
                                 translateExpr(n->child(1))
                                 );

    // MK_RECORD{rcd-id} a1 ... an  where ai = ASSIGN{fi} ei

    if (n->kind == MK_RECORD) {

        vector<Expr> es;
        vector<string> fnames;

        for (int i = 0; i != n->arity(); i++) {
            es.push_back( translateExpr(n->child(i)->child(0)) );
            fnames.push_back(n->child(i)->id);
        }

        return vc->recordExpr(fnames, es);
    }

    // Uniform cases now follow

    // Translate children nodes. 
    vector<Expr> es;

    for (int i = 0; i != n->arity(); i++) {
        es.push_back( translateExpr(n->child(i)) );
    }

    switch(n->kind) {
    case IFF:
        return vc->iffExpr(es.at(0), es.at(1));
    case IMPLIES:
        return vc->impliesExpr(es.at(0), es.at(1));
    case AND: {
        if (n->arity() == 2) 
            return vc->andExpr(es.at(0), es.at(1));
        else
            return vc->andExpr(es);
    }
    case OR: {
        if (n->arity() == 2) 
            return vc->orExpr(es.at(0), es.at(1));
        else
            return vc->orExpr(es);
    }
    case NOT:
        return vc->notExpr(es.at(0));
    case EQ:
        return vc->eqExpr(es.at(0), es.at(1));
    case NE:
        return vc->notExpr(vc->eqExpr(es.at(0), es.at(1)));
    case I_LT :
    case R_LT :
        return vc->ltExpr(es.at(0), es.at(1));
    case I_LE:
    case R_LE:
        return vc->leExpr(es.at(0), es.at(1));
    case I_UMINUS:
    case R_UMINUS:
        return vc->uminusExpr(es.at(0));
    case I_PLUS:
    case R_PLUS:
        return vc->plusExpr(es.at(0),es.at(1));
    case I_MINUS:
    case R_MINUS:
        return vc->minusExpr(es.at(0),es.at(1));
    case I_TIMES:
    case R_TIMES:
        return vc->multExpr(es.at(0),es.at(1));
    case RDIV:
        return vc->divideExpr(es.at(0),es.at(1));
    case TO_REAL:
        return es.at(0);
    case TRUE:
        return vc->trueExpr();
    case FALSE:
        return vc->falseExpr();
    case TERM_TRUE: 
            return vc->newBVConstExpr(string("1"));
    case TERM_FALSE: 
            return vc->newBVConstExpr(string("0"));
    case TERM_AND: 
        return vc->newBVAndExpr(es.at(0), es.at(1));
    case TERM_OR: 
        return vc->newBVOrExpr(es.at(0), es.at(1));
    case TERM_NOT: 
        return vc->newBVNegExpr(es.at(0));
    case TERM_I_LT:
        return vc->iteExpr(vc->ltExpr(es.at(0),es.at(1)),
                           vc->newBVConstExpr(string("1")),
                           vc->newBVConstExpr(string("0")));
    case TERM_I_LE:
        return vc->iteExpr(vc->leExpr(es.at(0),es.at(1)),
                           vc->newBVConstExpr(string("1")),
                           vc->newBVConstExpr(string("0")));
    case TERM_EQ:
        return vc->iteExpr(vc->eqExpr(es.at(0),es.at(1)),
                           vc->newBVConstExpr(string("1")),
                           vc->newBVConstExpr(string("0")));
    case TERM_NE:
        return vc->iteExpr(vc->eqExpr(es.at(0),es.at(1)),
                           vc->newBVConstExpr(string("0")),
                           vc->newBVConstExpr(string("1")));
    case VAR: {
        if (isBound(n->id)) 
            return lookupBinding(n->id);
        else
            throw std::runtime_error
                ("CVCState::translateExpr: unexpected VAR: "
                 + (n->id));
    }            
    case CONST: {
        if (varMap.count(n->id) > 0)
            return varMap.find(n->id)->second;
        else {
            throw std::runtime_error
                ("CVCState::translateExpr: unexpected CONST: "
                 + (n->id));
        }
    }
    case NATNUM:
        return vc->ratExpr(n->id);
    case I_EXP: 
    case R_EXP: 
        return vc->powExpr(es.at(0),es.at(1));


    case ARR_ELEMENT: {
        if (n->child(1)->arity() == 1) {// Catch 1D array special case
            return vc->readExpr(es.at(0), es.at(1).getKids().at(0));
        } else {
            return vc->readExpr(es.at(0), es.at(1));
        }
    }
    case ARR_UPDATE: {
        if (n->child(1)->arity() == 1) {// Catch 1D array special case
            return vc->writeExpr(es.at(0),
                                 es.at(1).getKids().at(0),
                                 es.at(2));
        } else {
            return vc->writeExpr(es.at(0), es.at(1), es.at(2));
        }
    }
    case SEQ:   // Only expect this to be used to tuple up index components
                // for ARR_ELEMENT and ARR_UPDATE
    case TUPLE: // Currently unused
        return vc->tupleExpr(es);

    //  FUN_AP{funid} es  --> 
    //  FUN_AP{integer__succ} e1 -->
    //  FUN_AP{integer__pred} e1 -->

    case PRED_AP:
    case FUN_AP: {

        if (funMap.count(n->id) > 0) {
            return vc->funExpr(funMap.find(n->id)->second, es);
        }
        if (es.size() == 0 && varMap.count(n->id) > 0) {
            // Nullary PRED_APs.
            return varMap.find(n->id)->second;
        }
        else {
            throw std::runtime_error
                ("CVCState::translateExpr: unbound FUN_AP/PRED_AP id: "
                 + (n->id));
        }

    } // END case FUN_AP
    default: {
        throw std::runtime_error("CVCState::translateExpr: unrecognised kind "
                                 + kindString(n->kind));
    }

    } // END switch
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// outputCounterExample
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Only call when CVC query returns false.

void
CVCState::outputCounterExample() {
    if (! option("counterex")) return;
    
    ExprMap<Expr> m;
    vc->getConcreteModel(m);

    ostringstream oss;

    oss << "Counter Example" << endl;

    for (ExprMap<Expr>::iterator i = m.begin();  i != m.end(); i++ ) {
        vc->printExpr(i->first, oss);
        oss << " --> ";
        vc->printExpr(i->second, oss);
        oss << endl;
    }
    printMessage(INFOm, oss.str());
    return;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// processDecl
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void 
CVCState::processDecl(Node* decl) {

    string dId = decl->id;
    switch (decl->kind) {

    case DEF_TYPE: {
        // DEF_TYPE  {id}  type 
        // DEF_TYPE {id}
        Type t;
        if (decl->arity() > 0) {
            t = vc->createType(dId, translateType(decl->child(0)));
        } else {
            t = vc->createType(dId);
        }
        typeMap.insert(pair<string,Type>(dId, t));
        return;
    }
    case DEF_CONST: {
        // DEF_CONST  {id} type exp
        // DEF_CONST {id} type
        Expr c;
        Type t (translateType(decl->child(0)));
                
        if (decl->arity() == 2) {
            c = vc->varExpr(dId, t, translateExpr(decl->child(1)));
        } else {
            c = vc->varExpr(dId, t);
        }
        varMap.insert(pair<string,Expr>(dId, c));
        return;
    }
    case DECL_VAR: {
        // DECL_VAR {id} type
        Expr v = vc->varExpr(dId, translateType(decl->child(0)));
        varMap.insert(pair<string,Expr>(dId, v));
        return;
    }
    case DECL_FUN: {
        // DECL_FUN {id} (SEQ type+) type

        int domArity = decl->child(0)->arity();
        vector<Type> domTys;
        for (int i = 0; i != domArity; i++) {
            domTys.push_back(translateType(decl->child(0)->child(i)));
        }
        Type rangeTy = translateType(decl->child(1));
        Type funTy = vc->funType(domTys, rangeTy);
        Op op = vc->createOp(dId, funTy);
        funMap.insert(pair<string,Op>(dId, op));
        return;
    }
    default: {
        throw std::runtime_error("CVCState::processDecl: unrecognised kind "
                        + kindString(decl->kind));
    }
    } // END switch
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// assertFormula
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void 
CVCState::assertFormula(Node* n) {
    vc->assertFormula(translateExpr(n));
    return;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// doCheck
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


QueryResult 
CVCState::doCheck() {
    Expr falseExp = translateExpr(new Node(FALSE));
    return vc->query(falseExp);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// outOfResources
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// CVC3 implementation dependency here.
bool 
CVCState::outOfResources() {
    return ((CVC3::VCL*) vc)->core()->outOfResources();
}

// This doesn't work because timeLimitReached is private
bool 
CVCState::timeLimitReached() {
    //    return ((CVC3::VCL*) vc)->core()->timeLimitReached();
    return false;
}

bool 
CVCState::incomplete() {
    return vc->incomplete();
}

bool 
CVCState::incomplete(vector<string>& reasons) {
    return vc->incomplete(reasons);
}


//==========================================================================
// CVCDriver Virtual Functions
//==========================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// translateUnit()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Only do decls.

Node*
CVCDriver::translateUnit(Node* unit) {

    Node* newDecls = new Node(DECLS);

    Node* bitTy =
        new Node (BITVEC_TY, "1");

    // Old alternative.  No longer useful since need bit operations.
    //        option("cvc-use-bitvec-for-bit-ty")
    //        : new Node (SUBRANGE_TY,
    //                    new Node(NATNUM, "0"),
    //                    new Node(NATNUM, "1")
    //                    );

    // DEF_TYPE{bit__ty} (SUBRANGE_TY NATNUM{0} NATNUM{1}) 

    newDecls->addChild( new Node (DEF_TYPE, "bit__ty", bitTy));

    newDecls->appendChildren(unit->child(0));

    Node* newUnit = new Node(z::UNIT, "",
                             newDecls,
                             unit->child(1),
                             unit->child(2));
    return newUnit;
}

void 
CVCDriver::initGoal(const string& unitName,
                    int goalNum,
                    int conclNum) {
    string fullGoalFileRoot
        = getFullGoalFileRoot("cvc3", unitName, goalNum, conclNum);

    state = new CVCState(fullGoalFileRoot);
}

void 
CVCDriver::addDecl(Node* decl) {
    try {
        state->processDecl(decl);
    } catch (CVC3::Exception& e) {
        throw std::runtime_error
            ("CVCDriver::addDecl: caught CVC3::Exception" + ENDLs
             + e.toString());
    }
}

void 
CVCDriver::addHyp(Node* h, const string& hId, string& remarks) {
    try {
        state->assertFormula(h);
    } catch (CVC3::Exception& e) {
        throw std::runtime_error
            ("CVCDriver::addHyp: caught CVC3::Exception" + ENDLs
             + e.toString());
    }
}

void 
CVCDriver::addConcl(Node* n, string& format) {
    try {
        state->assertFormula(new Node(NOT, n));
    } catch (CVC3::Exception& e) {
        throw std::runtime_error
            ("CVCDriver::addConcl: caught CVC3::Exception" + ENDLs
             + e.toString());
    }
} 

    //    virtual void finishSetup();

bool 
CVCDriver::checkGoal(string& remarks) {
    QueryResult queryResult;
    try {
        queryResult = state->doCheck();

    } catch (CVC3::Exception& e) {

        appendCommaString(remarks, "check error");
        printMessage
            (ERRORm,
             "CVCDriver::checkGoal: caught CVC3::Exception" + ENDLs
             + e.toString());
        status = ERROR;
        return true;
    }

    
    switch (queryResult) {
    case CVC3::VALID: 
	printMessage(FINEm, "concl true");
        status = TRUE;
        return false;

    case CVC3::INVALID:
        appendCommaString(remarks, "definitely false");
	printMessage(FINEm, "concl false");
        state->outputCounterExample();
        status = UNPROVEN;
        return false;

    case CVC3::UNKNOWN: {

        string reasonsString;
        vector<string> reasons;
        state->incomplete(reasons);

        bool resourceLimitReached = false;

        for (int i = 0; i != (int) reasons.size(); i++) {
            appendCommaString(reasonsString, reasons.at(i));

            if (reasons.at(i) == "Exhausted user-specified resource") {

                appendCommaString(remarks, "out of resources");
                resourceLimitReached = true;
                
            } else if (reasons.at(i)
                       == "Exhausted user-specified time limit") {

                appendCommaString(remarks, "timeout");
                resourceLimitReached = true;

            } else if (reasons.at(i)
                       == "Non-linear arithmetic equalities") {

                appendCommaString(remarks, "non-lin eqs");

            } else if (reasons.at(i)
                       == "Non-linear arithmetic inequalities") {

                appendCommaString(remarks, "non-lin ineqs");
            } else if (reasons.at(i)
                       == "Quantifier instantiation") {

                appendCommaString(remarks, "quant inst");
            } else {

                appendCommaString(remarks, reasons.at(i));

            }
        }

        printMessage(INFOm,
                     "CVCDriver::checkGoal: got UNKNOWN query result"
                     + ENDLs + reasonsString);

        state->outputCounterExample();
        status =  resourceLimitReached ? RESOURCE_LIMIT : UNPROVEN;
        return false;
    }
        
    case CVC3::ABORT: {
        appendCommaString(remarks, "query aborted");

        bool returnVal = true;
        
        string reasonsString;
        vector<string> reasons;
        state->incomplete(reasons);
        for (int i = 0; i != (int) reasons.size(); i++) {

            appendCommaString(reasonsString, reasons.at(i));

        }
            
        printMessage(WARNINGm,
                     "CVCDriver::checkGoal: got ABORT query result"
                     + ENDLs + reasonsString);

        status = UNPROVEN;

        return returnVal;


        /*
        if ( state->outOfResources()) {
            appendCommaString(remarks, "out of resources");
            printMessage(WARNINGm,
                         "CVCDriver::checkGoal: CVC3 resource limit reached");
            status = UNPROVEN;
            return false;
        }
        else if ( state->timeLimitReached()) {
            appendCommaString(remarks, "timeout");
            printMessage(WARNINGm,
                         "CVCDriver::checkGoal: CVC3 time limit reached");
            status = UNPROVEN;
            return false;
        } else {
            appendCommaString(remarks, "query aborted");
            printMessage(WARNINGm,
                         "CVCDriver::checkGoal: got ABORT query result");
            return true;
        }
        */
    }
    default:
        assert(false);
    } // END switch
    assert(false);
    return true;
}

SMTDriver::Status     
CVCDriver::getResults(string& remarks) {
    return status;
}

void 
CVCDriver::finaliseGoal() {
    try {
        delete state;

    } catch (CVC3::Exception& e) {
        // Remarks not defined.
        // appendCommaString(remarks, "finalise goal error");
        printMessage
            (ERRORm,
             "CVCDriver::finaliseGoal: caught CVC3::Exception" + ENDLs
             + e.toString());
        // Not wonderful idea since smt-driver doesn't yet handle this!
        throw std::runtime_error
            ("CVCDriver::finaliseGoal: caught CVC3::Exception" + ENDLs
             + e.toString());
    }
    return;

}



