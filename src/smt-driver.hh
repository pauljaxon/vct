//==========================================================================
//==========================================================================
// SMT-DRIVER.HH
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

// Define generic interface for SMT solvers and a uniform procedure
// for processing a unit using this interface.

// Terminology:

// Query:  Either a `goal' (Verification Condition) comprising
//         Decls + rules + hyps of VC + concls of VC
//         or a `goal slice'
//         Decls + rules + hyps + 1 of concls of VC
//
// QuerySet: Set of related queries, all from the same Unit, so sharing
//         Decls and rules.
//
// Session: Collection of all solver interactions during one invocation of
//         Victor.
// 

// Solver drivers inheriting from SMTDriver should only define those
// virtual functions they need to override.


#ifndef SMT_DRIVER_HH
#define SMT_DRIVER_HH

#include "node.hh"
#include "utility.hh"  // For UnitInfo decl

// FILE: prefix for comment about use with offline file interface to solver
// API: prefix for comment about use with  online API interface 

class SMTDriver {

public:
    enum Status { TRUE, UNPROVEN, ERROR, RESOURCE_LIMIT, UNCHECKED };

    class QueryStatus {
    public:
        Status status;
        string remarks;
        string time;
        QueryStatus(Status s, const string& r, const string& t) :
            status(s), remarks(r), time(t) {};
    };

    class QueryRecord {
    public:
        int goalNum;   //  1-based
        int conclNum;  //  1-based.  0 if concls fused
        Status status;
        string time;
        string remarks;
    };

    class ResultRecord {
    public:
        string unitKind;
        string origins;
        int goalNum;  // Used for trivial goals. o/w copy of queryNum.goalNum
        int queryNum; // -1 for trivial goal ("*** true")
    };

    vector<QueryRecord> queryRecords;
    vector<ResultRecord> resultRecords;
    
    void driveUnit(Node* unit, UnitInfo* unitInfo);

    // new alternative driver.
    void altDriveUnit(Node* unit, UnitInfo* unitInfo);

    // Auxiliary function used by altDriveUnit.
    // Overridden in RuleFilter child class.
    
    virtual vector<QueryStatus> driveQuerySet(UnitInfo* unitInfo,
                                              Node* unit,
                                              set<int> excludedRules,
                                              int startQuery,
                                              int endQuery);



    // Virtual functions for solver-specific interfaces

    virtual ~SMTDriver() {};

    virtual void initSession() {};

private:
    Timer goalTimer;

    void driveGoal(Node* decls,
                   Node* rules,
                   Node* hyps,
                   Node* concl,
                   UnitInfo* unitInfo,
                   int goalNum,
                   int currentConcl);

protected:
    virtual Node* translateUnit(Node* n);

    virtual bool onlineInterface() {return false; }; // FILE: return false
                                                     // API:  return true

    virtual bool resourceLimitsForQuerySet() {return false; }; 
           // Return true if resource limits can be set only for
           // processing of whole query set, not individual queries.
           // E.g. this is the case for FILE level interfaces where
           // shell-level ulimit command is used to set limits.


    virtual void initGoal(const string& unitName,
                          int goalNum,
                          int conclNum) {}; 
                              // Called at start of goal / goal slice
                              // goalNum and conclNum misleading if
                              // multiple queries.   Need to fix.
                              // OLD

    // Eventually replace initGoal with initQuerySet, since more accurate name
    virtual void initQuerySet(const string& unitName,
                              int goalNum,
                              int conclNum) {
        initGoal(unitName, goalNum, conclNum);
        return;
    }

    // add* methods should throw std::runtime_error if problems are
    // sufficiently severe that processing of current QuerySet needs
    // to be immedidately aborted.  The thrown exception object's
    // string should include an explanation for output to log file.
    // 
    virtual void addDecl(Node* n) {};
    virtual void addHyp(Node* h, const string& hId, string& remarks) {};
    virtual void addRule(Node* h, const string& hId, string& remarks) {
        addHyp(h, hId, remarks);
    };

    virtual void addConcl(Node* n, string& remarks) {};  // Only called once
    
    virtual void push() {};  // FILE: add push command
                             //   API:  do push

    virtual Status check(string& remarks);  
                           // FILE: add check command. Return dummy Status. 
                           // API: run check command 

    virtual void pop() {};  // FILE: add pop command
                            // API: o pop

    virtual void finishSetup() {};  // FILE only
                                    // Called after addConcl or on throw of
                                    // exception by any of add methods.
                                    // : Write QuerySet input file for solver
                                    // OLD

    virtual void outputQuerySet() {}; // FILE only

    virtual bool checkGoal(string& remarks); 

                  // Should return true if problems.  Any exceptions
                  // are uncaught Method must write log file with
                  // explanation.
                  // API: run solver
                  // OLD


    virtual bool runQuerySet(string& remarks) {return true;};
                           // FILE: Run solver on query set
                           // API: Unused

    virtual Status getResults(string& remarks);
                  // OLD.  

    virtual vector<QueryStatus> getRunResults(int numQueries) {
        return vector<QueryStatus>();
    }
                                // FILE: Read solver output files 
                                // API: Unused

    virtual void finaliseQuerySet() {
        finaliseGoal();
    }; 
                      // Called at end of query, both in normal and
                      // error cases.  Replaces finaliseGoal()

    virtual void finaliseGoal() {}; // Called at end of goal / goal slice
                                    // both in normal and error cases.
                                    // OLD

public:
    virtual void finaliseSession() {};


};




#endif // ! SMT_DRIVER_HH
