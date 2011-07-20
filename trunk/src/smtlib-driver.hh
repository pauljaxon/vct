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

#include "smt-driver.hh"

// Interface for SMTLib format file-level solver interface.


class SMTLibDriver : public SMTDriver {
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

    virtual Node* translateUnit(Node* n);

    virtual void initGoal(const string& unitName,
                          int goalNum,
                          int ConclNum);

    virtual void addDecl(Node* n);
    virtual void addRule(Node* h, const string& hId, string& remarks);
    virtual void addHyp(Node* h, const string& hId, string& remarks);
    virtual void addConcl(Node* n, string& remarks); 

    virtual void finishSetup();

    virtual bool checkGoal(string& remarks);
    
    virtual Status getResults(string& remarks);

    virtual void finaliseGoal();

};


#endif // ! SMTLIB_DRIVER_HH
