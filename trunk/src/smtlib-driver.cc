//==========================================================================
//==========================================================================
// SMTLIB-DRIVER.CC
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
#include <iostream>
using std::endl;
using std::cerr;

#include <cstdlib>  // for exit() and system()
#include <stdexcept>

#include <vector>
using std::vector;

#include "smtlib-driver.hh"

#include "node-utils.hh"

#include "formatter.hh"
#include "pprinter.hh"

using namespace z;


//==========================================================================
// Pretty print SMTLib abstract syntax
//==========================================================================

class SMTLibFormatter : public Formatter {
public:
    virtual Box& addSyntax(z::Kind k, const string& id,
                           vector<Box*>& bs);

    static Formatter* getFormatter();

private:

    static Formatter* instance;
    SMTLibFormatter();

    static set<string> reservedWords;

    string standardiseId(const string& s);
};

Box& SMTLibFormatter::addSyntax(z::Kind k, const std::string& id,
                              vector<Box*>& bs) {

    switch (k) {
    // Top level

    case(BENCHMARK):   return makeStringAp("benchmark " + id, bs);

    case(LOGIC):       return box(":logic " + id);
    case(EXTRASORTS):  return PP::makeHVSeq(":extrasorts ",
                                            "(",
                                            "",
                                            ")",
                                            bs);
    case(EXTRAFUNS):   return PP::makeHVSeq(":extrafuns ",
                                            "(",
                                            "",
                                            ")",
                                            bs);
    case(EXTRAPREDS):  return PP::makeHVSeq(":extrapreds ",
                                            "(",
                                            "",
                                            ")",
                                            bs);
    case(ASSUMPTION):     return box(":assumption") / *(bs.at(0));
    case(FORMULA):     return box(":formula") / *(bs.at(0));
    case(STATUS):      return box(":status " + id);

    case(COMMENT):     return box("; " + id);

    // Declarations

    case(DECL_FUN):    return makeStringAp(standardiseId(id), bs);
    case(DECL_PRED):   return makeStringAp(standardiseId(id), bs);

    // Sorts

    case(TYPE_ID):
    case(TCONST):      return box(standardiseId(id));
    case(INT_TY):      return box("Int");

    // Formulas

    case(OR):          return makeStringAp("or", bs);
    case(AND):         return makeStringAp("and", bs);
    case(NOT):         return makeStringAp("not", bs);
    case(IMPLIES):     return makeStringAp("implies", bs);
    case(IFF):         return makeStringAp("iff", bs);

    case(TRUE):        return box("true");
    case(FALSE):       return box("false");

    case(FORALL):      return makeStringAp("forall", bs);
    case(EXISTS):      return makeStringAp("exists", bs);
    case(PAT):         return PP::makeHVSeq(":pat {",
                                        "",
                                        "",
                                        "}",
                                        bs);

    case(PRED_AP):     {
        if (bs.size() == 0)
            return box(standardiseId(id));
        else
            return makeStringAp(standardiseId(id), bs);
    }
    case(EQ):          return makeStringAp("=", bs);
    case(DISTINCT):    return makeStringAp("distinct", bs);
   
        // SEQ is not quite prettiest: will indent vertical sequence
        // extra amount.

    case(TUPLE):
    case(SEQ):         return PP::makeHVSeq("",
                                            " ",
                                            "",
                                            "",
                                            bs);
    case(DECL):        return makeStringAp("?" + id, bs);


    // Terms

    case(FUN_AP):      return makeStringAp(standardiseId(id), bs);
    case(ITE):         return makeStringAp("ite", bs);
    case(IDIV):         return makeStringAp("div", bs);
    case(MOD):         return makeStringAp("mod", bs);
    case(CONST):       return box(standardiseId(id));
    case(VAR):         return box("?" + id);
    case(NATNUM):      return box(id);
    case(REALNUM):      return box(id + ".0");

    default:
	printMessage(ERRORm, "SMTLibFormatter::addSyntax: " + ENDLs
                     + "Encountered unsupported kind "
                     + kindString(k));
        return PP::makeHVSeq("***" + kindString(k) + "{" + id + "}***",
                             "[",
                             ";",
                             "]",
                             bs
                             );
    }
}


SMTLibFormatter::SMTLibFormatter() {

    // Generic reserved words

    reservedWords.insert("and");
    reservedWords.insert("benchmark");
    reservedWords.insert("distinct");
    reservedWords.insert("exists");
    reservedWords.insert("false");
    reservedWords.insert("flet");
    reservedWords.insert("forall");
    reservedWords.insert("if_then_else");
    reservedWords.insert("iff");
    reservedWords.insert("implies");
    reservedWords.insert("ite");
    reservedWords.insert("let");
    reservedWords.insert("logic");
    reservedWords.insert("not");
    reservedWords.insert("or");
    reservedWords.insert("sat");
    reservedWords.insert("theory");
    reservedWords.insert("true");
    reservedWords.insert("unknown");
    reservedWords.insert("unsat");
    reservedWords.insert("xor");

    // Symbols used in AUFLIA that we should avoid clashing with.

    reservedWords.insert("Int");
    reservedWords.insert("select");
    reservedWords.insert("store");

    // Keywords used in SMTLIB syntax
    // Yices and CVC3 are fussy about these being used as identifiers.
    reservedWords.insert("assumption");
    reservedWords.insert("axioms");
    reservedWords.insert("definition");
    reservedWords.insert("extensions");
    reservedWords.insert("formula");
    reservedWords.insert("funs");
    reservedWords.insert("extrafuns");
    reservedWords.insert("extrasorts");
    reservedWords.insert("extrapreds");
    reservedWords.insert("language");
    // Logic already covered previously
    // reservedWords.insert("logic");
    reservedWords.insert("notes");
    reservedWords.insert("preds");
    reservedWords.insert("sorts");
    reservedWords.insert("status");
    reservedWords.insert("theory");

    // Extra keywords accepted by Z3 in SMTLIB reading mode
    reservedWords.insert("div");
    reservedWords.insert("mod");
    reservedWords.insert("rem");

    // Extra keywords recognised by Yices in SMTLIB reading mode
    reservedWords.insert("real");
    reservedWords.insert("number");

};

