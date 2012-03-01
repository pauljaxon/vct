//==========================================================================
//==========================================================================
// YICES-DRIVER.CC
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
// Yices specific code.

#include <sstream>
using std::ostringstream;

#include <stdexcept>

extern "C" {

#include "yicesl_c.h"

}

#include "yices-driver.hh"

#include "formatter.hh"
#include "pprinter.hh"

#include "node-utils.hh"
#include "bignum.hh"


using namespace z;



//==========================================================================
// Pretty print Yices abstract syntax
//==========================================================================


/* 

Most expressions have 

1. lead string
2. list of subterms

Always want "(" added front and ")" added matching at end.
For vertical layout, use indent 2, with lead string on separate line

For horizontal layout, single space after opening delimiter and after

DECL    Use infix "::".  Append to end of prefix header.  Indent rest.

SEQ  H or Vertical: For V, line up all at same indent. 

*/

class YicesFormatter : public Formatter {
public:
    virtual Box& addSyntax(z::Kind k, const string& id,
                           vector<Box*>& bs);

    static Formatter* getFormatter();

private:

    static Formatter* instance;
    YicesFormatter();
};



Box& YicesFormatter::addSyntax(z::Kind k, const std::string& id,
                               vector<Box*>& bs) {

    switch (k) {

    // TOP LEVEL

    case(DEF_TYPE):  return makeStringAp("define-type " + id, bs);
    case(DEF_CONST): return PP::makeHVSeq("(define " + id,
                                          "::",
                                          "",
                                          ")",
                                          bs);
    case(ASSERT):    return makeStringAp("assert", bs);
    case(CHECK):     return box("(check)");
    case(PUSH):      return box("(push)");
    case(POP):       return box("(pop)");
        
    // TYPES

    case(FUN_TY):      return makeStringAp("->", bs);
    case(TUPLE_TY):    return makeStringAp("tuple", bs);
    case(ENUM_TY):     return makeStringAp("scalar", bs);
    case(SUBRANGE_TY): return makeStringAp("subrange", bs);
    case(REAL_TY):     return box("real");
    case(INT_TY):      return box("int");
    case(BOOL_TY):     return box("bool");

    case(RECORD_TY):   return makeStringAp("record", bs);

    // EXPRS

    case(OR):          return makeStringAp("or", bs);
    case(AND):         return makeStringAp("and", bs);
    case(NOT):         return makeStringAp("not", bs);
    case(IMPLIES):     return makeStringAp("=>", bs);
    case(EQ):          return makeStringAp("=", bs);
    case(NE):          return makeStringAp("/=", bs);

    case(FORALL):      return makeStringAp("forall", bs);
    case(EXISTS):      return makeStringAp("exists", bs);
    case(LAMBDA):      return makeStringAp("lambda", bs);
    case(TUPLE):
    case(SEQ):         return PP::makeHVSeq("",
                                            "(",
                                            "",
                                            ")",
                                            bs);
    case(ASSIGN):
    case(DECL):        return PP::makeHVSeq(id + "::",
                                            "",
                                            "",
                                            "",
                                            bs);
    case(I_LT):       return makeStringAp("<", bs);
    case(I_LE):       return makeStringAp("<=", bs);
    case(R_LT):       return makeStringAp("<", bs);
    case(R_LE):       return makeStringAp("<=", bs);
    case(I_PLUS):        return makeStringAp("+", bs);
    case(I_MINUS):       return makeStringAp("-", bs);
    case(I_TIMES):       return makeStringAp("*", bs);
    case(I_UMINUS):      return makeStringAp("-", bs);
    case(R_PLUS):        return makeStringAp("+", bs);
    case(R_MINUS):       return makeStringAp("-", bs);
    case(R_TIMES):       return makeStringAp("*", bs);
    case(RDIV):        return makeStringAp("/", bs);
    case(R_UMINUS):      return makeStringAp("-", bs);

    // Yices 1.0.3 does not recognise "div". So introduce new uifunction
    // case(IDIV):        return makeStringAp("i__div", bs);
    case(IDIV):        return makeStringAp("div", bs);
    // Yices 1.0.9 does.
    case(MOD):         return makeStringAp("mod", bs);
    case(APPLY):       return PP::makeHVSeq("",
                                            "(",
                                            "",
                                            ")",
                                            bs);
    case(CONST):       return box(id);
    case(VAR):         return box(id);
    case(TYPE_ID):     return box(id);
    case(TRUE):        return box("true");
    case(FALSE):       return box("false");
    case(NATNUM):      return box(id);
    case(PENDING):     return box("pending");

    case(UPDATE):      return makeStringAp("update", bs);
    case(SELECT):      return makeStringAp("select", bs);
    case(MK_RECORD):   return makeStringAp("mk-record", bs);
    case(GT): 
    case(GE): 
    case(LT): 
    case(LE): 
    default:
	std::cerr << "YicesFormatter::addSyntax: "
		  << "Encountered unrecognised kind "
		  << kindString(k) << endl;
        return PP::makeHVSeq("***" + kindString(k) + "{" + id + "}***",
                             "[",
                             ";",
                             "]",
                             bs
                             );
    }
}


