//==========================================================================
//==========================================================================
// ISAB-DRIVER.CC
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


// TO FIX:
// * standardising apart of function, constant, type and record fieldname ids 
//   each other and Isabelle's reserved words.

//==========================================================================
#include <iostream>
using std::endl;
using std::cerr;

#include <cstdlib>  // for exit() and system()
#include <stdexcept>

#include <vector>
using std::vector;

#include "isab-driver.hh"

#include "node-utils.hh"

#include "formatter.hh"
#include "pprinter.hh"

using namespace z;


//==========================================================================
// Pretty print Isabelle abstract syntax
//==========================================================================

class IsabFormatter : public Formatter {
public:
    virtual Box& addSyntax(z::Kind k, const string& id,
                           vector<Box*>& bs);

    static Formatter* getFormatter();

private:

    static Formatter* instance;
    IsabFormatter();

    static set<string> reservedWords;

    string standardiseId(const string& s);
};




Box& IsabFormatter::addSyntax(z::Kind k, const std::string& id,
                              vector<Box*>& bs) {

    switch (k) {

    // Top level
    //----------

    case(THEORY): {
        Box* decls = bs.at(0);
        Box* hyps = bs.at(1);
        Box* concls = bs.at(2);
        return
            box("theory " + id)
            / box("imports Real")
            / box("")
            / box("begin")
            / box("")
            / *decls
            / box("")
            / box("lemma L:")
            / box("")
            / *hyps
            / box("")
            / *concls
            / box("")
            / box(option("proof-command")
                  ? optionVal("proof-command")
                  : "oops")
            / box("")
            / box("end");
    }

    case(DECLS): return PP::simpleVSeq(bs);
    case(HYPS):  return PP::simpleVSeq(bs);
    case(ASSUMPTION): return PP::makeHVSeq("assumes " + id + ": ",
                                           "\"",
                                           "",
                                           "\"",
                                           bs);
        
    case(CONCLS): return PP::makeHVSeq("",
                                      "shows",
                                      "  and",
                                      "",
                                      bs);

    case(CONCL): return PP::makeHVSeq("",
                                     "\"",
                                     "",
                                     "\" (is \"?" + id + "\")",
                                     bs);


    // Declarations
    //-------------
    case(DEF_TYPE):   return PP::makeHVSeq("types " + id + " = ",
                                          "\"",
                                          "",
                                          "\"",
                                          bs);

    case(DECL_TYPE):  return box("typedecl " + id);

    case(DEF_RECORD): return PP::makeHVSeq("record " + id + " = ",
                                          "",
                                          "",
                                          "",
                                          bs);

    case(OUTER_DECL): return PP::makeHVSeq(id + " :: ",
                                          "\"",
                                          "",
                                          "\"",
                                          bs);

    case(DECL_CONST): return PP::makeHVSeq("consts " + id + " :: ",
                                          "\"",
                                          "",
                                          "\"",
                                          bs); 


    // Types
    // -----

    case(INT_TY):      return box("int");
    case(REAL_TY):      return box("real");
    case(BOOL_TY):      return box("bool");

    case(FUN_TY):
        return PP::makeHVSeq("", "", "=>", "", bs);

    case(FUN_ARG_TY):
        return PP::makeHVSeq("", "[", ",", "]", bs);

    case(TUPLE_TY):
        return PP::makeHVSeq("", "", "*", "", bs);

    case(TYPE_ID):       return box(id);




    // Logic
    // -----

    case(IMPLIES):  return PP::makeHVSeq("", "", "-->", "", bs);
    case(IFF):      return PP::makeHVSeq("", "", "<->", "", bs);
    case(OR):       return PP::makeHVSeq("", "", "|", "", bs);
    case(AND):      return PP::makeHVSeq("", "", "&", "", bs);
    case(NOT):      return PP::makeHVSeq("", "~", "", "", bs);

    case(TRUE):     return box("True");
    case(FALSE):    return box("False");

    case(FORALL):   return PP::makeHVSeq("", "ALL", ". ", "", bs);
    case(EXISTS):   return PP::makeHVSeq("", "EX", ". ", "", bs);

    case(SEQ):      return PP::makeHVSeq("", "", "", "", bs);
    case(DECL):     return PP::makeHVSeq(id + " ::"  , "", "", "", bs);

    // Polymorphic terms
    // -----------------

    case(VAR):      return box(id);
    case(CONST):    return box(id);

    case(FUN_AP):   return PP::makeHVSeq(id, "", "", "", bs);
    case(APPLY):    return PP::makeHVSeq("", "", "", "", bs);
    case(EQ):       return PP::makeHVSeq("", "", "=", "", bs);

    // pairs and tuples
    // ----------------

    case(TUPLE):    return PP::makeHVSeq("", "(", ",", ")", bs);

    // integers
    // --------

    case(I_LE):     return PP::makeHVSeq("", "", "<=", "", bs);
    case(I_LT):     return PP::makeHVSeq("", "", "<", "", bs);

    case(I_PLUS):   return PP::makeHVSeq("", "", "+", "", bs);
    case(I_MINUS):  return PP::makeHVSeq("", "", "-", "", bs);
    case(I_TIMES):  return PP::makeHVSeq("", "", "*", "", bs);
    case(IDIVM):    return PP::makeHVSeq("", "", "div", "", bs);
    case(MOD):      return PP::makeHVSeq("", "", "mod", "", bs);

    case(I_UMINUS): return PP::makeHVSeq("", "-", "", "", bs);

    case(I_EXP_N):  return PP::makeHVSeq("", "", "^", "", bs);
    case(I_TO_NAT): return PP::makeHVSeq("", "nat", "", "", bs);


    case(NATNUM):
        return box("(" + id + " :: int)");

    // reals
    // -----

    case(TO_REAL):  return PP::makeHVSeq("(of_int(", "", "", ") :: real)", bs);

    case(R_LE):     return PP::makeHVSeq("", "", "<=", "", bs);
    case(R_LT):     return PP::makeHVSeq("", "", "<", "", bs);

    case(R_PLUS):   return PP::makeHVSeq("", "", "+", "", bs);
    case(R_MINUS):  return PP::makeHVSeq("", "", "-", "", bs);
    case(R_TIMES):  return PP::makeHVSeq("", "", "*", "", bs);
    case(RDIV):     return PP::makeHVSeq("", "", "/", "", bs);

    case(R_UMINUS): return PP::makeHVSeq("", "-", "", "", bs);

    case(R_EXP_N):  return PP::makeHVSeq("", "", "^", "", bs);

    // arrays
    // -------

    // UPDATE formatting:

    // If doesn't fit on one line, go for
    // [b0] 
    // ( [b1]
    //   := [b2]
    // )

    case(UPDATE): {
        std::vector<Box*> newBs;

        newBs.push_back( bs[0] );
        newBs.push_back( &(box("( ") + *(bs[1])));
        newBs.push_back( &(box("  := ") + *(bs[2])));
        newBs.push_back( & box(")") );

        return PP::simpleHVSeq(newBs);
    }

    // records
    // -------
    case(RCD_ELEMENT): return PP::makeHVSeq(id, "", "", "", bs);

    // RCD_UPDATE

    // If doesn't fit on one line, go for
    // [b0]
    // (| <id>
    //    := [b2]
    // |)

    case(RCD_UPDATE): {
        std::vector<Box*> newBs;

        newBs.push_back( bs[0] );
        newBs.push_back( &(box("(| " + id) ) );
        newBs.push_back( &(box("   := ") + *(bs[1])));
        newBs.push_back( & box("|)") );

        return PP::simpleHVSeq(newBs);
    }

    // Otherwise
    // ---------

    default:
	printMessage(ERRORm, "IsabFormatter::addSyntax: " + ENDLs
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


IsabFormatter::IsabFormatter() {

    addOpInfo(FUN_TY, 0, "LE");    // Varying arity expected
    addOpInfo(TUPLE_TY, 0, "LE");  // Varying arity expected

    addOpInfo(IMPLIES, 25, "LE");
    addOpInfo(IFF, 25, "LE");
    addOpInfo(OR,  30, "LE");    // Varying arity expected
    addOpInfo(AND, 35, "LE");   // Varying arity expected
    addOpInfo(NOT, 40, "E");

    addOpInfo(EXISTS, 10, "*E");
    addOpInfo(FORALL, 10, "*E");

    addOpInfo(SEQ, 0, "L");      // Varying arity expected
    addOpInfo(DECL, 0, "E");

    addOpInfo(FUN_AP, 1000, "L");  // Varying arity expected
    addOpInfo(APPLY, 1000, "L");  // Varying arity expected
    addOpInfo(EQ, 50, "LL");

    // integers

    addOpInfo(I_LE, 50, "LL");
    addOpInfo(I_LT, 50, "LL");

    addOpInfo(I_PLUS, 65, "EL");
    addOpInfo(I_MINUS, 65, "EL");

    addOpInfo(I_TIMES, 70, "EL");
    addOpInfo(IDIVM, 70, "EL");
    addOpInfo(MOD, 70, "EL");

    addOpInfo(I_UMINUS, 80, "L");

    addOpInfo(I_EXP_N, 80, "LE");
    addOpInfo(I_TO_NAT, 1000, "L");


    // reals

    addOpInfo(R_LE, 50, "LL");
    addOpInfo(R_LT, 50, "LL");

    addOpInfo(R_PLUS, 65, "EL");
    addOpInfo(R_MINUS, 65, "EL");

    addOpInfo(R_TIMES, 70, "EL");
    addOpInfo(RDIV, 70, "EL");
    addOpInfo(MOD, 70, "EL");

    addOpInfo(R_UMINUS, 80, "L");

    addOpInfo(R_EXP_N, 80, "LE");

    // Arrays
    addOpInfo(UPDATE, 1000, "L**");

    // Records
    addOpInfo(RCD_ELEMENT, 1000, "L");
    addOpInfo(RCD_UPDATE, 900, "E**");

    return;
};




Formatter*
IsabFormatter::getFormatter() {
    if (instance == 0) instance = new IsabFormatter;
    return instance;
}

// Storage declarations for static class members.

Formatter* 
IsabFormatter::instance = 0;



/*
============================================================================
FDL to Isabelle/HOL abstract syntax translation
============================================================================
*/


class IsabTranslator : public Translator {
    
public:

    IsabTranslator() : Translator("Isabelle/HOL") {};

protected:
    virtual Node* translateAux (Node* oldN);

private:
    string fixFunId(const string& s);
    string fixConstId(const string& s);
    string fixRcdFldId(const string& id, const string& typeId);
    string fixTypeId(const string& id);

};

string 
IsabTranslator::fixFunId(const string& s) {
    return s + "'";
}
string 
IsabTranslator::fixConstId(const string& s) {
    return s + "''";
}
string 
IsabTranslator::fixRcdFldId(const string& id, const string& typeId) {
    return id + "'" + typeId ;
}
 
string 
IsabTranslator::fixTypeId(const string& id) {
    return id + "'";
}



Node* 
IsabTranslator::translateAux (Node* oldN) {



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
Top-level structure of unit
----------------------------------------------------------------------------
*/
    case UNIT:
    case DECLS:
    case RULES:
    case GOALS:
    case GOAL:
    case HYPS:
    case CONCLS:
        return oldN;

/*
----------------------------------------------------------------------------
Declarations and definitions
----------------------------------------------------------------------------
*/


    case DEF_TYPE: {
        Node* newN;
        if (oldN->arity() == 0) {
            newN = oldN->updateKind(DECL_TYPE);
        }
        else if (oldN->child(0)->kind == RECORD_TY) {
            newN = oldN->child(0);
            newN->kind = DEF_RECORD;
            for (int i = 0; i != newN->arity(); i++) {
                Node* decl = newN->child(i);
                decl->kind = OUTER_DECL;
                decl->id = fixRcdFldId(decl->id,oldN->id);
            }
        }
        else {
            newN = oldN;
        }
        return newN->updateId(fixTypeId(oldN->id));
    }

    case DEF_CONST: {
        oldN->id = fixConstId(oldN->id);
        if (oldN->arity() == 1) 
            return oldN->updateKind(DECL_CONST);
        else
            return oldN;
    }
    case DECL_FUN: {
        Node* arg_tys = oldN->child(0);
        if (arg_tys->arity() == 1) {
            arg_tys = arg_tys->child(0);
        } else {
            arg_tys->kind = FUN_ARG_TY;
        }
        Node* range_ty = oldN->child(1);
        oldN->kind = DECL_CONST;
        oldN->id = fixFunId(oldN->id);
        oldN->popChild();
        oldN->child(0) = new Node(FUN_TY, arg_tys, range_ty);
        return oldN;
    }



/*
----------------------------------------------------------------------------
Types
----------------------------------------------------------------------------
*/

    case ARRAY_TY: {
        oldN->kind = FUN_TY;
        oldN->id = "";
        if (oldN->child(0)->arity() == 1) {
            oldN->child(0) = oldN->child(0)->child(0);
        } else {
            oldN->child(0)->kind = TUPLE_TY;
        }
        return oldN;
    }

    case INT_TY: 
    case REAL_TY:
    case BOOL_TY:
        return oldN;
    case TYPE_ID: 
        return oldN->updateId(fixTypeId(oldN->id));

    case TYPE_PARAM: // Used for type args of record operators
        return oldN; // Don't fix.

    // Node kinds eliminated later by translation of parent node
    case RECORD_TY: return oldN;

    // Not expecting ENUM_TY, SUBRANGE_TY, BIT_TY

/*
----------------------------------------------------------------------------
Formulas
----------------------------------------------------------------------------
*/

    case IMPLIES:
    case IFF: 
    case OR:
    case AND:
    case NOT:

    case TRUE:
    case FALSE:
        return oldN;

    case FORALL:
    case EXISTS: {
        Node* decls = oldN->child(0);
        for (int i = 0; i != decls->arity(); i++) {
            Node* decl = decls->child(i);
            decl->id = fixConstId(decl->id);
        }
    }

    case SEQ:
    case DECL:
        return oldN;

    case CONST:
    case VAR:
        return oldN->updateId(fixConstId(oldN->id));

    case FUN_AP:
        return oldN->updateId(fixFunId(oldN->id));

    case EQ:
        return oldN;

/*
----------------------------------------------------------------------------
Integers
----------------------------------------------------------------------------
*/

    case I_LT:
    case I_LE:
   
    case I_PLUS:
    case I_MINUS:
    case I_TIMES:
        return oldN;
    case IDIV: return oldN->updateKind(IDIVM); // FIXME: Unsound!!

    case MOD:
    case I_UMINUS: 
        return oldN;

    case I_EXP: {
        oldN->child(1) = new Node(I_TO_NAT, oldN->child(1));
        return oldN->updateKind(I_EXP_N);
    }

    case NATNUM:
        return oldN;

/*
----------------------------------------------------------------------------
reals
----------------------------------------------------------------------------
*/
    case TO_REAL:

    case R_LE:
    case R_LT:
   
    case R_PLUS:
    case R_MINUS:
    case R_TIMES:
    case RDIV: 

    case R_UMINUS: 
        return oldN;

    case R_EXP: {
        oldN->child(1) = new Node(I_TO_NAT, oldN->child(1));
        return oldN->updateKind(R_EXP_N);
    }

/*
----------------------------------------------------------------------------
Arrays
----------------------------------------------------------------------------
*/

    case ARR_ELEMENT: {
        if (oldN->child(1)->arity() > 1) {
            oldN->child(1)->kind = TUPLE;
        } else {
            oldN->child(1) = oldN->child(1)->child(0);
        }
        return oldN->updateKindAndId(APPLY, "");
    }

    case ARR_UPDATE: {
        if (oldN->child(1)->arity() > 1) {
            oldN->child(1)->kind = TUPLE;
        } else {
            oldN->child(1) = oldN->child(1)->child(0);
        }
        return oldN->updateKindAndId(UPDATE, "");
    }


/*
----------------------------------------------------------------------------
Records
----------------------------------------------------------------------------
*/

    // Remove type argument. Fix fieldname.

    case RCD_ELEMENT: {
        string typeId = oldN->child(1)->id;
        oldN->id = fixRcdFldId(oldN->id, typeId);
        oldN->popChild();
        return oldN;
    }
    case RCD_UPDATE: {
        string typeId = oldN->child(2)->id;
        oldN->id = fixRcdFldId(oldN->id, typeId);
        oldN->popChild();
        return oldN;
    }



/*
----------------------------------------------------------------------------
Otherwise
----------------------------------------------------------------------------
*/

    default: {
            error ("unrecognised kind "
                   + kindString(oldN->kind)
                   );
            return oldN;
        }
    } // end switch(oldN->kind)


}




//========================================================================
// Virtual functions for IsabDriver class
//========================================================================




Node* 
IsabDriver::translateUnit(Node* unit) {

    IsabTranslator t;
    return t.translate(unit);
}


// Create a valid Isabelle/HOL identifier from argument.

// Replaces all unacceptable characters with 's (forward single quotes)
// Not concerned here with ensuring function is injective.

bool isIsabIdChar(char c) {
    return
        ('A' <= c && c <= 'Z')
        || 
        ('a' <= c && c <= 'z')
        || 
        ('0' <= c && c <= '9')
        ||
        c == '\''
        ||
        c == '_';
}

string mkIsabId(const string& s) {
    string result;
    for (int i = 0; i != (int) s.size(); i++) {
        char c = s.at(i);
        if (isIsabIdChar(c))
            result.append(1,c);
        else
            result.append(1,'\'');
    }
    return result;
}



void
IsabDriver::initGoal(const string& unitName,
                         int goalNum,
                         int conclNum) {

    string fullGoalFileRoot
        = getFullGoalFileRoot("vc", unitName, goalNum, conclNum);

    proverInputFileName = fullGoalFileRoot  + ".thy";
    proverOutputFileName = fullGoalFileRoot  + ".out";
    proverErrorFileName = fullGoalFileRoot  + ".err";


    string theoryname = mkIsabId(unitName) + "'" + intToString(goalNum);
    if (conclNum != 0) theoryname += "'" + intToString(conclNum);

    theory = new Node(THEORY, 
                      theoryname,
                      new Node (DECLS),
                      new Node (HYPS),
                      new Node (CONCLS)
                     );
    return;
}

void 
IsabDriver::addDecl(Node* decl) {
    theory->child(0)->addChild(decl);
    return;
}

void
IsabDriver::addRule(Node* hyp, const string& hId, string& remarks) {
    addHyp(hyp,hId,remarks);
    return;
}

void
IsabDriver::addHyp(Node* hyp, const string& hId, string& remarks) {
    theory->child(1)->addChild(new Node(ASSUMPTION,hId,hyp));
    return;
}

void
IsabDriver::addConcl(Node* concl, string& remarks) {
    Node* concls = theory->child(2);
    if (concl->kind == AND) {
        for (int i = 0; i != concl->arity(); i++) {

            concls->addChild(new Node (CONCL,
                                       "C" + intToString(i + 1),
                                       concl->child(i)
                                      ));
        }
    }
    else {
        concls->addChild(new Node (CONCL,
                                   "C1",
                                   concl
                                   ));
    }
    return;
}

void
IsabDriver::finishSetup() {

    // Handle case when skip-concls option asserted

    if (theory->child(2)->arity() == 0) {
        theory->child(2)->addChild(new Node (CONCL, "C1", new Node(z::FALSE)));
    }

    ofstream proverInput;
    proverInput.open(proverInputFileName.c_str());
    if (proverInput.fail()) {
        cerr << endl
             << "Error on trying to open file " << proverInputFileName << endl;
        exit(1);
    }

    Formatter::setFormatter(IsabFormatter::getFormatter());

    proverInput << *theory << endl;
    proverInput.close();
}


//---------------------------------------------------------------------------
// checkGoal
//---------------------------------------------------------------------------

bool
IsabDriver::checkGoal(string& remarks) {

    string cmd;

    if (option("prover-command")) {
        cmd = optionVal("prover-command") + " ";
    } else {
        return false;
    }

    cmd = withTimeoutAndIO(cmd,
                           proverInputFileName,
                           proverOutputFileName,
                           proverErrorFileName);

    printMessage(INFOm, "Running command" + ENDLs
                             + cmd + ENDLs);

    int exitStatus = std::system(cmd.c_str());

    printMessage(INFOm, "Exit status is " + intToString(exitStatus));

    return false;
}



IsabDriver::Status
IsabDriver::getResults(string& remarks) {

    // Do not check output files if none were generated in first place

    if (! (option("prover") || option("prover-command") )) {
        return UNKNOWN;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Read in output and error files from solver
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    ifstream proverOut (proverOutputFileName.c_str() );
    ifstream proverErr (proverErrorFileName.c_str() );

    if (!proverOut) {
        printMessage(ERRORm, "Cannot open output file "
                             + proverOutputFileName);
        appendCommaString(remarks, "Outfile not found");
        return ERROR;
    }

    if (!proverErr) {
        printMessage(ERRORm, "Cannot open error output file "
                             + proverErrorFileName);
        appendCommaString(remarks, "Errorfile not found");
        return ERROR;
    }

    vector<string> proverOutput;
    vector<string> proverErrOutput;
    {
        string line;

        while (getline(proverOut, line)) proverOutput.push_back(line);
        proverOut.close();

        while (getline(proverErr, line)) proverErrOutput.push_back(line);
        proverErr.close();
    }

    // flags for processing of standard error file

    bool seenStdErrNonEmpty = proverErrOutput.size() > 0;

    // flags for processing of standard output file

    bool seenWarningOnStdOut = false;

    vector<string> stdOutWarnings;

    for (int i = 0; i != (int) proverOutput.size(); i++) {

        string s = proverOutput.at(i);

        if (hasPrefix(s, "***")) {
            seenWarningOnStdOut = true;
            stdOutWarnings.push_back(intToString(i) + ": " + s);
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Report on output and decide return status
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    if (seenWarningOnStdOut || seenStdErrNonEmpty) {

        string outMessage = concatStrings(stdOutWarnings, ENDLs);
        string errMessage = concatStrings(proverErrOutput, ENDLs);

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

    return UNKNOWN;

}



//=========================================================================
// END OF FILE
//=========================================================================
