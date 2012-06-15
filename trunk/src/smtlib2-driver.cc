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

#include "smtlib2-driver.hh"

#include "node-utils.hh"

#include "formatter.hh"
#include "pprinter.hh"

#include <algorithm>  // For set operations
using namespace z;


//==========================================================================
// Pretty print SMTLib2 abstract syntax
//==========================================================================

class SMTLib2Formatter : public Formatter {
public:
    virtual Box& addSyntax(z::Kind k, const string& id,
                           vector<Box*>& bs);

    static Formatter* getFormatter();

private:

    static Formatter* instance;
    SMTLib2Formatter() {};


};

Box& SMTLib2Formatter::addSyntax(z::Kind k, const std::string& id,
                              vector<Box*>& bs) {

    switch (k) {
    // Top level

    case(SCRIPT):      return PP::simpleVSeq(bs);

    case(LOGIC):       return box("(set-logic " + id + ")");
    case(SET_OPTION):  return makeStringAp("set-option :" + id, bs);
    case(ASSERT):      return makeStringAp("assert", bs);
    case(CHECK):       return makeStringAp("check-sat", bs);
    case(GET_UNSAT_CORE):
                       return makeStringAp("get-unsat-core", bs);
    case(PUSH):        return box("(push 1)");
    case(POP):         return box("(pop 1)");

    case(LABEL):       { bs.push_back(&box(":named "
                                           + mkLispSymbolString(id)));
                         return makeStringAp("!",bs);
                       }
        
    case(COMMENT):     return box("; " + id);

    // Declarations

    case(DECL_TYPE):   return box("(declare-sort " + id + " 0)");
    case(DEF_TYPE):    return makeStringAp("define-sort " + id + " ()", bs);
    case(DECL_FUN):    return makeStringAp("declare-fun " + id, bs);
    case(DECL_CONST):  return makeStringAp("declare-fun " + id + " ()", bs);

    // Sorts

    case(TYPE_ID):     return box(id);
    case(INT_TY):      return box("Int");
    case(REAL_TY):      return box("Real");
    case(BOOL_TY):      return box("Bool");

    // Core logic terms and consts

    case(OR):          return makeStringAp("or", bs);
    case(AND):         return makeStringAp("and", bs);
    case(NOT):         return makeStringAp("not", bs);
    case(IMPLIES):     return makeStringAp("=>", bs);
    case(IFF):         return makeStringAp("=", bs);

    case(TRUE):        return box("true");
    case(FALSE):       return box("false");

    case(FORALL):      return makeStringAp("forall", bs);
    case(EXISTS):      return makeStringAp("exists", bs);

    case(EQ):          return makeStringAp("=", bs);
    case(DISTINCT):    return makeStringAp("distinct", bs);
    case(ITE):         return makeStringAp("ite", bs);

        // SEQ is not quite prettiest: will indent vertical sequence
        // extra amount.

    case(SEQ):         return PP::makeHVSeq("",
                                            "(",
                                            "",
                                            ")",
                                            bs);
    case(DECL):        return makeStringAp(id, bs);

    case(FUN_AP):      return makeStringAp(id, bs);
    case(CONST):       return box(id);
    case(VAR):         return box(id);

    // Arithmetic operators and constants

    case(I_UMINUS):    return makeStringAp("-", bs);
    case(I_MINUS):     return makeStringAp("-", bs);
    case(I_PLUS):      return makeStringAp("+", bs);
    case(I_TIMES):     return makeStringAp("*", bs);
    case(IDIV_E):      return makeStringAp("div", bs);
    case(MOD_E):       return makeStringAp("mod", bs);
    case(I_ABS):       return makeStringAp("abs", bs);
    case(I_LE):        return makeStringAp("<=", bs);
    case(I_LT):        return makeStringAp("<", bs);
    case(NATNUM):      return box(id);
    case(R_UMINUS):    return makeStringAp("-", bs);
    case(R_MINUS):     return makeStringAp("-", bs);
    case(R_PLUS):      return makeStringAp("+", bs);
    case(R_TIMES):     return makeStringAp("*", bs);
    case(RDIV):        return makeStringAp("/", bs);
    case(R_LE):        return makeStringAp("<=", bs);
    case(R_LT):        return makeStringAp("<", bs);
    case(TO_REAL):     return makeStringAp("to_real", bs);
    case(TO_INT):      return makeStringAp("to_int", bs);
    case(IS_INT):      return makeStringAp("is_int", bs);

    case(SET_INFO):    return makeStringAp("set-info :" + id, bs);
    case(INFO_STR):    return box('"' + id + '"');

    default:
	printMessage(ERRORm, "SMTLib2Formatter::addSyntax: " + ENDLs
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





Formatter*
SMTLib2Formatter::getFormatter() {
    if (instance == 0) instance = new SMTLib2Formatter;
    return instance;
}

// Storage declarations for static class member.

Formatter*
SMTLib2Formatter::instance = 0;


//========================================================================
// Standardise identifiers
//========================================================================
/*
1. Type renaming.

   If type-id clashes with SMTLIB2 reserved word or type defined in
   one of SMTLIB2 standard theories, add "~" suffix.

2. Function renaming

   If function id clashes with reserved word or (possibly nullary)
   function defined in one of SMTLIB2 standard theories, add "~"
   suffix.

3. Constant renaming

   (Including bound variables in notion of constant here.)

   If constant id clashes with

   1. reserved word, or
     (possibly nullary) function defined in one of SMTLIB2 standard
      theories, or
   2. Any of function ids declared in unit,

   But not 1 & 2, add "~" suffix.

   If 1 & 2, add "~~" suffix.
*/

// funIdsInUnit: functions declared in unit
// constIdsInUnit: constants declared in unit at top level and
//   in quantifier bindings


class StandardiseIdsFun {
public:

    StandardiseIdsFun(Node* unit);

    // Sets for use in standardising ids.

    set<string> typeIdsToRename;
    set<string> funIdsToRename;
    set<string> constIdsToRename1;
    set<string> constIdsToRename2;

    void operator() (Node* n);
};

/*
#include <iostream>
using std::endl;
using std::cout;
using std::cerr;
*/

StandardiseIdsFun::StandardiseIdsFun(Node* unit) {

    set<string> typeIdsInUnit;
    set<string> funIdsInUnit;
    set<string> constIdsInUnit;

    Node* decls = unit->child(0);
    for (int i = 0; i != decls->arity(); i++) {
        Node* decl = decls->child(i);
        if (decl->kind == DECL_FUN)
            funIdsInUnit.insert(decl->id);
        else if (decl->kind == DEF_CONST)
            constIdsInUnit.insert(decl->id);
        else if (decl->kind == DEF_TYPE)
            typeIdsInUnit.insert(decl->id);
    }
    set<string> boundVars = gatherBoundVars(unit);
    constIdsInUnit.insert(boundVars.begin(), boundVars.end());

    // Initialise reservedWords and stdFunIds

    set<string> reservedWords;
    set<string>& rw = reservedWords;
    rw.insert("par");
    rw.insert("NUMERAL");
    rw.insert("DECIMAL");
    rw.insert("STRING");
    rw.insert("_");
    rw.insert("as");
    rw.insert("let");
    rw.insert("forall");
    rw.insert("exists");
    rw.insert("set");
    rw.insert("push");
    rw.insert("pop");
    rw.insert("assert");
    rw.insert("exit");

    set<string> stdFunIds;
    set<string>& sfi = stdFunIds;
    sfi.insert("distinct");
    sfi.insert("true");
    sfi.insert("false");
    sfi.insert("not");
    sfi.insert("and");
    sfi.insert("or");
    sfi.insert("xor");
    sfi.insert("ite");
    sfi.insert("div");
    sfi.insert("mod");
    sfi.insert("abs");
    sfi.insert("to_real");
    sfi.insert("to_int");
    sfi.insert("is_int");
    sfi.insert("select");
    sfi.insert("store");

    // typeIdsToRename = typeIdsInUnit intersect (reservedWords U stdTypeIds)

    set<string> s0;
    s0.insert(reservedWords.begin(), reservedWords.end());
    s0.insert("Bool");
    s0.insert("Int");
    s0.insert("Real");

    setIsect(typeIdsInUnit, s0, typeIdsToRename);

    // s1 = reservedWords U stdFunIds
    set<string> s1;
    setUnion(reservedWords, stdFunIds, s1);

    // funIdsToRename = funIdsInUnit intersect (reservedWords U stdFunIds)
    //                = funIdsInUnit intersect s1

    setIsect(funIdsInUnit, s1, funIdsToRename);

    // s2 = symdiff funIdsInUnit (reservedWords U stdFunIds)
    //    = symdiff funIdsInUnit s1

    set<string> s2;
    setSymDiff(funIdsInUnit, s1, s2);

    // constIdsToRename1
    //    = constIdsInUnit
    //      intersect (symdiff  (reservedWords U StdFunIds)
    //                          funIdsInUnit)
    //    = constIdsInUnit intersect s2

    setIsect(constIdsInUnit, s2, constIdsToRename1);

    // constIdsToRename2 =
    //    constIdsInUnit intersect funIdsToRename

    setIsect(constIdsInUnit, funIdsToRename, constIdsToRename2);

    /*
    cout << endl;
    cout << "typeIdsToRename = " << typeIdsToRename.size() << endl;
    cout << "funIdsToRename = " << funIdsToRename.size() << endl;
    cout << "constIdsToRename1 = " << constIdsToRename1.size() << endl;
    cout << "constIdsToRename2 = " << constIdsToRename2.size() << endl;
    */
    return;
}


void
StandardiseIdsFun::operator() (Node* n) {

    switch(n->kind) {

    case(TYPE_ID):
    case(DEF_TYPE):
    case(DECL_TYPE): {
        if (typeIdsToRename.find(n->id) != typeIdsToRename.end())
            n->id.append("~");
        return;
    }
    case(FUN_AP):
    case(DECL_FUN): {
        if (funIdsToRename.find(n->id) != funIdsToRename.end())
            n->id.append("~");
        return;
    }
    case(CONST):
    case(VAR):
    case(DECL): // In quantifiers
    case(DECL_CONST):
    case(DEF_CONST): {
        if (constIdsToRename1.find(n->id) != constIdsToRename1.end())
            n->id.append("~");
        else if (constIdsToRename2.find(n->id) != constIdsToRename2.end())
            n->id.append("~~");
        return;
    }
    default: return;
    }
    return;
}


//========================================================================
// Virtual functions for SMTLIB2Driver class
//========================================================================


Node*
SMTLib2Driver::translateUnit(Node* unit) {

    // standardise identifiers

    StandardiseIdsFun f(unit);
    unit->mapOver(f);

    // Update declarations

    // DEF_TYPE{<type-id>}    --> DECL_TYPE{<type-id>}
    // DEF_CONST{<id>} <type> --> DECL_CONST{<id>} <type>
    // DEF_TYPE{<type-id>} <type>             Untouched
    // DECL_FUN{<id>} (SEQ <types>) <type>    Untouched


    Node* decls = unit->child(0);

    for (int i = 0; i != decls->arity(); i++) {
        Node* decl = decls->child(i);

        if (decl->kind == DEF_TYPE && decl->arity() == 0) {
            decl->kind = DECL_TYPE;
        }
        else if (decl->kind == DEF_CONST && decl->arity() == 1) {
            decl->kind = DECL_CONST;
        }
    }


    // CVC3 2.4 is not recognising primitive SMTLIB2 to_real operator.
    if (option("smtlib2-add-to_real-decl")) {
        decls->addChild(new Node(DECL_FUN,"to_real",
                                 new Node (SEQ, new Node(INT_TY)),
                                 new Node (REAL_TY)));
    }

    return unit;
}


// If watchdog timer used for timeouts, then time limit is for each query.

bool
SMTLib2Driver::resourceLimitsForQuerySet() {
        return !option("watchdog-timeout");
}

void
SMTLib2Driver::initQuerySet(const string& unitName,
                         int goalNum,
                         int conclNum) {

    string fullGoalFileRoot
        = getFullGoalFileRoot("smtsolver", unitName, goalNum, conclNum);

    solverInputFileName = fullGoalFileRoot  + ".smt2";
    solverOutputFileName = fullGoalFileRoot  + ".out";
    solverErrorFileName = fullGoalFileRoot  + ".err";

    string logic(option("logic") ? optionVal("logic") : "AUFNIRA");

    script = new Node(SCRIPT);
    script->addChild(new Node(SET_INFO, "spark-source",
			      new Node(INFO_STR, unitName)));
    if (!option("smtlib2-omit-set-option-command")) {
        script->addChild(new Node(SET_OPTION, "print-success",
                                  new Node(z::FALSE)));
    }
    if (option("smtlib2-soft-timeout")) {
        script->addChild(new Node(SET_OPTION, "soft-timeout",
                                  new Node(NATNUM,
                                           optionVal("smtlib2-soft-timeout")))
            );
    }
    if (option("smtlib2-unsat-cores")) {
        script->addChild(new Node(SET_OPTION, "produce-unsat-cores",
                                  new Node(z::TRUE)));
    }
    script->addChild(new Node(LOGIC,logic));

    return;
}

void
SMTLib2Driver::initGoal(const string& unitName,
                         int goalNum,
                         int conclNum) {
    initQuerySet(unitName, goalNum, conclNum);
}

void
SMTLib2Driver::addDecl(Node* decl) {
    script->addChild(decl);
}

void
SMTLib2Driver::addRule(Node* rule, const string& ruleId, string& remarks) {
    addHyp(rule, ruleId, remarks);
}

void
SMTLib2Driver::addHyp(Node* hyp, const string& hId, string& remarks) {
    if (option("add-formula-descriptions")) {
        script->addChild(new Node(COMMENT, hId));
    }
    if (option("smtlib2-unsat-cores")) {
        script->addChild(new Node(ASSERT,new Node(LABEL,hId,hyp)));
    } else {
        script->addChild(new Node(ASSERT,hyp));
    }
}

void
SMTLib2Driver::addConcl(Node* concl, string& remarks) {
    addHyp(new Node(NOT, concl), "C", remarks);
}

void
SMTLib2Driver::push() {
    script->addChild(new Node(PUSH));
}

SMTDriver::Status
SMTLib2Driver::check(string& remarks) {
    script->addChild(new Node(CHECK));
    if (option("smtlib2-unsat-cores")) {
        script->addChild(new Node(GET_UNSAT_CORE));
    }

    return UNCHECKED;
}

void
SMTLib2Driver::pop() {
    script->addChild(new Node(POP));
}


//---------------------------------------------------------------------------
// check
//---------------------------------------------------------------------------

void
SMTLib2Driver::finishSetup() {
    script->addChild(new Node(CHECK));
    outputQuerySet();
    return;
}

void
SMTLib2Driver::outputQuerySet() {

    ofstream solverInput;
    solverInput.open(solverInputFileName.c_str());
    if (solverInput.fail()) {
        cerr << endl
             << "Error on trying to open file " << solverInputFileName << endl;
        exit(1);
    }

    Formatter::setFormatter(SMTLib2Formatter::getFormatter());

    solverInput << *script << endl;
    solverInput.close();
}
//---------------------------------------------------------------------------
// check
//---------------------------------------------------------------------------
bool
SMTLib2Driver::checkGoal(string& remarks) {
    return runQuerySet(remarks);
}

bool
SMTLib2Driver::runQuerySet(string& remarks) {

    string cmd;
    string cmdOptions;

    if (! (option("prover") || option("prover-command") )) {
        return false;
    }

    if (option("prover-command")) {

        cmd = optionVal("prover-command") + " ";
    }

    // Must be case now that prover option set

    else if (optionVal("prover") == "z3") {

        cmd = "z3";

        if (option("z3-fourier-motzkin"))
            cmdOptions += "FOURIER_MOTZKIN_ELIM=true ";
        if (option("timeout"))
            // Was not supported in Z3 v1.3 Linux.
            cmdOptions += "SOFT_TIMEOUT=" + optionVal("timeout") + " ";
    }
    else if (optionVal("prover") == "cvc3") {

        cmd = "cvc3";
        cmdOptions = "-lang smt2 ";
        if (option("timeout"))
            cmdOptions += "-stimeout " + optionVal("timeout") + " ";
        if (option("resourcelimit"))
            cmdOptions += "-resource " + optionVal("resourcelimit") + " ";
    }
    else {
        printMessage(ERRORm, "Unsupported prover option: "
                     + optionVal("prover") + ENDLs);
        return false;
    }

    // Modify cmd for I/O files and detecting timeouts.
    if (option("shell-timeout")) {
        // Use shell-level timeout utility
        // This will accept integer or fixed point time in sec.

        cmd = "./timeout.sh " + optionVal("shell-timeout") + " "
            + cmd + " " + cmdOptions
            + " 1> "  + solverOutputFileName
            + " 2> "  + solverErrorFileName;
    }
    else if (option("ulimit-timeout")) {
        // Use bash built-in timeout facility
        // This accepts  integer time in sec.

        //  cmd = "ulimit -t " + optionVal("ulimit-timeout") + " ; " + cmd;

        // Run in a subshell (y using enclosing ()) so we can catch output
        // to stderr on termination and avoid this output polluting
        // output from running vct.

        cmd = "( ulimit -t " + optionVal("ulimit-timeout")
            + " ; " + cmd + " " + cmdOptions + solverInputFileName + " )"
            + " 1> "  + solverOutputFileName
            + " 2> "  + solverErrorFileName;
    }
    else if (option("watchdog-timeout")) {
        // Use watchdogrun timeout facility:
        // Usage:
        //   watchdogrun outfile errfile timeout cmd arg1 ...  argn

        // timeout is read as floating-point number.

        cmd = "./watchdogrun "
            + solverOutputFileName + " "
            + solverErrorFileName + " "
            + optionVal("watchdog-timeout") + " "
            + cmd + " "
            + cmdOptions + " "
            + solverInputFileName;

    } else {

        cmd += " " + solverInputFileName
            + " 1> "  + solverOutputFileName
            + " 2> "  + solverErrorFileName;
    }

    if (option("doublerun")) cmd = cmd + " ; " + cmd;

    printMessage(INFOm, "Running command" + ENDLs
                             + cmd + ENDLs);

    // Should rename this to terminationStatus.  exitStatus is just
    // part of the terminationStatus.
    exitStatus = std::system(cmd.c_str());

    // For SMT mode, exit status is not reliable guide for something going
    // wrong.  E.g. z3 -smt returns exit status 103.

    printMessage(INFOm, "Exit status is " + intToString(exitStatus));

    return false;
}

//---------------------------------------------------------------------------
// analyse Exit Status of solver
//---------------------------------------------------------------------------

// Returns true iff exit code indicates that resource limit reached.
bool
SMTLib2Driver::analyseExitStatus(int exitStatus, string& remarks) {
    bool resourceLimitReached = false;

    //  Detecting termination signals, not on windows platform:
#ifndef _WIN32

    /* Code here discovered by trial and error and reading man pages.

       Man page for getrlimit makes clear that a process is terminated using
       a KILL signal when the time limit is reached.

       system(3) man page says that return status of system call is in format
       specified on wait(2) man page.  There is says:

       WIFEXITED(status)
              returns true if the child terminated normally, that is, by call-
              ing exit(3) or _exit(2), or by returning from main().

       WEXITSTATUS(status)
              returns the exit status of the  child.   This  consists  of  the
              least  significant  8 bits of the status argument that the child
              specified in a call to exit(3) or _exit(2) or  as  the  argument
              for  a  return  statement  in main().  This macro should only be
              employed if WIFEXITED returned true.

       WIFSIGNALED(status)
              returns true if the child process was terminated by a signal.

       WTERMSIG(status)
              returns the number of the signal that caused the  child  process
              to terminate.  This macro should only be employed if WIFSIGNALED
              returned true.

     For z3 3.0 on Scientific Linux 6, are seeing:

     on unsat:

     exitStatus = 0
     WIFSIGNALED = 0
     WTERMSIG = 0
     WIFEXITED = 1
     WEXITSTATUS = 0

     on ulimit timeout:

     exitStatus = 35072   (= 137 * 256)
     WIFSIGNALED = 0
     WTERMSIG = 0
     WIFEXITED = 1
     WEXITSTATUS = 137   (bits 15-8 of exitStatus, as remarked on wait(2) page)

     WIFSIGNALED is *not* being set on a kill, and WTERMSIG is not pulling
     out any signal number.

     Some web pages remark on exit codes of form 128 + signumber being used
     to flag processes terminated by signals.  SIGKILL has value 9, hence
     this 137.

      Can see this 137 by e.g. doing:
        ulimit -t 1 ; z3 -smt x.smt ; echo $?

     Watchdogrun routine uses SIGTERM (15) to kill z3.  This results
     in z3 returning 128+15 = 143.
    */
    /*
    printMessageWithHeader
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
            resourceLimitReached = true;
        }
        else if (exitCode == 128 + SIGTERM) {
            appendCommaString(remarks, "timeout (exit code 143)");
            printMessage(INFOm,
                         "Solver killed.  Assume by SIGTERM from watchdogrun.");
            resourceLimitReached = true;
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

    return resourceLimitReached;
}
//---------------------------------------------------------------------------
// getResults
//---------------------------------------------------------------------------
SMTLib2Driver::Status
SMTLib2Driver::getResults(string& remarks) {

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

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Initialise flags for tracking run status
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // flags for processing of standard error file

    bool seenTimeout = false;
    bool seenWarning = false;
    bool seenUnexpectedErrOutput = false;

    // flags for processing of standard output file

    bool seenSatOutput = false;
    bool seenUnsatOutput = false;
    bool seenUnknownOutput = false;
    bool seenUnexpectedOutput = false;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Check over stderr output from solver
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // First inspect error output.  Check for
    // - warning messages than can be logged
    // - Timeout termination messages
    //
    // First cut, just echo output to log file. Minimally process.

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

        // z3 at least uses this.
        if (hasPrefix(s,"WARNING:")) {

            if (option("log-smtsolver-warnings")) {

                seenWarning = true;
                printMessage(WARNINGm, "Warning message from SMTLib solver"
                             + ENDLs + s);
            }
            continue;
        }
        seenUnexpectedErrOutput = true;

    }
    if (seenWarning) appendCommaString(remarks, "warning(s)");


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Check over stdout output from solver
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int i = 0; i != (int) solverOutput.size(); i++) {

        string s = solverOutput.at(i);
        vector<string> line = tokeniseString(s);

        if (line.size() == 0) {
            continue;
        }

        if (line.size() == 1 && line.at(0) == "unsat") {
            seenUnsatOutput = true;
        }
        else if (line.size() == 1 && line.at(0) == "sat") {
            seenSatOutput = true;
        }
        else if (line.size() == 1 && line.at(0) == "unknown") {
            seenUnknownOutput = true;
        } else {
            seenUnexpectedOutput = true;
        }
    }


    if (analyseExitStatus(exitStatus, remarks)) {
        seenTimeout = true;
    }

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
// getRunResults
//---------------------------------------------------------------------------

vector<SMTDriver::QueryStatus>
SMTLib2Driver::getRunResults(int numQueries) {

    vector<SMTLib2Driver::QueryStatus> results;

    // Do not check output files if none were generated in first place

    if (! (option("prover") || option("prover-command") )) {
        results.push_back(QueryStatus(UNKNOWN,"prover not run",0.0));
        return results;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Read in output and error files from solver
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    ifstream solverOut (solverOutputFileName.c_str() );
    ifstream solverErr (solverErrorFileName.c_str() );

    if (!solverOut) {
        printMessage(ERRORm, "Cannot open output file "
                             + solverOutputFileName);
        results.push_back(QueryStatus(ERROR, ".out file not found",0.0));
        return results;
    }

    if (!solverErr) {
        printMessage(ERRORm, "Cannot open error output file "
                             + solverErrorFileName);
        results.push_back(QueryStatus(ERROR, ".err file not found",0.0));
        return results;
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

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Initialise flags for tracking run status
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // flags for processing of standard error file

    bool seenTimeout = false;
    bool seenWarning = false;
    bool seenUnexpectedErrOutput = false;

    // flags for processing of standard output file

    bool seenUnexpectedOutput = false;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Check over stderr output from solver
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // First inspect error output.  Check for
    // - warning messages than can be logged
    // - Timeout termination messages
    //
    // First cut, just echo output to log file. Minimally process.

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

        // z3 at least uses this.
        if (hasPrefix(s,"WARNING:")) {

            if (option("log-smtsolver-warnings")) {

                seenWarning = true;
                printMessage(WARNINGm, "Warning message from SMTLib solver"
                             + ENDLs + s);
            }
            continue;
        }

        seenUnexpectedErrOutput = true;


    }
    // If there are multiple goals, we really don't know which one the
    // warning messages are associated with.  If we were to do
    // anything here, we would be conservative and record warning
    // message with all the goals.

    // if (seenWarning) appendCommaString(remarks, "warning(s)");

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Check over stdout output from solver
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // With unsat cores, expect one of
    //
    // unsat
    // (<name1> ... <namek>)
    //
    // sat
    // (error "<errstring>")
    //
    // unknown
    // (error "<errstring>")
    //
    for (int i = 0; i != (int) solverOutput.size(); i++) {

        string s = solverOutput.at(i);
        vector<string> line = tokeniseString(s);
        
        
        if (line.size() == 0) {
            continue;
        }
        Status st = ERROR;
        if (line.size() == 1) {
            if (line.at(0) == "unsat") st = TRUE;
            if (line.at(0) == "sat") st = FALSE;
            if (line.at(0) == "unknown") st = UNKNOWN;
        }

        if (st == ERROR) {
            seenUnexpectedOutput = true;
        } else {
            string remarks;

            if (option("smtlib2-unsat-cores")) {
                // Grab extra line.
                // if it is an unsat core, record it in remarks, and
                // otherwise check for "(error" prefix

                if (i == (int) solverOutput.size() - 1) {
                    seenUnexpectedOutput = true;
                } else {
                    i++;
                    if (st == TRUE)
                        remarks = solverOutput.at(i);
                    else if (!hasPrefix(solverOutput.at(i),"(error"))
                        seenUnexpectedOutput = true;
                }
            }
            results.push_back(QueryStatus(st,remarks,0.0));
        }

    }


    // Make the assumption that the solver outputs "unknown" on stdout if
    // it is killed because of resource limit.  This seems to be true of z3
    // If not true, then an extra QueryStatus should be added to results.
    // NOT TRUE
    string exitStatusRemarks;
    if (analyseExitStatus(exitStatus, exitStatusRemarks)) {
        seenTimeout = true;
    }

    /*
    if (seenTimeout) {
        if (results.back().status == TRUE) {
            printMessage(ERRORm,
                         "Resource limit detected on true goal.  Something has gone wrong");
        }
        results.back().status = RESOURCE_LIMIT;
        results.back().remarks = exitStatusRemarks;
    }
    */

    if (seenTimeout) {
        results.push_back(QueryStatus(RESOURCE_LIMIT,exitStatusRemarks,0.0));
    }

    // Ensure at least 1 result reported
    if (results.size() == 0) {
        results.push_back(QueryStatus(ERROR,"",0.0));
        seenUnexpectedOutput = true;
    }


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
        // It's hard to know which of results an error is associated with.
        // Just pick to report it with the first.
        results.front().status = ERROR;
    }

    if ((int) results.size() > numQueries
        && !option("ignore-extra-query-results")) {

        printMessage(ERRORm,
                     "getRunResults is reporting more results than queries");
    }

    return results;
}

//---------------------------------------------------------------------------
// finaliseQuerySet
//---------------------------------------------------------------------------

void
SMTLib2Driver::finaliseQuerySet() {

    if (option("delete-working-files")) {

        tryRemoveFile(solverInputFileName);
        tryRemoveFile(solverOutputFileName);
        tryRemoveFile(solverErrorFileName);
    }
    return;
}

//---------------------------------------------------------------------------
// finaliseGoal
//---------------------------------------------------------------------------

void
SMTLib2Driver::finaliseGoal() { finaliseQuerySet(); }


//=========================================================================
// END OF FILE
//=========================================================================
