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



//==========================================================================
// Drive Goal
//==========================================================================

void
SMTDriver::driveGoal(Node* decls,
                     Node* rules,
                     Node* hyps,
                     Node* concl,
                     UnitInfo* unitInfo,
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
          
    initGoal(unitInfo->getUnitName(), goalNum, currentConcl);

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

                Node* hyp = hyps->child(currentHyp-1);

                string currentHypStr;
                if (hyp->kind == z::HYP) {
                    currentHypStr = hyp->id;
                    hyp = hyp->child(0);
                }
                else {
                    currentHypStr = "H" + intToString(currentHyp);
                }


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
        sessionInfo.errorConcls++;
        finishSetup();
        finaliseGoal();
        return;
    }

    finishSetup();

    goalTimer.sample();
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
            
    bool checkError = true;

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
        sessionInfo.trueConcls++;

    } else if (s == UNKNOWN) {

        printCSVRecord("unproven", remarks);
        sessionInfo.unknownConcls++;

    } else if (s == FALSE) {

        appendCommaString(remarks,"false");
        printCSVRecord("unproven", remarks);
        sessionInfo.falseConcls++;

    } else if (s == RESOURCE_LIMIT) {

        printCSVRecord("unproven", remarks);
        sessionInfo.timeoutConcls++;

    } else {// s == ERROR

        printCSVRecord("error", remarks);
        sessionInfo.errorConcls++;
    }
    finaliseGoal();
}

//==========================================================================
// Drive Unit
//==========================================================================