// Differentiate id string from any of reserved words

string 
SMTLibFormatter::standardiseId(const string& s) {
    if (reservedWords.find(s) != reservedWords.end())
        return s + "'";
    else
        return s;
}

Formatter*
SMTLibFormatter::getFormatter() {
    if (instance == 0) instance = new SMTLibFormatter;
    return instance;
}

// Storage declarations for static class members.

Formatter* 
SMTLibFormatter::instance = 0;

set<string>
SMTLibFormatter::reservedWords;


//==========================================================================
// Alternate Simplify-format printer
//==========================================================================

class AltSimpFormatter : public Formatter {
public:
    virtual Box& addSyntax(z::Kind k, const string& id,
                           vector<Box*>& bs);

    static Formatter* getFormatter();

private:

    static Formatter* instance;
    AltSimpFormatter();

    static set<string> reservedWords;

    string standardiseId(const string& s);
};

Box& AltSimpFormatter::addSyntax(z::Kind k, const std::string& id,
                              vector<Box*>& bs) {

    switch (k) {
    // Top level

    case(BENCHMARK):   return PP::makeHVSeq("",
                                            "",
                                            "",
                                            "",
                                            bs);

    case(LOGIC):       return box("");
    case(EXTRAPREDS):  return PP::makeHVSeq("",
                                            "",
                                            "",
                                            "",
                                            bs);
    case(ASSUMPTION):  return makeStringAp("BG_PUSH",bs);
    case(FORMULA):     return *(bs.at(0));
    case(STATUS):      return box("");

    // Declarations

    case(DECL_FUN):    return box("");
    case(DECL_PRED):
        return PP::makeHVSeq("(DEFPRED (|" + standardiseId(id) + "|",
                                            " ",
                                            " ",
                                            "))",
                                            bs);
                                           

    // Sorts

    case(TCONST):      return box("");
    case(TYPE_ID):     return box("");
    case(INT_TY):      return box("Int");

    // Formulas

    case(OR):          return makeStringAp("OR", bs);
    case(AND):         return makeStringAp("AND", bs);
    case(NOT):         return makeStringAp("NOT", bs);
    case(IMPLIES):     return makeStringAp("IMPLIES", bs);
    case(IFF):         return makeStringAp("IFF", bs);

    case(TRUE):        return box("TRUE");
    case(FALSE):       return box("FALSE");

    case(FORALL):      return makeStringAp("FORALL", bs);
    case(EXISTS):      return makeStringAp("EXISTS", bs);

    case(PAT):         return makeStringAp("PATS", bs);


    case(PRED_AP):     
            return makeStringAp("|" + standardiseId(id) + "|", bs);

    case(EQ):          return makeStringAp("EQ", bs);
    case(DISTINCT):    return makeStringAp("DISTINCT", bs);
   
        // SEQ is not quite prettiest: will indent vertical sequence
        // extra amount.

    case(SEQ):         return PP::makeHVSeq("(",
                                            " ",
                                            " ",
                                            ")",
                                            bs);
    case(DECL):        return box("|?" + id + "|");


    // Terms

    case(FUN_AP):      return makeStringAp("|" + standardiseId(id) + "|", bs);
    case(ITE):         return makeStringAp("ite", bs);
    case(CONST):       return box("|" + standardiseId(id) + "|");
    case(VAR):         return box("|?" + id + "|");
    case(NATNUM):      return box(id);

    default:
	printMessage(ERRORm, "AltSimpFormatter::addSyntax: " + ENDLs
                     + "Encountered unsupported kind "
                     + kindString(k));
        return PP::makeHVSeq("***" + kindString(k) + "{" + id + "}***",
                             "[",
                             ";",
                             "]",
                             bs
                             );
    }
}


AltSimpFormatter::AltSimpFormatter() {

    reservedWords.insert("select");
    reservedWords.insert("store");
    reservedWords.insert("DISTINCT");
    reservedWords.insert("OR");
    reservedWords.insert("AND");
    reservedWords.insert("NOT");
    reservedWords.insert("IMPLIES");
    reservedWords.insert("IFF");
    reservedWords.insert("TRUE");
    reservedWords.insert("FALSE");
    reservedWords.insert("FORALL");
    reservedWords.insert("EXISTS");
    reservedWords.insert("EQ");
    reservedWords.insert("PATS");
    reservedWords.insert("ite");

};

