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

// Solver goals correspond to either FDL goal slices or FDL goals, depending
// on mode of operation.


// Solver drivers inheriting from SMTDriver should only define those
// virtual functions they need to override.


#ifndef SMT_DRIVER_HH
#define SMT_DRIVER_HH

#include "node.hh"
#include "utility.hh"  // For UnitInfo decl



class SMTDriver {

public:
    enum Status { TRUE, UNPROVEN, ERROR, RESOURCE_LIMIT };

    void driveUnit(Node* unit, UnitInfo unitInfo);

    // Virtual functions for solver-specific interfaces

    virtual ~SMTDriver() {};

    virtual void initSession() {};

private:
    static Timer goalTimer;

    void driveGoal(Node* decls,
                   Node* rules,
                   Node* hyps,
                   Node* concl,
                   UnitInfo unitInfo,
                   int goalNum,
                   int currentConcl);

protected:
    virtual Node* translateUnit(Node* n);

    virtual void initGoal(const string& unitName,
                          int goalNum,
                          int conclNum) {}; // Called at start of goal / 
                                            // goal slice

    // add* methods should throw std::runtime_error if problems and include
    // explanation for output to log file in exception argument.
    virtual void addDecl(Node* n) {};
    virtual void addHyp(Node* h, const string& hId, string& remarks) {};
    virtual void addRule(Node* h, const string& hId, string& remarks) {
        addHyp(h, hId, remarks);
    };

    virtual void addConcl(Node* n, string& remarks) {};  // Only called once

    virtual void finishSetup() {};  // Called after addConcl or on throw of
                                    // exception by any of add methods.

    virtual bool checkGoal(string& remarks); // Should return true if problems.
                                             // Any exceptions are uncaught
                                             // Method must write log file
                                             // with explanation.
    
    virtual Status getResults(string& remarks);

    virtual void finaliseGoal() {}; // Called at end of goal / goal slice
                                    // both in normal and error cases.

public:
    virtual void finaliseSession() {};


};




#endif // ! SMT_DRIVER_HH
