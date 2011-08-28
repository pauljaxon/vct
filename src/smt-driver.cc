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

#include <cassert>

#include "smt-driver.hh"

#include "utility.hh"

#include "formatter.hh"

// Allocate local storage for static field.
Timer SMTDriver::goalTimer;



//==========================================================================
// Drive Goal
//==========================================================================

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

            Node* rule = rules->child(currentRule - 1);
            string currentRuleStr;
            if (rule->kind == z::RULE) {
                currentRuleStr = rule->id;
                rule = rule->child(0);
            }
            else {
                currentRuleStr = "R" + intToString(currentRule);
            }



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

//==========================================================================
// Drive Unit
//==========================================================================

void
SMTDriver::driveUnit(Node* unit, UnitInfo unitInfo) {

    if (option("use-alt-solver-driver")) {
        return altDriveUnit(unit, unitInfo);
    }
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

// Default implementation for online (API) solver interface 
SMTDriver::Status
SMTDriver::check(string& remarks) {
    bool checkError = checkGoal(remarks);
   if (checkError) {
       printMessage(ERRORm, "Error flagged on check of goal\n");
   }
   return getResults(remarks);
}



//==========================================================================
// Alternative Driver
// ==========================================================================
// Idea is to allow for exploiting incrementality of solvers.  Hope is
// to get some performance improvement, partly because of
// incrementality, partly because of reduced number of solver runs and
// overhead of starting solver.

// To add: if goal has multiple concls and are not fusing concls,
// should only add hyps once.

//==========================================================================
// Drive Query Set
//==========================================================================

vector<SMTDriver::QueryStatus> 
SMTDriver::driveQuerySet(UnitInfo unitInfo,
                         Node* unit,
                         int startQuery,
                         int endQuery) {

    string remarks(queryRecords.at(startQuery).remarks);

    {
        int currentGoalNum = queryRecords.at(startQuery).goalNum;

        // Set globals used in messages
        currentGoalNumStr = intToString(currentGoalNum);
        currentConcl = queryRecords.at(startQuery).conclNum;

        // Set up header of query set
        initQuerySet(unitInfo.getUnitName(), currentGoalNum, currentConcl);
    }

    // ---------------------------------------------------------------------
    // Break out unit components
    // ---------------------------------------------------------------------

    Node* decls = unit->child(0);
    Node* rules = unit->child(1);
    Node* goals = unit->child(2);

    int numNonTrivQueries = 0;
    vector<QueryStatus> results;

    // ---------------------------------------------------------------------
    // Start block for capturing online interface exceptions
    // ---------------------------------------------------------------------
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
        // Push rules 
        // -----------------------------------------------------------

        for ( int currentRule = 1;
              currentRule <= rules->arity();
              currentRule++) {

            Node* rule = rules->child(currentRule - 1);
            string currentRuleStr;
            if (rule->kind == z::RULE) {
                currentRuleStr = rule->id;
                rule = rule->child(0);
            }
            else {
                currentRuleStr = "R" + intToString(currentRule);
            }


            Formatter::setFormatter(VanillaFormatter::getFormatter());
            printMessage(FINESTm,
                         "pushing rule to solver" + ENDLs
                         + rule->toString());

            addRule(rule, currentRuleStr, remarks);
        }

        // ---------------------------------------------------------------------
        // Push hyps and concls incrementally
        // ---------------------------------------------------------------------

        string declsRulesRemarks(remarks);
        
        // ---------------------------------------------------------------------
        // Loop over queries of queryset
        // ---------------------------------------------------------------------
        
        for (int query = startQuery; query <= endQuery; query++) {

            remarks = declsRulesRemarks;
            
            if (queryRecords.at(query).status == TRIVIAL) {
                
                continue;
            }
            numNonTrivQueries++;

            int goalNum = queryRecords.at(query).goalNum;
            Node* goal = goals->child(goalNum-1);

            // Set global for message reporting
            currentGoalNumStr = intToString(goalNum);

            Node* hyps = goal->child(0);
            Node* concls = goal->child(1);

            int conclNum = queryRecords.at(query).conclNum;

            // Set global for message reporting
            currentConcl = conclNum;

            Node* queryConcl;
            if (conclNum == 0) {
                if (concls->arity() > 1) {
                    queryConcl = new Node(z::AND); // Assume n-ary AND
                                                   // supported by solver
                    queryConcl->appendChildren(concls);
                }
                else {
                    queryConcl = concls->child(0);
                }
            }
            else {
                queryConcl = concls->child(conclNum-1);
            }

            // -----------------------------------------------------------
            // Push new empty assertion set onto assertion set stack
            // -----------------------------------------------------------

            if (option("exploit-solver-incrementality")) push();

            // -----------------------------------------------------------
            // Push hyps
            // -----------------------------------------------------------

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

            // -----------------------------------------------------------
            // Push concl 
            // -----------------------------------------------------------

            if (! option("skip-concls") ) {
                Formatter::setFormatter(VanillaFormatter::getFormatter());
                printMessage(FINESTm,"Pushing concl: " + ENDLs
                             + queryConcl->toString());

                addConcl(queryConcl, remarks);
            }

            // -----------------------------------------------------------
            // Check
            // -----------------------------------------------------------

            Status status = check(remarks);
        
            // -----------------------------------------------------------
            // Pop assertion set stack
            // -----------------------------------------------------------
            if (option("exploit-solver-incrementality")) pop();

            // -----------------------------------------------------------
            // Record status if online interface
            // -----------------------------------------------------------
            // Status ignored if have offline interface.
            // Status recording delayed to here to allow exception handler
            // to catch pop exception and record an alternate status.
            if (onlineInterface()) {
                results.push_back(QueryStatus(status,remarks));
            }

        } // END for each query
        // ---------------------------------------------------------------------
        // END of loop over queries of queryset
        // ---------------------------------------------------------------------

    // ---------------------------------------------------------------------
    // Handle online interface exceptions
    // ---------------------------------------------------------------------
    } catch (std::runtime_error e) {

        printMessage(ERRORm,
                     string("Exception in driving query set\n")
                     + e.what()
            );
        // goalSliceTime = "0";
        appendCommaString(remarks, "exception in driving query set");

        finaliseQuerySet();

        results.push_back(QueryStatus(ERROR,remarks));

        return results;
     
    }

    // ---------------------------------------------------------------------
    // Run query set offline on solver
    // ---------------------------------------------------------------------

    if (!onlineInterface() && numNonTrivQueries > 0) {

        // With file level interface, write file.
        outputQuerySet();

        bool runError = runQuerySet(remarks);
        if (runError) {
            printMessage(ERRORm, "Error flagged on run of solver\n");
        }
        results = getRunResults(numNonTrivQueries);
    }

    finaliseQuerySet();
    return results;
}