// Differentiate id string from any of reserved words

string 
AltSimpFormatter::standardiseId(const string& s) {
    if (reservedWords.find(s) != reservedWords.end())
        return s + "'";
    else
        return s;
}

Formatter*
AltSimpFormatter::getFormatter() {
    if (instance == 0) instance = new AltSimpFormatter;
    return instance;
}

// Storage declarations for static class members.

Formatter* 
AltSimpFormatter::instance = 0;

set<string>
AltSimpFormatter::reservedWords;






/*
============================================================================
New SMTLIB Translator class
============================================================================
Cut down from old one.

So far, no fixes for Alt Simplify translation.
*/


class NewSMTLibTranslator : public Translator {
    
public:

    NewSMTLibTranslator() : Translator("NewSMTLib") {};

protected:
    virtual Node* translateAux (Node* oldN);
};



Node* 
NewSMTLibTranslator::translateAux (Node* oldN) {


/*
----------------------------------------------------------------------------
Top-level translation
----------------------------------------------------------------------------

Basic bottom up translation needs a little care to ensure some Node
children serving as extra parameters (e.g. types on RCD_* operators)
are not altered.

*/

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // top level for SMTLIB translation
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    if (oldN->kind == UNIT && optionVal("interface-mode") == "smtlib") {

        Node* decls = oldN->child(0);
        // Node* rules = oldN->child(1);
        // Node* goals = oldN->child(2);

        Node* extrafuns = new Node(EXTRAFUNS);
        Node* extrapreds = new Node(EXTRAPREDS);
        Node* extrasorts = new Node(EXTRASORTS);


        for (int i = 0; i != decls->arity(); i++) {
            Node* decl = decls->child(i);

            // decl == DECL_FUN{id} (SEQ t1 ... tn) t

            if (decl->kind == DECL_FUN) {

                if (decl->child(1)->kind == BOOL_TY) { 

                    // DECL_FUN{id} (SEQ t1 ... tn) BOOL_TY
                    // --> DECL_PRED{id} t1 .. tn
                
                    // Build result from SEQ node
                    Node* newDecl = decl->child(0);
                    newDecl->kind = DECL_PRED;
                    newDecl->id = decl->id;
                    extrapreds->addChild(newDecl);

                } else {  

                    // DECL_FUN{id} (SEQ t1 ... tn) t  {t != BOOL_TY}
                    // --> DECL_FUN{id} t1 .. tn t

                    // Build result from SEQ node
                    Node* newDecl = decl->child(0);
                    newDecl->kind = DECL_FUN;
                    newDecl->id = decl->id;
                    newDecl->addChild(decl->child(1));
                    extrafuns->addChild(newDecl);

                }
                
            }
            else if (decl->kind == DEF_CONST || decl->kind == DECL_VAR) {

                // Assume constant value never specified.
                // True of Examiner output.
                // Flag value specified as error.
                if (decl->arity() > 1) {
                    error("encountered DEF_CONST with value");
                    decl->popChild();
                }

                if (decl->child(0)->kind == BOOL_TY) {

                    // DEF_CONST{id} BOOL_TY  --> DECL_PRED{id'}
                    // DECL_VAR{id} BOOL_TY  --> DECL_PRED{id'}

                    decl->kind = DECL_PRED;
                    decl->popChild();
                    extrapreds->addChild(decl);

                } else {

                    // DEF_CONST{id} t  --> DECL_FUN{id} t
                    // DECL_VAR{id} t  --> DECL_FUN{id'} t
                    // t != BOOL_TY

                    decl->kind = DECL_FUN;
                    extrafuns->addChild(decl);

                }
            }

            else if (decl->kind == DEF_TYPE) {

                // DEF_TYPE{id} --> TCONST{id}
                // DEF_TYPE{id} T  -->

                // If T in {REAL_TY, INT_TY, TYPE_ID} just ignore
                // otherwise flag as error.

                if (decl->arity() == 0) {

                    extrasorts->addChild(new Node(TCONST, decl->id));
                    
                } else { // decl->arity() == 1

                    Node* type = decl->child(0);
                    if (! (type->kind == TYPE_ID
                           || type->kind == INT_TY
                           || type->kind == REAL_TY)) {

                        error("Unexpected type " + type->toShortString()
                              + " in type definition");
                    }
                }
            }

        } // END for loop over decls

        // Add in new declarations to decls


        decls->clearChildren();
        if (extrasorts->arity() > 0) decls->addChild(extrasorts);
        if (extrafuns->arity() > 0) decls->addChild(extrafuns);
        if (extrapreds->arity() > 0) decls->addChild(extrapreds);

        // Continue now with translation of lower levels.

    } // END if kind == UNIT && optionVal(interface-mode) is SMTLIB

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // top level for Simplify translation
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    else if (oldN->kind == UNIT) {
        // optionVal("interface-mode") == "simplify" 

        Node* decls = oldN->child(0);
        // Node* rules = oldN->child(1);
        // Node* goals = oldN->child(2);

        Node* extrapreds = new Node(EXTRAPREDS);

        for (int i = 0; i != decls->arity(); i++) {
            Node* decl = decls->child(i);

            if (decl->kind == DECL_FUN
                && decl->child(1)->kind == BOOL_TY) { 

                // DECL_FUN{id} (SEQ t1 ... tn) BOOL_TY
                // --> DECL_PRED{id} "v"1 ... "v"n
                // (Simplify prints DECL_PRED using DEFPRED)

                Node* pred = new Node(DECL_PRED, decl->id);

                int domainArity = decl->child(0)->arity();
                for (int i = 0; i != domainArity; i++) {
                    pred->addChild(new Node(CONST, "v" + intToString(i)));
                }
                extrapreds->addChild(pred);
            } 
            else if (decl->kind == DEF_CONST || decl->kind == DECL_VAR) {

                // Assume constant value never specified.
                // True of Examiner output.
                // Flag value specified as error.
                if (decl->arity() > 1) {
                    error("encountered DEF_CONST with value");
                    decl->popChild();
                }

                if (decl->child(0)->kind == BOOL_TY) {

                    // DEF_CONST{id} BOOL_TY  --> DECL_PRED{id'}
                    // DECL_VAR{id} BOOL_TY  --> DECL_PRED{id'}

                    decl->kind = DECL_PRED;
                    decl->popChild();
                    extrapreds->addChild(decl);
                }
            }

        } // End for loop over decls
        
        // Add in new declarations to decls

        decls->clearChildren();
        if (extrapreds->arity() > 0) decls->addChild(extrapreds);

        // Continue now with translation of lower levels.


    } // End if UNIT and simplify interface mode


/*
----------------------------------------------------------------------------
Translate subtrees of current node.
----------------------------------------------------------------------------
*/

    for (int i = 0; i != oldN->arity(); i++) {

        oldN->child(i) = translateAux(oldN->child(i));
    }
/*
----------------------------------------------------------------------------
Translate current node
----------------------------------------------------------------------------
*/


    switch (oldN->kind) {

/*
----------------------------------------------------------------------------
Formulas
----------------------------------------------------------------------------
*/

    case FORALL:
    case EXISTS: {
        if (optionVal("interface-mode") == "simplify"
            && oldN->arity() == 3) {
            Node* decls = oldN->child(0);
            Node* fmla = oldN->child(1);
            Node* pat = oldN->child(2);
            oldN->child(0) = decls;
            oldN->child(1) = pat;
            oldN->child(2) = fmla;
        }
        return oldN;
    }
    case PAT:
    case IMPLIES:
    case AND:
    case OR:
    case NOT:
    case IFF: 
    case EQ:
    case TRUE:
    case FALSE:
    case DISTINCT:
    case PRED_AP:
        return oldN;
    case NE: {
        oldN->kind = EQ;
        return new Node(NOT, oldN);
    }
    case I_LT: return oldN->updateKindAndId(PRED_AP, "<");
    case I_LE: return oldN->updateKindAndId(PRED_AP, "<=");

/*
----------------------------------------------------------------------------
Terms
----------------------------------------------------------------------------
*/  
    case I_PLUS:   return oldN->updateKindAndId(FUN_AP, "+");
    case I_MINUS:  return oldN->updateKindAndId(FUN_AP, "-");
    case I_TIMES:  return oldN->updateKindAndId(FUN_AP, "*");
    case I_UMINUS: {
        if (optionVal("interface-mode") == "simplify")
            return new Node(FUN_AP, "-",
                            new Node(NATNUM, "0"),
                            oldN->child(0));
        else
            return oldN->updateKindAndId(FUN_AP, "~");
    }

        // IDIV and MOD are treated specially by pretty-printing 

    case IDIV:   return oldN;
    case MOD:   return oldN;

        /*
        {

        // TO_REAL NATNUM{n}  -->  REALNUM{n}
        // TO_REAL (I_UMINUS NATNUM{n})  -->  R_UMINUS REALNUM{n}
        // TO_REAL e --> FUN_AP{i.to_real} e                      otherwise

        Node* intNode = oldN->child(0);
        bool isNegated = false;
        if (intNode->kind == NATNUM) {
            intNode->kind = REALNUM;
            return intNode;
        }
        else if (intNode->kind == I_UMINUS
                 && intNode->child(0)->kind == NATNUM) {
            intNode->kind = R_UMINUS;
            intNode->child(0)->kind = REALNUM;
            return intNode;
        }
        else {
            return oldN->updateKindAndId(FUN_AP, "i.to_real");
        }
    }
        */

    case CONST:
    case VAR:
    case NATNUM:
        return oldN;

    case DECL: return oldN;  // Assume only use of DECL is in quantifiers

    //  FUN_AP{<funid>} es  --> FUN_AP{<funid>} es      

    case FUN_AP: {
        return oldN;
    }
    case ITE: {
        return oldN;
    }

/*
----------------------------------------------------------------------------
Multipurpose
----------------------------------------------------------------------------
*/

    case TUPLE: return oldN;
    case SEQ: return oldN;
/*
----------------------------------------------------------------------------
Types
----------------------------------------------------------------------------
*/


    case TYPE_ID: 
    case INT_TY: return oldN;
        
    case REAL_TY: return new Node(TYPE_ID, "real");



/*
----------------------------------------------------------------------------
Top level structure with no further changes
----------------------------------------------------------------------------
*/
    case UNIT:
    case DECLS:
    case EXTRAFUNS:
    case DECL_FUN:
    case EXTRAPREDS:
    case DECL_PRED:
    case EXTRASORTS:
    case TCONST:
    case RULES:
    case RULE:
    case GOALS:
    case GOAL:
    case HYPS:
    case CONCLS:
        return oldN;


/*
----------------------------------------------------------------------------
Constructors without translation
----------------------------------------------------------------------------
*/


    case ENUM_TY:
    case SUBRANGE_TY:
    case RECORD_TY: 
    case ARRAY_TY: 
    case BIT_TY: 
    case BOOL_TY:

        
    case LE:
    case GE:


    default: {
            error ("unrecognised kind "
                   + kindString(oldN->kind)
                   );
            return oldN;
        }
    } // end switch(oldN->kind)


}