void
SMTDriver::driveUnit(Node* unit, UnitInfo* unitInfo) {

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
                printCSVRecord("trivial","");
                sessionInfo.trivialConcls++;
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

            if (! unitInfo->include(goalNum, currentConcl)) {
                goalSliceTime = "0";
                printCSVRecord("unproven", "excluded");
                sessionInfo.unknownConcls++;
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
SMTDriver::checkGoal(string& remarks) {
    appendCommaString(remarks, "no prover");
    return false;
}

bool 
SMTDriver::runQuerySet(string& remarks) {
    appendCommaString(remarks, "no prover");
    return false;
}

SMTDriver::Status
SMTDriver::getResults(string& remarks) { return UNKNOWN; }


vector<SMTDriver::QueryStatus>
SMTDriver::getRunResults(int numQueries) {
    return vector<QueryStatus>();
}

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
// Drive queries queryRecords[startQuery..endQuery-1] 

// It is important that this method is idempotent, that calling it
// multiple times on the same queries produces a result and has
// side-effects similar to a call of it a single time on the queries.
// It gets called multiple times on the same queries both in rule
// filtering and when timeouts are encountered in drives of multiple
// queries

vector<SMTDriver::QueryStatus> 
SMTDriver::driveQuerySet(UnitInfo* unitInfo,
                         Node* unit,
                         set<int> excludedRules,
                         int startQuery,
                         int endQuery) {

    string remarks(queryRecords.at(startQuery).remarks);

    {
        int startGoalNum  = queryRecords.at(startQuery).goalNum;
        int endGoalNum    = queryRecords.at(endQuery-1).goalNum;
        int startConclNum = queryRecords.at(startQuery).conclNum;
        int endConclNum   = queryRecords.at(endQuery-1).conclNum;

        // Set globals used in messages
        currentGoalNumStr = intToString(startGoalNum);
        currentConcl = startConclNum;

        // Set up header of query set
        initQuerySet(unitInfo->getUnitName(), startGoalNum, currentConcl);

        if (option("gtick")) {
            if (option("longtick")) {
                if (startQuery + 1 == endQuery || onlineInterface()) {
                    cout << " " << startGoalNum;
                    if (option("ctick"))
                        cout << "." << startConclNum;
                }
                else {
                    cout << " " << startGoalNum;
                    if (option("ctick")) cout << "." << startConclNum;
                    cout << "-" << endGoalNum;
                    if (option("ctick")) cout << "." << endConclNum;
                }
            } else {
                if (option("ctick")) {
                    if (startConclNum == 1)
                        cout << ";.";
                    else 
                        cout << ".";
                }
                else {
                    cout << ";";
                }
            }
            cout.flush();
        }
    }

    // ---------------------------------------------------------------------
    // Break out unit components
    // ---------------------------------------------------------------------

    Node* decls = unit->child(0);
    Node* rules = unit->child(1);
    Node* goals = unit->child(2);

    vector<QueryStatus> results;

    string declsRulesRemarks;

    Timer queryTimer;

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

        for ( int r = 0; r < rules->arity(); r++) {

            if (setMember(r, excludedRules)) continue;
            
            Node* rule = rules->child(r);
            string currentRuleStr;
            if (rule->kind == z::RULE) {
                currentRuleStr = rule->id;
                rule = rule->child(0);
            }
            else {
                currentRuleStr = "R" + intToString(r+1);
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

        declsRulesRemarks = remarks;
        
        // ---------------------------------------------------------------------
        // Loop over queries of queryset
        // ---------------------------------------------------------------------
        
        for (int query = startQuery; query < endQuery; query++) {

            remarks = declsRulesRemarks;
            
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

            // When in incremental mode, ensure we only record setup time
            // for new hyps and concls for queries after the start.

            if (onlineInterface()
                && option("gstime-inc-setup")
                && query > startQuery) {

                queryTimer.restart();
            }

            if (option("gtick")
                && onlineInterface()
                && query > startQuery) {
                
                if (option("longtick")) {
                    cout << " " << goalNum;
                    if (option("ctick"))
                        cout << "." << conclNum;

                }
                else if (option("ctick")) {
                    if (conclNum == 1)
                        cout << ";.";
                    else 
                        cout << ".";
                }
                else {
                    cout << ";";
                }
                cout.flush();
            }

            // -----------------------------------------------------------
            // Push new empty assertion set onto assertion set stack
            // -----------------------------------------------------------

            if (option("exploit-solver-incrementality")) push();

            // -----------------------------------------------------------
            // Push hyps
            // -----------------------------------------------------------

            if (! option("skip-hyps") ) {

                for (int currentHyp = 1;
                     currentHyp <= hyps->arity();
                     currentHyp++) {

                    Node* hyp = hyps->child(currentHyp-1);

                    string currentHypStr;
                    if (hyp->kind == z::HYP) {
                        currentHypStr = hyp->id;
                        hyp = hyp->child(0);
                    }
                    else {
                        currentHypStr = "H" + intToString(currentHyp);
                    }

                    Formatter::setFormatter
                        (VanillaFormatter::getFormatter());
                    printMessage(FINESTm, 
                                 "Writing " + currentHypStr + ENDLs
                                 + hyp->toString()
                        );

                    addHyp(hyp, currentHypStr, remarks);

                } // END: for each hyp in hyps

            } // END: if !skip-hyps option

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

            if (onlineInterface()
                && !option("gstime-inc-setup")) {

                queryTimer.restart();
            }

            Status status =
                option("skip-check") ? UNCHECKED : check(remarks);
  
            double queryTime = 0.0;
            if (onlineInterface()) {
                queryTimer.sample();
                queryTime = queryTimer.getTime();
            }
        
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
                results.push_back(QueryStatus(status,remarks,queryTime));
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

        appendCommaString(remarks, "exception in driving query set");

        finaliseQuerySet();

        results.push_back(QueryStatus(ERROR,remarks,0));

        return results;
     
    }

    // ---------------------------------------------------------------------
    // Run query set offline on solver
    // ---------------------------------------------------------------------
    // The handling of recording the time here will need revisiting
    // if the run results returned by getRunResults actually include
    // run times for each query in a query set.

    // The solution here records the time for running the whole query
    // set in the results for the 1st query in the set, leaving the
    // time at 0 for the results for the remaining queries in the set.

    if (!onlineInterface()) {

        // With file level interface, write file.
        outputQuerySet();

        queryTimer.restart();
        
        bool runError = runQuerySet(remarks);

        queryTimer.sample();
        double queryTime = queryTimer.getTime();

        if (runError) {
            printMessage(ERRORm, "Error flagged on run of solver\n");
        }
        results = getRunResults(endQuery - startQuery);

        if (results.size() > 0) results.at(0).time = queryTime;

        // Merge in declsRulesRemarks with any remarks coming from
        // solver.  This isn't ideal: we lose possible remarks coming
        // from offline push of hyps and concl and check.  However,
        // such remarks are hopefully unlikely.  Are none at time of
        // writing this comment.

        for (int i = 0; i != (int) results.size(); i++) {
            string r(declsRulesRemarks);
            appendCommaString(r, results.at(i).remarks);
            results.at(i).remarks = r;
        }
        
    }

    finaliseQuerySet();
    return results;
}


//==========================================================================
// Alternative Drive Unit
//==========================================================================

void
SMTDriver::altDriveUnit(Node* unit, UnitInfo* unitInfo) {

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
    // Initialise query and result tables
    //------------------------------------------------------------------------
    queryRecords.clear();
    resultRecords.clear();
    
    Node* goals = solverUnit->child(2);

    for (int goalNum = 1; goalNum <= goals->arity(); goalNum++) {

        if (option("goal") && intOptionVal("goal") != goalNum) {
            continue;
        }
        Node* goal = goals->child(goalNum-1);

        // Set up result record for goal / goal slices
        // Initialise all fields except queryNum and conclNum.
        ResultRecord rRcd;
        rRcd.goalNum = goalNum; 

        if (option("do-rule-audit")) {
            rRcd.origins = goal->id;
        } else {
            string goalNumStr;
            extractGoalInfo(goal->id,
                            rRcd.unitKind,
                            goalNumStr,
                            rRcd.origins);

            if (intToString(goalNum) != goalNumStr) {
                // Set currentGoalNumStr and currentConcl globals used by
                // printMessage
                currentGoalNumStr = goalNumStr;
                currentConcl = 0;
                printMessage(WARNINGm, "Mismatch between goal position "
                             + intToString(goalNum)
                             + " and numbering " + goalNumStr);
            }
        }
        if (goal->arity() < 2) { // "*** true" goals
            if (option("count-trivial-goals")) {
                rRcd.queryNum = -1;
                resultRecords.push_back(rRcd);
            }
            continue; 
        }
        // goal non-trivial


        // Customise query and result records for goal / goal slices and save
        Node* concls = goal->child(1);

        int fromConcl = 1; 
        int toConcl   = concls->arity();

        if (option("fuse-concls")) {
            fromConcl = 0;
            toConcl   = 0;
        }
        else if (option("concl")) {
            fromConcl = intOptionVal("concl");
            toConcl   = intOptionVal("concl");
        }

        for (int conclNum = fromConcl; conclNum <= toConcl; conclNum++) {

            rRcd.conclNum = conclNum;
            
            if (unitInfo->include(goalNum, conclNum)) {

                //  Set up query record for goal / goal slices
                QueryRecord qRcd;
                qRcd.goalNum = goalNum;
                qRcd.time = 0.0;
                qRcd.status = UNCHECKED;
                qRcd.conclNum = conclNum;
                queryRecords.push_back(qRcd);

                rRcd.queryNum = (int) queryRecords.size() - 1;
                resultRecords.push_back(rRcd);
            }
            else {
                rRcd.queryNum = -2;
                resultRecords.push_back(rRcd);
            }

        }
    }

    //------------------------------------------------------------------------
    // Drive unit queries to solver
    //------------------------------------------------------------------------
    // Allow for results from running a querySet to possibly be
    // shorter than number of queries in querySet.  This can occur
    // because

    // - Processing a query throws an exception
    // - Processing a querySet hits a resource limit, e.g. a timeout, for
    //   some particular query or for a prefix of the querySet.

    // Only references query table, not results table.

    int startQuery = 0;
    while (startQuery < (int) queryRecords.size()) {

        // ---------------------------------------------------------------------
        // Set range of queries to drive
        // ---------------------------------------------------------------------
        // Range is [startQuery,endQuery-1]

        // If don't want incrementality, just do 1 query. 

        int endQuery;
    
        if (option("exploit-solver-incrementality")) {
            endQuery = queryRecords.size();
        }
        else {
            endQuery = startQuery + 1;
        }

        // ---------------------------------------------------------------------
        // Drive queries and collect results
        // ---------------------------------------------------------------------

        vector<QueryStatus> queryResults
            = driveQuerySet(unitInfo,
                            solverUnit,
                            unitInfo->getExcludedRules(),
                            startQuery,
                            endQuery);

        int numQueryResultsToProcess = queryResults.size();
        if (numQueryResultsToProcess > endQuery - startQuery) {
            // Issue seen with Alt Ergo v0.94
            if (!option("ignore-extra-query-results")) {
                printMessage(ERRORm,
                             "Solver returned "
                             + intToString(numQueryResultsToProcess)
                             + " query results, but only expecting "
                             + intToString(endQuery - startQuery));
            }

            numQueryResultsToProcess = endQuery - startQuery;
        }

        // Copy current result into query table
        for (int qr = 0;
             qr != numQueryResultsToProcess;
             qr++) {

            int currentQuery = startQuery + qr;

            queryRecords.at(currentQuery).status
                = queryResults.at(qr).status;
            
            queryRecords.at(currentQuery).remarks
                = queryResults.at(qr).remarks;

            queryRecords.at(currentQuery).time
                = queryResults.at(qr).time;

        }
        startQuery = startQuery + numQueryResultsToProcess;

        // Move on one query if get back no results.
        if (numQueryResultsToProcess == 0) {
            startQuery++;
        }
        // Redo last query if it didn't get use of whole of resource
        // allowance.
        else if (resourceLimitsForQuerySet()
            && numQueryResultsToProcess > 1
            && queryResults.back().status == RESOURCE_LIMIT) {

            startQuery--;
        }
    }
        
    //------------------------------------------------------------------------
    // Write results from query and results tables to VCT file.
    //------------------------------------------------------------------------

    for (int i = 0; i != (int) resultRecords.size(); i++) {

        int queryNum = resultRecords.at(i).queryNum;

        string resultStatus;
        double time = 0.0;
        string remarks;

        // currentGoalNumStr and currentConcl are globals from utility.cc
        currentGoalNumStr = intToString(resultRecords.at(i).goalNum);

        if (queryNum == -1) {
            resultStatus = "trivial";
            unitInfo->trivialGoals++;
        }
        else if (queryNum == -2) {
            remarks = "excluded goal/goal slice";
            resultStatus = "unproven";
            unitInfo->excludedConcls++;
        }
        else {
            currentConcl = queryRecords.at(queryNum).conclNum;
            time = queryRecords.at(queryNum).time;
            remarks = queryRecords.at(queryNum).remarks;

            switch (queryRecords.at(queryNum).status) {
            case(TRUE):
                resultStatus = "true";
                unitInfo->trueQueries++;
                unitInfo->provenQueriesTime += time;
                if (time > unitInfo->maxProvenQueryTime) {
                    unitInfo->maxProvenQueryTime = time;
                }
                if (option("do-rule-audit")) {
                    string origins = resultRecords.at(i).origins;
                    if (hasPrefix(origins,"no urules"))
                        unitInfo->inconsistentSysRuleSets++;
                    if (hasPrefix(origins,"all urules"))
                        unitInfo->inconsistentUserRuleSets++;
                    if (hasPrefix(origins,"urule as H"))
                        unitInfo->inconsistentUserRules++;
                    if (hasPrefix(origins,"urule as C"))
                        unitInfo->derivableUserRules++;
                    if (hasPrefix(origins,"urule from rest"))
                        unitInfo->interdependentUserRules++;
                }
                break;
            case(UNKNOWN):
                resultStatus = "unproven";
                unitInfo->unknownQueries++;
                unitInfo->unprovenQueriesTime += time;
                break;
            case(FALSE):
                resultStatus = "unproven";
                appendCommaString(remarks, "false");
                unitInfo->falseQueries++;
                unitInfo->unprovenQueriesTime += time;
                break;
            case(RESOURCE_LIMIT):
                resultStatus = "unproven";
                unitInfo->timeoutQueries++;
                unitInfo->unprovenQueriesTime += time;
                break;
            case(ERROR):
                resultStatus = "error";
                unitInfo->errorQueries++;
                break;
            case(UNCHECKED):
                resultStatus = "error";
                unitInfo->errorQueries++;
                printMessage(ERRORm, "Found unchecked query");
                break;
            }
        }
        
        printCSVRecordAux(resultRecords.at(i).unitKind,
                          resultRecords.at(i).origins,
                          intToString(resultRecords.at(i).goalNum),
                          resultRecords.at(i).conclNum,
                          resultStatus,
                          doubleToFixPtString(time,3),
                          remarks);
    } // END for

    // Update stats for whole session

    sessionInfo.update(unitInfo);
    return;
}