//==========================================================================
// Altrnative Drive Unit
//==========================================================================

void
SMTDriver::altDriveUnit(Node* unit, UnitInfo unitInfo) {

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

    
    //------------------------------------------------------------------------
    // Initialise table of query records, one for each solver query
    //------------------------------------------------------------------------
    queryRecords.clear();
    
    Node* goals = solverUnit->child(2);

    for (int goalNum = 1; goalNum <= goals->arity(); goalNum++) {

        if (option("goal") && intOptionVal("goal") != goalNum) {
            continue;
        }
        Node* goal = goals->child(goalNum-1);

        QueryRecord qRcd;

        string goalNumStr;
        extractGoalInfo(goal->id,
                        qRcd.unitKind,
                        goalNumStr,
                        qRcd.origins);
        // Assume goals always numbered sequentially so goalNum and goalNumStr
        // correspond
        qRcd.goalNum = goalNum; 
        qRcd.conclNum = 0;
        qRcd.status = UNCHECKED;

        if (goal->arity() < 2) { // "*** true" goals
            if (option("count-trivial-goals")) {
                qRcd.status = TRIVIAL;
                qRcd.remarks = "trivial goal";
                queryRecords.push_back(qRcd);
            }
            continue; 
        }

        if (option("fuse-concls")) {
            // conclNum == 0 used to signal concls fused
            queryRecords.push_back(qRcd);
        }
        else {  // unfused concls
            Node* concls = goal->child(1);
            for (int conclNum = 1; conclNum <= concls->arity(); conclNum++) {
                qRcd.conclNum = conclNum;
                queryRecords.push_back(qRcd);
            }
        }
    }
    
    //------------------------------------------------------------------------
    // Drive unit queries to solver
    //------------------------------------------------------------------------

    // Allow for results from running a querySet to possibly be
    // shorter than number of non-trivial queries in querySet.  This
    // can occur because
    // - Processing a query throws an exception
    // - querySet processing hits a resource limit, e.g. a timeout.

    int startQuery = 0;
    while (startQuery < (int) queryRecords.size()) {

        // ---------------------------------------------------------------------
        // Set range of queries to drive
        // ---------------------------------------------------------------------
        // Range is [startQuery,endQuery]

        // If don't want incrementality, just do 1 query. 

        int endQuery;
    
        if (option("exploit-solver-incrementality")) {
            endQuery = queryRecords.size() - 1;
        }
        else {
            endQuery = startQuery;
        }

        if (option("gtick")) {
            if (option("longtick")) {
                if (startQuery == endQuery) {
                    cout << " " << queryRecords.at(startQuery).goalNum;
                }
                else {
                    cout << " "
                         << queryRecords.at(startQuery).goalNum
                         << "-"
                         << queryRecords.at(endQuery).goalNum;
                }
            } else {
                cout << ";";
            }
            cout.flush();
        }

        // ---------------------------------------------------------------------
        // Drive queries and collect results
        // ---------------------------------------------------------------------

        vector<QueryStatus> queryResults
            = driveQuerySet(unitInfo, unit, startQuery, endQuery);
        
        int currentQuery;
        int currentResult = 0;

        for (currentQuery = startQuery;
             currentQuery <= endQuery;
             currentQuery++) {

            // Skip over any trivial queries
            if (queryRecords.at(currentQuery).status == TRIVIAL) continue;

            // Quit loop if have run out of results to record.

            if (currentResult >= (int) queryResults.size()) break;

            // Copy current result into query table
            queryRecords.at(currentQuery).status
                = queryResults.at(currentResult).status;
            
            appendCommaString(queryRecords.at(currentQuery).remarks,
                              queryResults.at(currentResult).remarks);

            currentResult++;
        }
            
        // Redo last query if it didn't get use of whole of resource
        // allowances.
        if (resourceLimitsForQuerySet()
            && queryResults.size() > 1
            && queryResults.back().status == RESOURCE_LIMIT) {

            currentQuery--;
        }

        assert(currentQuery > startQuery);

        startQuery = currentQuery;
    }
        
    //------------------------------------------------------------------------
    // Write results from query record table to  VCT file.
    //------------------------------------------------------------------------

    for (int i = 0; i != (int) queryRecords.size(); i++) {

        // update globals
        currentGoalNumStr = intToString(queryRecords.at(i).goalNum);
        currentConcl = queryRecords.at(i).conclNum;

        string status;
        switch (queryRecords.at(i).status) {
        case(TRUE):
        case(TRIVIAL):
            status = "true";
            trueConcls++;
            break;
        case(UNPROVEN):
            status = "unproven";
            unprovenConcls++;
            break;
        case(RESOURCE_LIMIT):
            status = "unproven";
            unprovenConcls++;
            timeoutConcls++;
            break;
        case(ERROR):
            status = "error";
            unprovenConcls++;
            timeoutConcls++;
            break;
        case(UNCHECKED):
            status = "error";
            unprovenConcls++;
            printMessage(ERRORm, "Found unchecked query");
            break;

        }
        printCSVRecordAux(queryRecords.at(i).unitKind,
                          queryRecords.at(i).origins,
                          intToString(queryRecords.at(i).goalNum),
                          queryRecords.at(i).conclNum,
                          status,
                          queryRecords.at(i).time,
                          queryRecords.at(i).remarks);
    } // END for
    return;
}