//========================================================================
// Virtual functions for SMTLIBDriver class
//========================================================================


Node* 
SMTLibDriver::translateUnit(Node* unit) {

    NewSMTLibTranslator t;
    return t.translate(unit);
}


// Create a valid SMTLib identifier from argument.

// Replaces all unacceptable characters with '.'s.
// Not concerned here with ensuring function is injective.

bool isSMTLibIdChar(char c) {
    return
        ('A' <= c && c <= 'Z')
        || 
        ('a' <= c && c <= 'z')
        || 
        ('0' <= c && c <= '9')
        ||
        c == '.'
        ||
        c == '\''
        ||
        c == '_';
}

string mkSMTLibId(const string& s) {
    string result;
    for (int i = 0; i != (int) s.size(); i++) {
        char c = s.at(i);
        if (isSMTLibIdChar(c))
            result.append(1,c);
        else
            result.append(1,'.');
    }
    return result;
}


//---------------------------------------------------------------------------
// initGoal
//---------------------------------------------------------------------------

void
SMTLibDriver::initGoal(const string& unitName,
                         int goalNum,
                         int conclNum) {

    string fullGoalFileRoot
        = getFullGoalFileRoot("smtsolver", unitName, goalNum, conclNum);

    if (optionVal("interface-mode") == "simplify") {
        solverInputFileName = fullGoalFileRoot  + ".smp";
        solverOutputFileName = fullGoalFileRoot  + ".outsmp";
        solverErrorFileName = fullGoalFileRoot  + ".errsmp";
    }
    else {
        solverInputFileName = fullGoalFileRoot  + ".smt";
        solverOutputFileName = fullGoalFileRoot  + ".out";
        solverErrorFileName = fullGoalFileRoot  + ".err";
    }


    string benchname = mkSMTLibId(unitName) + "." + intToString(goalNum);
    if (conclNum != 0) benchname += "." + intToString(conclNum);

    string logic(option("logic") ? optionVal("logic") : "AUFLIA"); 

    benchmark = new Node(BENCHMARK, benchname, new Node(LOGIC,logic));
    formula = new Node(SEQ);
    return;
}

