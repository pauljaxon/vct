//==========================================================================
//==========================================================================
// YICES-DRIVER.HH
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

#ifndef YICES_DRIVER_HH
#define YICES_DRIVER_HH

extern "C" {

#include "yicesl_c.h"

}

#include "smt-driver.hh"


// Interface for Yices


class YicesDriver : public SMTDriver {
private:

    yicesl_context ctx;
    Status status;

    bool push(yicesl_context ctx,
              Node* n,
              string& input,
              string& output);

    string formatErrorString(const string& input, const string& output);
    bool ignoreErrorMessage(const string& s);
    
public:

    YicesDriver() {};
    
    virtual void initSession();

protected:

    virtual Node* translateUnit(Node* unit);

    virtual void initGoal(const string& unitName,
                          int goalNum,
                          int ConclNum);

    virtual void addDecl(Node* n);
    virtual void addHyp(Node* h, const string& hId, string& remarks);
    virtual void addConcl(Node* n, string& format); 

    //    virtual void finishSetup();

    virtual bool checkGoal(string& format);
    
    virtual Status getResults(string& remarks);

    virtual void finaliseGoal();

    // virtual void finaliseSession()
};


#endif // ! YICES_DRIVER_HH
