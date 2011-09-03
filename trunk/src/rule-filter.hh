//==========================================================================
//==========================================================================
// RULE-FILTER.HH
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

// Figure out which user-supplied rules are not needed by SMT solver.


#ifndef RULE_FILTER_HH
#define RULE_FILTER_HH

#include "smt-driver.hh"
#include "utility.hh"

#include <map>
using std::map;


class RuleFilter : public SMTDriver {

private:

    map<string, set<int> > unrequiredUnitRules;
    map<string, set<int> > unrequiredDirRules;

    map<string, vector<string> > unitRLURuleNames;
    map<string, vector<string> > dirRLURuleNames;

    Status driveQuery(UnitInfo* unitInfo,
                      Node* unit,
                      set<int> excludedRules,
                      int startQuery);

protected:
    virtual vector<QueryStatus> driveQuerySet(UnitInfo* unitInfo,
                                              Node* unit,
                                              set<int> excludedRules,
                                              int startQuery,
                                              int endQuery);

    void saveExclusionInfo(UnitInfo* unitInfo, set<int> exclRules);
    void saveRuleNames(UnitInfo* unitInfo, Node* unit);
    
    virtual void finaliseSession();

};




#endif // ! RULE_FILTER_HH