//---------------------------------------------------------------------------
// addDecl
//---------------------------------------------------------------------------
void 
SMTLibDriver::addDecl(Node* decl) {
    benchmark->addChild(decl);
}

//---------------------------------------------------------------------------
// addRule
//---------------------------------------------------------------------------

void
SMTLibDriver::addRule(Node* hyp, const string& hId, string& remarks) {
    if (option("add-formula-descriptions")) {
        benchmark->addChild(new Node(COMMENT, hId));
    }
    benchmark->addChild(new Node(ASSUMPTION,hyp));
}

//---------------------------------------------------------------------------
// addHyp
//---------------------------------------------------------------------------

void
SMTLibDriver::addHyp(Node* hyp, const string& hId, string& remarks) {
    if (option("smtlib-hyps-as-assums")) {
        if (option("add-formula-descriptions")) {
            benchmark->addChild(new Node(COMMENT, hId));
        }
        benchmark->addChild(new Node(ASSUMPTION,hyp));
    }
    else
        formula->addChild(hyp);
}

//---------------------------------------------------------------------------
// addConcl
//---------------------------------------------------------------------------

void
SMTLibDriver::addConcl(Node* concl, string& remarks) {
    if (optionVal("interface-mode") == "simplify") {
        formula->addChild(concl);
    }
    else {
        formula->addChild(new Node(NOT, concl));
    }
}

//---------------------------------------------------------------------------
// check
//---------------------------------------------------------------------------
// Dummy method to make new alternative driver happy

SMTDriver::Status
SMTLibDriver::check(string& remarks) {
    return UNCHECKED;
}

//---------------------------------------------------------------------------
// finishSetup
//---------------------------------------------------------------------------

