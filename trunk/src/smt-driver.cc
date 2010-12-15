//==========================================================================
//==========================================================================
// SMT-DRIVER.CC
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
using std::cout;

#include <stdexcept>

#include "smt-driver.hh"

#include "utility.hh"

#include "formatter.hh"

// Allocate local storage for static field.
Timer SMTDriver::goalTimer;

void
SMTDriver::driveGoal(Node* decls,
                     Node* rules,
                     Node* hyps,
                     Node* concl,
                     UnitInfo unitInfo,
                     int goalNum,
                     int currentConcl) {

    // ---------------------------------------------------------------
    // Initialise for solver invocation
    // ---------------------------------------------------------------
    printMessage(FINEm, "Considering concl");

    string remarks;  // for insertion into CSV report file.

    if (concl->kind == z::TRUE) {
        printMessage(FINEm, "Input concl is trivial");
        remarks = "trivial concl";
    }
          
    initGoal(unitInfo.getUnitName(), goalNum, currentConcl);

    // ---------------------------------------------------------------
    // Push decls, hyp and concl
    // ---------------------------------------------------------------

    try {

        // -----------------------------------------------------------
        // Push decls
        // -----------------------------------------------------------

        for ( int i = 0;
              i != decls->arity();
              i++) {

            Node* decl = decls->child(i);

            Formatter::setFormatter(VanillaFormatter::getFormatter());
            printMessage(FINESTm,
                         "pushing decl to solver" + ENDLs
                         + decl->toString());

            addDecl(decl);

        }

        // -----------------------------------------------------------
        // Push rules as hyps
        // -----------------------------------------------------------

        for ( int currentRule = 1;
              currentRule <= rules->arity();
              currentRule++) {

            string currentRuleStr ("R" + intToString(currentRule));

            Node* rule = rules->child(currentRule - 1);

            Formatter::setFormatter(VanillaFormatter::getFormatter());
            printMessage(FINESTm,
                         "pushing rule to solver" + ENDLs
                         + rule->toString());

            addRule(rule, currentRuleStr, remarks);
        }


        // -----------------------------------------------------------
        // Push hyps
        // -----------------------------------------------------------

        if (! option("skip-hyps") ) {
                
            for (int currentHyp = 1;
                 currentHyp <= hyps->arity();
                 currentHyp++) {

                string currentHypStr ("H" + intToString(currentHyp));

                Node* hyp = hyps->child(currentHyp-1);

                Formatter::setFormatter
                    (VanillaFormatter::getFormatter());
                printMessage(FINESTm, 
                             "Writing " + currentHypStr + ENDLs
                             + hyp->toString()
                             );

                addHyp(hyp, currentHypStr, remarks);

            } // END: for each hyp in hyps
        }

        // -----------------------------------------------------------
        // Push concl 
        // -----------------------------------------------------------

        if (! option("skip-concls") ) {
            Formatter::setFormatter(VanillaFormatter::getFormatter());
            printMessage(FINESTm,"Pushing concl: " + ENDLs
                         + concl->toString());

            addConcl(concl, remarks);
        }

    } catch (std::runtime_error e) {

        printMessage(ERRORm,
                     string("Push of decl, rule, hyp or concl failed\n")
                     + e.what()
                     );
        goalSliceTime = "0";
        appendCommaString(remarks, "exception in setup");
        printCSVRecord("error", remarks);
        errorConcls++;
        finishSetup();
        finaliseGoal();
        return;
    }

    finishSetup();
    printMessage(INFOm, "setup time: " + goalTimer.toString() + " s");

    // ---------------------------------------------------------------
    // Invoke solver
    // ---------------------------------------------------------------
    if (! option("gstime-inc-setup")) goalTimer.restart();

    int checkRepeats =
        option("check-goal-repeats")
        ? intOptionVal("check-goal-repeats")
        : 1;

    string remarksAtCheckStart(remarks);
            
    bool checkError;

    for (int i = 0; i < checkRepeats; i++) {
        remarks = remarksAtCheckStart;
        checkError = checkGoal(remarks);
    }

    goalSliceTime = goalTimer.toString();

    if (checkError) {
        printMessage(ERRORm, "Error flagged on check of goal\n");
                
        // Let results processing have chance to give further
        // info on error.

        /* 
           printCSVRecord("error", remarks);
           errorConcls++;
           finaliseGoal();
           continue;
        */
    }

    // ---------------------------------------------------------------
    // Process results
    // ---------------------------------------------------------------

    Status s = getResults(remarks);

    if (s == TRUE) {

        printCSVRecord("true", remarks);
        trueConcls++;

    } else if (s == UNPROVEN) {

        printCSVRecord("unproven", remarks);
        unprovenConcls++;

    } else if (s == RESOURCE_LIMIT) {

        printCSVRecord("unproven", remarks);
        unprovenConcls++;
        timeoutConcls++;

    } else {// s == ERROR

        printCSVRecord("error", remarks);
        errorConcls++;
    }
    finaliseGoal();
}


