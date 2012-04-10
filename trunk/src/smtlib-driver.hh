//==========================================================================
//==========================================================================
// SMTLIB-DRIVER.HH
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

#ifndef SMTLIB_DRIVER_HH
#define SMTLIB_DRIVER_HH

#include <fstream>
using std::ifstream;
using std::ofstream;

#include "rule-filter.hh"

// Interface for SMTLib 1.2 format file-level solver interface.


class SMTLibDriver : public RuleFilter { // RuleFilter is a subclass
                                         // of SMTDriver
public:

    SMTLibDriver() {};
    
private:

    string solverInputFileName;
    string solverOutputFileName;
    string solverErrorFileName;

    int exitStatus;

    Node* benchmark;
    Node* formula;

protected:
    // OLD = Needed by old prover driver 
    // NEW = Needed by alternative prover driver

    virtual Node* translateUnit(Node* n);

    virtual bool onlineInterface() {return false; }

    virtual void initGoal(const string& unitName,
                          int goalNum,
                          int ConclNum);

    virtual void addDecl(Node* n);
    virtual void addRule(Node* h, const string& hId, string& remarks);
    virtual void addHyp(Node* h, const string& hId, string& remarks);
    virtual void addConcl(Node* n, string& remarks); 

    virtual Status check(string& remarks);  // NEW
    virtual void outputQuerySet();          // NEW

    virtual void finishSetup();     // OLD



    virtual bool checkGoal(string& remarks);    // OLD
    virtual bool runQuerySet(string& remarks);  // NEW
    
    virtual Status getResults(string& remarks);         // OLD
    vector<QueryStatus> getRunResults(int numQueries);  // NEW


    virtual void finaliseGoal();

};


#endif // ! SMTLIB_DRIVER_HH