void
SMTLibDriver::finishSetup() {
    Node* goal;
    if (formula->arity() == 0) {
        goal = new Node(z::TRUE);
    }
    else if (formula->arity() == 1) {
        goal = formula->child(0);
    }
    else {
        goal = formula;
        goal->kind = AND;
    }
    if (option("add-formula-descriptions")) {
        if (option("smtlib-hyps-as-assums")) {
            benchmark->addChild(new Node(COMMENT, "C"));
        } else {
            benchmark->addChild(new Node(COMMENT, "Hs and C"));
        }
    }
    benchmark->addChild(new Node(FORMULA,goal));
    benchmark->addChild(new Node(STATUS,"unknown"));


    ofstream solverInput;
    solverInput.open(solverInputFileName.c_str());
    if (solverInput.fail()) {
        cerr << endl
             << "Error on trying to open file " << solverInputFileName << endl;
        exit(1);
    }

    if (optionVal("interface-mode") == "simplify")
        Formatter::setFormatter(AltSimpFormatter::getFormatter());
    else
        Formatter::setFormatter(SMTLibFormatter::getFormatter());

    solverInput << *benchmark << endl;
    solverInput.close();
}
//---------------------------------------------------------------------------
// outputQuerySet
//---------------------------------------------------------------------------
// For new alternative SMT driver
void
SMTLibDriver::outputQuerySet() {
    finishSetup();
    return;
}


//---------------------------------------------------------------------------
// checkGoal
//---------------------------------------------------------------------------

bool
SMTLibDriver::checkGoal(string& remarks) {

    string cmd;

    if (! (option("prover") || option("prover-command") )) {
        return false;
    }
        
    if (option("prover-command")) {
            
        cmd = optionVal("prover-command") + " ";
    }

    // Must be case now that prover option set

    else if (optionVal("prover") == "yices") {

        cmd = "yices -smt ";

        // timeout value is timeout in sec.
        if (option("timeout"))
            cmd += "--timeout=" + optionVal("timeout") + " ";
    }
    else if (optionVal("prover") == "z3") {

        if (optionVal("interface-mode") == "simplify")
            cmd = "z3 -s ";
        else
            cmd = "z3 -smt ";

        if (option("z3-fourier-motzkin"))
            cmd += "FOURIER_MOTZKIN_ELIM=true ";
        if (option("timeout"))
            // Was not supported in Z3 v1.3 Linux. 
            cmd += "SOFT_TIMEOUT=" + optionVal("timeout") + " ";
    }
    else if (optionVal("prover") == "cvc3") {

        cmd = "cvc3 -lang smt ";
        if (option("timeout"))
            cmd += "-timeout " + optionVal("timeout") + " ";
        if (option("resourcelimit"))
            cmd += "-resource " + optionVal("resourcelimit") + " ";
    }
    else if (optionVal("prover") == "simplify") {

        // -nosc = No satisfying conjunctions (for counterexamples)
        // Causes simplify to return just "valid" or "invalid".  
        // See hpl/simplify/src/Simplify.1.html

        cmd = "simplify -nosc ";
    }
    else {
        printMessage(ERRORm, "Unrecognised prover option: "
                     + optionVal("prover") + ENDLs);
        return false;
    }
    
    cmd = withTimeoutAndIO(cmd,
                           solverInputFileName,
                           solverOutputFileName,
                           solverErrorFileName);

    if (option("doublerun")) cmd = cmd + " ; " + cmd;

    printMessage(INFOm, "Running command" + ENDLs
                             + cmd + ENDLs);

    exitStatus = std::system(cmd.c_str());

    // For SMT mode, exit status is not reliable guide for something going
    // wrong.  z3 -smt returns non-zero status when result is unknown.

    printMessage(INFOm, "Exit status is " + intToString(exitStatus));

    return false;
}

//---------------------------------------------------------------------------
// runQuerySet
//---------------------------------------------------------------------------

bool 
SMTLibDriver::runQuerySet(string& remarks) {
    return checkGoal(remarks);
}

//---------------------------------------------------------------------------
// getResults
//---------------------------------------------------------------------------