void
SMTDriver::driveUnit(Node* unit, UnitInfo unitInfo) {

    // ---------------------------------------------------------------------
    // Translate unit into solver-specific abstract syntax
    // ---------------------------------------------------------------------

    Node* solverUnit;
    try {
        solverUnit = translateUnit(unit);

    } catch (std::runtime_error e) {

        printMessage(ERRORm, 
                     "Exception in solver-specific translation" + ENDLs
                     + e.what());

        printCSVRecord("error", "solver translation failed");
        return;
    }        
        
    Formatter::setFormatter(VanillaFormatter::getFormatter());
    printMessage(FINESTm, "Solver-specific translated unit:"
                 + ENDLs + solverUnit->toString());

    // ---------------------------------------------------------------------
    // Break out unit components
    // ---------------------------------------------------------------------

    Node* decls = solverUnit->child(0);
    Node* rules = solverUnit->child(1);
    Node* goals = solverUnit->child(2);


    // -------------------------------------------------------------------
    // Loop for each goal in goals
    // -------------------------------------------------------------------
    for (int goalNum = 1; goalNum <= goals->arity(); goalNum++) {

        if (option("goal") && intOptionVal("goal") != goalNum) {
            continue;
        }

        if (option("gtick")) {
            if (option("longtick")) {
                cout << " " << goalNum;
            } else {
                cout << ";";
            }
            cout.flush();
        }

        Node* goal = goals->child(goalNum-1);
        
        // Extract out components of goal->id and store in global vars
        // used for output messages
        updateCurrentGoalInfo(goal->id);
        currentConcl = 0;

        if (goal->arity() < 2) { // "*** true" goals
            printMessage(FINEm, "Input goal is trivial");
            if (option("count-trivial-goals")) {
                printCSVRecord("true", "trivial goal");
                trueConcls++;
            }
            continue; 
        }

        Node* hyps = goal->child(0);
        Node* concls = goal->child(1);

        if (option("fuse-concls") 
            && (option("fuse-unary-concls") || concls->arity() > 1)
           ) {
            concls->kind = z::AND;
            concls = new Node(z::CONCLS, concls);
        }
        
        if (option("hkinds")) currentHypsKinds = gatherKinds(hyps);
        
        // -------------------------------------------------------------------
        // Loop for each concl in concls of goal
        // -------------------------------------------------------------------

        for ( currentConcl = 1;
              currentConcl <= concls->arity();
              currentConcl++) {

            if (option("concl") && intOptionVal("concl") != currentConcl) {
                continue;
            }

            if (option("ctick")) {
                if (option("longtick")) {
                    cout << "." << currentConcl;
                } else {
                    cout << ".";
                }
                cout.flush();
            }

            Node* concl = concls->child(currentConcl-1);

            if (option("ckinds")) currentConclKinds = gatherKinds(concl);

            if (! unitInfo.include(goalNum, currentConcl)) {
                goalSliceTime = "0";
                printCSVRecord("unproven", "excluded");
                unprovenConcls++;
                continue;
            }
            
            int driveGoalRepeats =
                option("drive-goal-repeats")
                ? intOptionVal("drive-goal-repeats")
                : 1;

            goalTimer.restart();
            for (int i = 0; i < driveGoalRepeats; i++) {
                driveGoal(decls,
                          rules,
                          hyps,
                          concl,
                          unitInfo,
                          goalNum,
                          currentConcl);
            }

        } // END for each concl in concls
    } // END for each goal in goals

    return;
}


//==========================================================================
// Default definitions for solver API
//==========================================================================


Node*
SMTDriver::translateUnit(Node* unit) { return unit; }

bool
SMTDriver::checkGoal(string& remarks) { return false; }

SMTDriver::Status
SMTDriver::getResults(string& remarks) { return UNPROVEN; }