YicesFormatter::YicesFormatter() {};

Formatter* 
YicesFormatter::instance = 0;

Formatter*
YicesFormatter::getFormatter() {
    if (instance == 0) instance = new YicesFormatter;
    return instance;
}


//==========================================================================
// Translate FDL abstract syntax to Yices abstract syntax
//==========================================================================
// Translation non destructive.

// Yices parser disallows use of its keywords as identifiers, type identifiers
// and record fieldnames, so we have to make these all distinct:

// ' suffix added to type ids
// . suffix added to ids (vars and consts) and field selectors.


class YicesTranslator : public Translator {
    
public:
    YicesTranslator() : Translator("yices") {};

protected:
    virtual Node* translateAux (Node* oldN);
};



Node* 
YicesTranslator::translateAux (Node* oldN) {

/*
----------------------------------------------------------------------------
Main translation
----------------------------------------------------------------------------
*/


    // default value of new kind.
    
    Node* newN = new Node(oldN->kind, oldN->id);

    for (int i = 0; i != oldN->arity(); i++) {
        newN->addChild( translateAux(oldN->child(i)) );
    }
    switch (newN->kind) {
/* 
----------------------------------------------------------------------------
Top level of units
----------------------------------------------------------------------------
*/
    case UNIT:

    case RULES:
    case RULE:
    case GOALS:

    case GOAL:
    case CONCLS:        
    case HYPS:        
    case DECLS:
        return newN;
        
/* 
----------------------------------------------------------------------------
fdl declarations
----------------------------------------------------------------------------
*/
    case DEF_TYPE: {

        newN->id.append("'");
        return newN;
    }
        
    case DEF_CONST: {

        newN->id.append(".");
        return newN;
    }

    case DECL_VAR:  {
        newN->kind = DEF_CONST;
        newN->id.append(".");
        return newN;
    }

    //  DECL_FUN {id} (SEQ ts) t  --> DEF_CONST {id} (FUN_TY ts' t')

    case DECL_FUN: {

        newN->id.append(".");
        newN->child(0)->addChild(newN->child(1));
        newN->popChild();
        newN->child(0)->kind = FUN_TY;
        newN->kind = DEF_CONST;
        return newN;
    }


/*
----------------------------------------------------------------------------
types
----------------------------------------------------------------------------
*/

    // Unchanged

    case INT_TY:
    case REAL_TY:
    case BOOL_TY:
    case ENUM_TY:
    case SUBRANGE_TY:
    case RECORD_TY: return newN;

    //  ARRAY_TY (SEQ ts) t  --> FUN_TY ts' t'

    case ARRAY_TY:  {
        newN->child(0)->addChild(newN->child(1));
        newN->child(0)->kind = FUN_TY;
        return newN->child(0);
    }

/*
----------------------------------------------------------------------------
expressions
----------------------------------------------------------------------------
*/
    // Unchanged

    case FORALL:
    case EXISTS:
    case IMPLIES:
    case AND:
    case OR:
    case NOT:
    case EQ:
    case NE:
    case I_LT:
    case I_LE:
    // case UMINUS:
    case I_PLUS:
    case I_TIMES:
    case I_MINUS:
    case IDIV: 
    case MOD: 
    case R_PLUS:
    case R_TIMES:
    case R_MINUS:
    case RDIV:
    case R_LT:
    case R_LE:
    case TRUE:
    case FALSE:
    case NATNUM: return newN;

    case TO_REAL: return newN->child(0);

    case VAR: {
        newN->id.append(".");
        return newN;
    }
    case CONST: {
        newN->id.append(".");
        return newN;
    }
    case TYPE_ID: {

        newN->id.append("'");
        return newN;
    }
        

    // Work around Yices 1.0.3 bug where UMINUS is identity!
    case I_UMINUS:
        newN->kind = I_MINUS;
        newN->addLeftChild(new Node(NATNUM, "0"));
        return newN;

    case R_UMINUS:
        newN->kind = R_MINUS;
        newN->addLeftChild(new Node(NATNUM, "0"));
        return newN;

    //  ARR_ELEMENT{ty-id} e1 (SEQ es)   --> APPLY e1' es'

    case ARR_ELEMENT: {
        newN->child(1)->addLeftChild(newN->child(0));
        newN->child(1)->kind = APPLY;
        return newN->child(1);
    }
    //  ARR_UPDATE{ty-id} e1 (SEQ es) e3 --> UPDATE e1' (SEQ es') e3'

    case ARR_UPDATE: {
        newN->kind = UPDATE;
        newN->id.clear();
        return newN;
    }

    //  RCD_ELEMENT{rcd-id} e t-id --> SELECT e' CONST{rcd-id.}

    case RCD_ELEMENT: {
        if (newN->arity() == 2) newN->popChild();
        
        newN->addChild(new Node(CONST, newN->id + "."));
        newN->kind = SELECT;
        newN->id.clear();
        return newN;
    }
    //  RCD_UPDATE{rcd-id} e1 e2 t-id --> UPDATE e1' CONST{rcd-id.} e2'

    case RCD_UPDATE: {
        if (newN->arity() == 3) newN->popChild();

        Node* e2 = newN->child(1);
        newN->popChild();
        newN->addChild(new Node(CONST, newN->id + "."));
        newN->addChild(e2);
        newN->id.clear();
        newN->kind = UPDATE;
        return newN;
    }

    // These only occur as children of RCD_UPDATE and RCD_ELEMENT.
    // Will be ignored when those nodes are translated.
    case TYPE_PARAM:
        return newN;

    case ASSIGN: {
        newN->id.append(".");
    }
    case MK_RECORD: {
        return newN;
    }
    //  IFF e1 e2 --> (EQ e1 e2)

    case IFF: {
        newN->kind = EQ;
        return newN;
    }

    //  FUN_AP{funid} es  --> APPLY CONST{funid} es

    case FUN_AP: {
        // Taken care of in prelude.rul

        newN->addLeftChild(new Node(CONST, newN->id + "."));
        newN->id.clear();
        newN->kind = APPLY;
        return newN;
    }

/*
----------------------------------------------------------------------------
Multipurpose
----------------------------------------------------------------------------
*/

        
    case DECL: {
        newN->id.append(".");
    }
    case TUPLE: return newN;
    case SEQ: return newN;

/*
----------------------------------------------------------------------------
Constructors without translation
----------------------------------------------------------------------------
*/

    case LE:
    case GE:
    default: {
            error ("unrecognised kind "
                   + kindString(newN->kind)
                   );
            return newN;
        }
    } // end switch(newN->kind)


}