SMTLibDriver::Status
SMTLibDriver::getResults(string& remarks) {

    // Do not check output files if none were generated in first place

    if (! (option("prover") || option("prover-command") )) {
        return UNKNOWN;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Read in output and error files from solver
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    ifstream solverOut (solverOutputFileName.c_str() );
    ifstream solverErr (solverErrorFileName.c_str() );

    if (!solverOut) {
        printMessage(ERRORm, "Cannot open output file "
                             + solverOutputFileName);
        appendCommaString(remarks, "Outfile not found");
        return ERROR;
    }

    if (!solverErr) {
        printMessage(ERRORm, "Cannot open error output file "
                             + solverErrorFileName);
        appendCommaString(remarks, "Errorfile not found");
        return ERROR;
    }

    vector<string> solverOutput;
    vector<string> solverErrOutput;
    {
        string line;

        while (getline(solverOut, line)) solverOutput.push_back(line);
        solverOut.close();

        while (getline(solverErr, line)) solverErrOutput.push_back(line);
        solverErr.close();
    }

    // flags for processing of standard error file

    bool seenTimeout = false;
    bool seenWarning = false;
    bool seenUnexpectedErrOutput = false;

    // flags for processing of standard output file

    bool seenSatOutput = false;
    bool seenUnsatOutput = false;
    bool seenUnknownOutput = false;
    bool seenUnexpectedOutput = false;

    if (! (optionVal("interface-mode") == "simplify")) {

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // interface-mode == smtlib
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // 

        // First inspect error output.  Check for
        // - warning messages than can be logged
        // - Timeout termination messages
        //

        for (int i = 0; i != (int) solverErrOutput.size(); i++) {

            string s = solverErrOutput.at(i);

            if (tokeniseString(s).size() == 0) {
                continue;
            }
                
            // Detect message produced by ulimit -t killing process
            
            // See below for alternate method of detecting this timeout.
            // Approach here works on Scientific Linux 5.3 but not 
            // Ubuntu 10.4.2.

            if (hasPrefix(s, "sh: line 1:") && hasSubstring(s, "Killed")) {


                // appendCommaString(remarks, "timeout (ulim)");
                // printMessage(INFOm,
                //              "SMTLib solver reached ulimit time limit");
                // seenTimeout = true;
                continue;
            }

            // Detect message produced by ulimit -v killing process

            if (hasPrefix(s, "Fatal error: out of memory.")) {

                    appendCommaString(remarks, "stacklimit (ulim)");
                    printMessage(INFOm,
                                 "SMTLib solver reached ulimit stackspace limit");
		    // We treat this the same as a "normal" timeout
                    seenTimeout = true;
                    continue;
            }

            // Detect message from timeout.sh script
            // Script outputs 
            // ./timeout.sh: line 37: 22246 Terminated              sleep $timeout
            // When command executed has error return value. 
            // Seems z3 has error return value on unsat!

            if (hasPrefix(s, "./timeout.sh: line 37")
                && hasSubstring(s, "Terminated")) {

                    continue;
            }
  
            // cvc3 outputs on kill (because of shell timeout)
            // Interrupted by signal 15.  /home/pbj/smt/cvc3/r2.1/bin/cvc3 is aborting.
            if (hasPrefix(s, "Interrupted by signal 15.")) {
                // Just ignore this.  Should also see "timeout" on stdout
                continue;
            }

            // cvc3 outputs on stderr on self timeout
            // Interrupted by signal 14 (self-timeout).  /home/pbj/smt/cvc3/r2.2/bin/cvc3 is aborting.
            // sh: line 1: 23491 Aborted                 /home/pbj/smt/cvc3/r2.2/bin/cvc3 -lang smt -timeout 1 /tmp/smtsolver-vulcan.inf.ed.ac.uk-23485.smt


            if (hasPrefix(s, "Interrupted by signal 14 (self-timeout).")) {
                    appendCommaString(remarks, "timeout (cvc3)");
                    printMessage(INFOm,
                                 "SMTLib solver had self timeout");
                    seenTimeout = true;
                    continue;
            }
            if (hasPrefix(s, "sh: line 1:") && hasSubstring(s, "Aborted")) {
                continue;
            }

            // z3 at least uses this. 
            if (hasPrefix(s,"WARNING:")) {

                seenWarning = true;
                printMessage(WARNINGm, "Warning message from SMTLib solver"
                             + ENDLs + s);
                continue;
            }
            seenUnexpectedErrOutput = true;
        }
        if (seenWarning) appendCommaString(remarks, "warning(s)");

        // Allow for multiple lines: Alt-Ergo sometimes outputs
        // multiple lines of unsats when unsat discovered on
        // assumption assertion, it seeems

        for (int i = 0; i != (int) solverOutput.size(); i++) {

            string s = solverOutput.at(i);

            if (tokeniseString(s).size() == 0) {
                continue;
            }
            
            if (s == "unsat") {
                seenUnsatOutput = true;
            }
            else if (s == "sat") {
                seenSatOutput = true;
                appendCommaString(remarks, "sat");
            }
            // "unknown (sat)" is output occasionally by Alt-Ergo
            else if (s == "unknown" || s == "unknown (sat)") {
                appendCommaString(remarks, "unknown");
                seenUnknownOutput = true;
            }
            // Timeout message from timeout.sh
            else if (s == "timeout") {
                appendCommaString(remarks, "timeout (sh2)");
                printMessage(INFOm,
                             "SMTLib solver reached timeout.sh time limit");
                seenTimeout = true;
            }
            // Proof steps limit message from Alt-Ergo
            else if (hasPrefix(s, "Steps limit reached")) {
                appendCommaString(remarks, "steps limit reached");
                printMessage(INFOm,
                             "Alt-Ergo proof steps limit reached");
                seenTimeout = true;
	    } 
	    else {
                seenUnexpectedOutput = true;
            }
            
            // Who will write out this?  Is this in competition?

            // else if (solverOutput.at(0) == "timeout") {
            //    appendCommaString(remarks, "timeout");
            //    status = UNPROVEN;
            // }
        }

    }
    else { 

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // interface-mode == simplify
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

        // Sometimes see 
        //   - "Error"
        //   - "ASSERT failed"
        //   - "Bad input:"     on stdout
        //   - "Warning:"

        //   - "Counterexample:" on stdout (with Z3 -s)

        // When it's clear which output stream these are seen on, can add
        // tests for them.

        for (int i = 0; i != (int) solverErrOutput.size(); i++) {

            string s = solverErrOutput.at(i);
                
            // Detect message produced by ulimit -t killing process

            if (hasPrefix(s, "sh: line 1:") && hasSubstring(s, "Killed")) {

                appendCommaString(remarks, "timeout");
                printMessage(INFOm,
                             "Solver reached ulimit time limit");
                seenTimeout = true;
                continue;
            }

            if (hasSubstring(s, "ASSERT failed")) {

                appendCommaString(remarks, "ASSERT failed");
                printMessage(INFOm, "Simplify ASSERT failed" + ENDLs);

            }

            if (tokeniseString(s).size() > 0) {
                seenUnexpectedErrOutput = true;
            }
        }


        for (int i = 0; i != (int) solverOutput.size(); i++) {
            string s = solverOutput.at(i);

            // Expect <nn>: Valid.

            if (hasSubstring(s,"Valid.")) {
                seenUnsatOutput = true;
            }
            else if (hasSubstring(s,"Invalid.")) {
                seenSatOutput = true;
            }
            else if (hasSubstring(s,"Counterexample:")) {
                continue;
            }
            else if (tokeniseString(s).size() > 0) {
                seenUnexpectedOutput = true;
            }
        }

    }

    //  Detecting termination signals, not on windows platform:
#ifndef _WIN32

    // Code here discovered by trial and error and reading man pages.

    // Man page for getrlimit makes clear that a process is terminated using
    // a KILL signal when the time limit is reached.
    //
    // system(3) man page says that return status of system call is in format 
    // specified on wait(2) man page.

    // wait(2) man page suggests using WIFSIGNALED(exitStatus) to
    // detect if process terminated by signal, and WTERMSIG to extract
    // this signal.

    // This doesn't work.  Instead, even on exit because of signal, 
    // are seeing WIFEXITED() returning 1 (true).  Get this even without
    // running in sub-process with (). (see above).

    // With timeout, are seeing exitStatus == 35072 and 
    // WEXITSTATUS(exitStatus) == 137. (As wait(2) man page remarks, 
    // WEXITSTATUS picks out bits 15-8 of argument)

    // Some web pages remark on exit codes of form 128 + signumber being used
    // to flag processes terminated by signals.  SIGKILL has value 9, hence
    // this 137.

    // Can see this 137 by e.g. doing:
    //    ulimit -t 1 ; z3 -smt x.smt ; echo $?

    /*    printMessageWithHeader
        ("DEBUG",
         "exitStatus = " + intToString(exitStatus) + ENDLs + 
         "WIFSIGNALED = " + intToString(WIFSIGNALED(exitStatus)) + ENDLs + 
         "WTERMSIG = " + intToString(WTERMSIG(exitStatus)) + ENDLs +
         "WIFEXITED = " + intToString(WIFEXITED(exitStatus)) + ENDLs + 
         "WEXITSTATUS = " + intToString(WEXITSTATUS(exitStatus))
         );

    */    

    if (WIFEXITED(exitStatus)) {
        int exitCode = WEXITSTATUS(exitStatus);
        if (exitCode == 128 + SIGKILL) {
            appendCommaString(remarks, "timeout (exit code 137)");
            printMessage(INFOm,
                         "Solver killed.  Assume ulimit time limit reached.");
            seenTimeout = true;
        }
        else if (exitCode != EXIT_SUCCESS) { // EXIT_SUCCESS == 0 usually
            appendCommaString(remarks, "exit code " + intToString(exitCode));
        }
    }
    else if (WIFSIGNALED(exitStatus)) {
        printMessage(WARNINGm,
                     "Subprocess termination on signal "
                     + intToString(WTERMSIG(exitStatus)));
    }
    else {
        printMessage(WARNINGm,
                     "Unexpected subprocess exit status "
                     + intToString(exitStatus));
    }
#endif

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Report on output and decide return status
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    if (seenUnexpectedErrOutput || seenUnexpectedOutput) {

        string outMessage = concatStrings(solverOutput, ENDLs);
        string errMessage = concatStrings(solverErrOutput, ENDLs);

        printMessage(ERRORm, 
                     "Error(s) on prover output." + ENDLs
                     + "On STDOUT:" + ENDLs
                     + outMessage + ENDLs
                     + "On STDERR: " + ENDLs
                     + errMessage + ENDLs
                     + "END of output");
        return ERROR;
    }

    // All output expected.

    if (seenTimeout)
        return RESOURCE_LIMIT;

    if (seenSatOutput)
        return FALSE;

    if (seenUnknownOutput)
        return UNKNOWN;

    if (seenUnsatOutput)
        return TRUE;

    // Both outputs empty.

    printMessage(ERRORm,
                 "Prover standard output and error output both empty");
    return ERROR;


}

//---------------------------------------------------------------------------
// getRunResults()
//---------------------------------------------------------------------------

vector<SMTDriver::QueryStatus>
SMTLibDriver::getRunResults(int numQueries) {

    if (numQueries > 1) {
       printMessage(ERRORm, "Multiple results sought from prover without"
         " support for incrementality\n");
    }

    string remarks;
    Status status = getResults(remarks);

    vector<SMTDriver::QueryStatus> result;
    result.push_back(QueryStatus(status,remarks,0.0));
    return result;
}

//---------------------------------------------------------------------------
// finaliseGoal()
//---------------------------------------------------------------------------

void
SMTLibDriver::finaliseGoal() {

    if (option("delete-working-files")) {

        tryRemoveFile(solverInputFileName);
        tryRemoveFile(solverOutputFileName);
        tryRemoveFile(solverErrorFileName);
    }
    return;

    
}


//=========================================================================
// END OF FILE
//=========================================================================