//==========================================================================
// Calling Yices 
//==========================================================================
// Virtual function definitions for YicesDriver class

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// yicesRead
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Pass expression represented by node tree to Yices

// Returns true and sets yicesOutput string if are problems in read.

bool
YicesDriver::push(yicesl_context ctx,
                  Node* n,
                  string& yicesInput,
                  string& yicesOutput) {

    Formatter::setFormatter(YicesFormatter::getFormatter());

    ostringstream oss;
    oss << *n;
    const string nStr(oss.str());  // const string needed for c_str() call

    printMessage(FINESTm, "Yices reading" + ENDLs + nStr);
    
    if (yicesl_read(ctx, (char*) nStr.c_str()) == 0) { // cast away constness.
                                                       // for yicesl_read
        yicesInput = oss.str();
        yicesOutput = yicesl_get_last_error_message();
        return true;
    } else
        return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// formatErrorString()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

string
YicesDriver::formatErrorString(const string& yicesInput,
                               const string& yicesErrorOutput) {

    ostringstream s;

    s << "Yices read error" << endl;
    s << "input to Yices:" << endl << endl << yicesInput << endl << endl;
    s << "output from Yices:" << endl << endl << yicesErrorOutput << endl;

    return s.str();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ignoreErrorMessage()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool
YicesDriver::ignoreErrorMessage(const string& s) {
    return (s == "feature not supported: non linear problem.");
}





//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// translateUnit()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Node* 
YicesDriver::translateUnit(Node* unit) {
    YicesTranslator t; 
    Formatter::setFormatter(YicesFormatter::getFormatter());
    return t.translate(unit);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// initSession()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void 
YicesDriver::initSession() {
    // -------------------------------------------------------------------
    // Process command line options
    // -------------------------------------------------------------------

    if (option("yverb"))
        yicesl_set_verbosity(intOptionVal("yverb"));
    
    if (option ("ynotc")) 
        yicesl_enable_type_checker(false);
    else
        yicesl_enable_type_checker(true);



}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// initGoal()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void 
YicesDriver::initGoal(const string& unitName,
                      int goalNum,
                      int conclNum) {

    string fullGoalFileRoot
        = getFullGoalFileRoot("yices", unitName, goalNum, conclNum);

    ctx = yicesl_mk_context();

    // Log commands passed to Yices.
    if (option("yices-loginput")) {
        string saveFile = fullGoalFileRoot + ".yices";
        yicesl_enable_log_file((char*) saveFile.c_str());
    }

    // Redirect output messages from Yices (e.g. "unsat").
    // With 1.0.9 this used to be set on a session basis.  
    // With 1.0.16 seems that have to set for each new context.

    // Always set to something, since default is to send to standard output
    // which is just clutter.

    if (option("yices-logoutput")) {
        string outputFile = fullGoalFileRoot + ".ylog";
        yicesl_set_output_file((char*) outputFile.c_str());
    } else {
        yicesl_set_output_file((char*) "/dev/null");
    }


    if (option("counterex")) {
        yicesl_read(ctx, (char*) "(set-evidence! true)");
    }

    // ---------------------------------------------------------------
    // Add declarations of functions introduced by translation
    // ---------------------------------------------------------------
    // yicesl_read(ctx, "(define int__times :: (-> int int int))");
    // yicesl_read(ctx, "(define real__times :: (-> int int int))");

    // ---------------------------------------------------------------
    // Add declarations missing from API Lite context
    // ---------------------------------------------------------------
    // Yices executable is compiled to include these declarations, but
    // they are not included when calling Yices via API.

    // NB: For -ve dividend these declarations specify different values
    // for div and mod as expected for Ada/FDL.

    yicesl_read
        (ctx, (char*)
         "(define div::                                       \
                 (-> x::int y::int                                  \
                  (subtype (r::int) (if (> y 0)                     \
                                        (and (>= x (* y r))         \
                                             (< x (* y (+ r 1))))   \
                                        (and (<= x (* y r))         \
                                             (> x (* y (+ r 1))))   \
                                     )                              \
                   )))"
               );
        yicesl_read
            (ctx, (char*)
         "(define mod::(-> x::int y::int                                \
                              (subtype (r::int) (= (+ r (* (div x y) y)) x))))"
         );

    return;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// addDecl()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void 
YicesDriver::addDecl(Node* decl) {

    string yicesInput;
    string yicesOutput;
    if (push(ctx, decl, yicesInput, yicesOutput)) {

        throw std::runtime_error(formatErrorString(yicesInput,
                                                   yicesOutput));
    }
    return;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// addHyp()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void 
YicesDriver::addHyp(Node* hyp, const string& hypId, string& remarks) {

    string yicesInput;
    string yicesOutput;
    if (push(ctx, new Node(z::ASSERT, hyp), yicesInput, yicesOutput)) {

        string errorMessage(formatErrorString(yicesInput,
                                                   yicesOutput));


        if( ignoreErrorMessage(yicesOutput) ) {
        
            printMessage(WARNINGm,
                         "Yices rejected "
                         + hypId + ":" + ENDLs 
                         + errorMessage);

            appendCommaString(remarks, "-" + hypId);
            return;
        }        
        throw std::runtime_error(errorMessage);
    }
    return;
}    

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// addConcl()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
YicesDriver::addConcl(Node* concl, string& remarks) {
    addHyp(new Node (z::NOT, concl), "C", remarks);
    return;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// checkGoal()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool 
YicesDriver::checkGoal(string& remarks) {

    string yicesInput;
    string yicesOutput;
    if (push(ctx, new Node(z::CHECK), yicesInput, yicesOutput)) {

        // Detect whether problem due to non-linearity.
        // (non-linearity arises during checking 
        //    on instantiation of quantified formulas)

        if(ignoreErrorMessage(yicesOutput)) {
        
            printMessage(WARNINGm,
                         "Yices error during check" + ENDLs 
                         + yicesOutput);

            appendCommaString(remarks, "non-linearity in check");
            status = UNKNOWN;
            return false;
        } else {
            printMessage(ERRORm,
                         "Yices error during check" + ENDLs 
                         + yicesOutput);
            // Remarks update taken care of by driveUnit
            status = ERROR;
            return true;
        }
    }   
    if (yicesl_inconsistent(ctx)) 
        status = TRUE;
    else
        status = UNKNOWN;
    return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// getResults()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


SMTDriver::Status
YicesDriver::getResults(string& remarks) {
    return status;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// finaliseGoal()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
YicesDriver::finaliseGoal() {
    yicesl_del_context(ctx);
    return;
}

