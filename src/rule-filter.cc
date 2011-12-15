//==========================================================================
//==========================================================================
// RULE-FILTER.CC
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

#include "rule-filter.hh"

#include <utility>
using std::make_pair;
using std::pair;

#include <iostream>
using std::cout;
using std::endl;

//==========================================================================
// Auxiliary functions
//==========================================================================

template <class K, class V>
void
isectSetIntoMapVal(map<K,set<V> >& m,
                   const K& key, 
                   set<V>& vals) {

    // C++ parser requires the typename here.  Without it, the parser
    // doesn't recognise map<K,set<V> >::iterator as a type.

    typedef typename map<K,set<V> >::iterator iter;

    pair<iter, bool> p = m.insert(make_pair(key,vals));
    // pair<map<K,set<V> >::iterator, bool> p = m.insert(make_pair(key,vals));
    bool insertSuccess = p.second;

    if (insertSuccess) return;

    set<V>& mapVals =  (p.first)->second;
    set<V> newVals;
    setIsect(mapVals, vals, newVals);
    mapVals = newVals;
    return;
}


//==========================================================================
// Drive single query
//==========================================================================

vector<SMTDriver::QueryStatus> 
RuleFilter::driveQuerySet(UnitInfo* unitInfo,
                         Node* unit,
                         set<int> excludedRules,
                         int startQuery,
                         int endQuery) {

    vector<QueryStatus> normalResult
        = SMTDriver::driveQuerySet(unitInfo,
                                   unit,
                                   excludedRules,
                                   startQuery,
                                   endQuery);


    if (! (option("find-redundant-rules")
           && startQuery + 1 == endQuery
           && unitInfo->unitRLURulesEnd > 0
           && normalResult.size() > 0
           && normalResult.at(0).status == TRUE)
        ) {
        return normalResult;
    }
    printMessage(INFOm,"Starting rule filtering on goal");

    string queryTime(normalResult.at(0).time);

    // Record translation from unit and directory rlu rule numbers to names.
    saveRuleNames(unitInfo, unit);

    // Try excluding single rules in range [0 .. unitRLUFileEnd - 1]
    // that are not already excluded because of unbound ids.

    set<int> newExclRules(excludedRules);
    
    for (int i = 0; i != unitInfo->unitRLURulesEnd; i++) {

        if (setMember(i, excludedRules)) continue;

        set<int> exclRules1Extra(newExclRules);
        exclRules1Extra.insert(i);

        vector<QueryStatus> result
            = SMTDriver::driveQuerySet(unitInfo,
                                       unit,
                                       exclRules1Extra,
                                       startQuery,
                                       startQuery+1);

        if (result.size() > 0 && result.at(0).status == TRUE) {
            newExclRules.insert(i);
            queryTime = result.at(0).time;
        }
    }

    saveExclusionInfo(unitInfo, newExclRules);


    if (option("report-excluded-rlu-rules-per-goal")) {
        Node* rules = unit->child(1);
        string report("RuleFilter: excluded RLU rules for goal");
        report.append(ENDLs);
        report.append("(** = undecl ids, s, -- = absence doesn't falsify goal)");
        report.append(ENDLs);
        for (int i = 0; i != unitInfo->unitRLURulesEnd; i++) {
            string ruleName(rules->child(i)->id);
            if (setMember(i,excludedRules))
                report.append("** ");
            else if  (setMember(i,newExclRules))
                report.append("-- ");
            else
                report.append("   ");
            report.append(ruleName);
            report.append(ENDLs);
        }
        printMessage(INFOm, report);
    }
    vector<QueryStatus> result;
    result.push_back(QueryStatus(TRUE,"",queryTime));
    return result;
    
}
 

void
RuleFilter::saveExclusionInfo(UnitInfo* unitInfo, set<int> exclRules) {

    // Partition excluded rules into sets for dir and unit rules files.
    set<int> dirExclRules;
    set<int> unitExclRules;
    for (set<int>::iterator i = exclRules.begin();
         i != exclRules.end();
         i++) {

        if (*i >= unitInfo->dirRLURulesEnd) {

            // Adjust rule # so have index relative to start of dirRLU
            // file start
            unitExclRules.insert(*i - unitInfo->dirRLURulesEnd);
        }
        else {
            dirExclRules.insert(*i);
        }
    }
    // Save info.  Index dir rules by unit path + dir name,
    // unit rules by unit name.

    // if (dirExclRules.size() > 0)
    if (unitInfo->dirRLURulesEnd > 0)
    {
        isectSetIntoMapVal(unrequiredDirRules,
                           joinPaths(unitInfo->getUnitPath(),
                                     unitInfo->getUnitDirName()),
                           dirExclRules);
    }

    // if (unitExclRules.size() > 0)
    if (unitInfo->unitRLURulesEnd > unitInfo->dirRLURulesEnd)
    {
        isectSetIntoMapVal(unrequiredUnitRules,
                           unitInfo->getUnitName(),
                           unitExclRules);
    }
    return;
}

void 
RuleFilter::saveRuleNames(UnitInfo* unitInfo, Node* unit) {
    Node* rules = unit->child(1);
    
    if (unitInfo->dirRLURulesEnd > 0) {

        string dirRLUFileIndex(joinPaths(unitInfo->getUnitPath(),
                                         unitInfo->getUnitDirName()));
        if (dirRLURuleNames.count(dirRLUFileIndex) == 0) {
            vector<string> names;
            for (int i = 0; i < unitInfo->dirRLURulesEnd; i++) {
                names.push_back(rules->child(i)->id);
            }
            dirRLURuleNames.insert(make_pair(dirRLUFileIndex, names));
        }
    }
    if (unitInfo->unitRLURulesEnd > unitInfo->dirRLURulesEnd) {

        string unitRLUFileIndex = unitInfo->getUnitName();
        if (unitRLURuleNames.count(unitRLUFileIndex) == 0) {
            vector<string> names;
            for (int i = unitInfo->dirRLURulesEnd;
                 i < unitInfo->unitRLURulesEnd; i++) {
                names.push_back(rules->child(i)->id);
            }
            unitRLURuleNames.insert(make_pair(unitRLUFileIndex, names));
        }
    }
    return;

}


void
RuleFilter::finaliseSession() {
    // Write results to log file.
    // For each package RLU file, report which rules not needed
    // For each unit RLU file, report which rules not needed

    if (!option("find-redundant-rules")) return;
    
    int totalDirRLURules = 0;
    int totalUnrequiredDirRLURules = 0;
    int totalUnitRLURules = 0;
    int totalUnrequiredUnitRLURules = 0;
    
    for (map<string, set<int> >::iterator i = unrequiredDirRules.begin();
         i != unrequiredDirRules.end();
         i++) {

        string dirRLURuleIndex = i->first;
        set<int> unreqDirRules = i->second;

        vector<string>& dirRuleNameTable
            = dirRLURuleNames.find(dirRLURuleIndex)->second;

        totalDirRLURules += dirRuleNameTable.size();
        totalUnrequiredDirRLURules += unreqDirRules.size();
        
        logStream << endl << endl;
        logStream << "Unrequired rules in package RLU file "  << endl
                  << dirRLURuleIndex << ".rlu" 
                  << "(" << dirRuleNameTable.size() << " rules):"
                  << endl;
        for (set<int>::iterator j = unreqDirRules.begin();
             j != unreqDirRules.end();
             j++) {
            int unreqRuleNum = *j;
            logStream << "  ";
            if (unreqRuleNum < (int) dirRuleNameTable.size())
                logStream << dirRuleNameTable.at(unreqRuleNum);
            else
                logStream << "Out of range rule number: " << unreqRuleNum;
            logStream << endl;
        }
        logStream << endl;
        
    }

    for (map<string, set<int> >::iterator i = unrequiredUnitRules.begin();
         i != unrequiredUnitRules.end();
         i++) {

        string unitRLURuleIndex = i->first;
        set<int> unreqUnitRules = i->second;

        vector<string>& unitRuleNameTable
            = unitRLURuleNames.find(unitRLURuleIndex)->second;
        
        totalUnitRLURules += unitRuleNameTable.size();
        totalUnrequiredUnitRLURules += unreqUnitRules.size();

        logStream << endl << endl;
        logStream << "Unrequired rules in program unit RLU file " << endl
                  << unitRLURuleIndex << ".rlu"
                  << " (" << unitRuleNameTable.size() << " rules):"
                  << endl;
        for (set<int>::iterator j = unreqUnitRules.begin();
             j != unreqUnitRules.end();
             j++) {
            int unreqRuleNum = *j;
            logStream << "  ";
            if (unreqRuleNum < (int) unitRuleNameTable.size())
                logStream << unitRuleNameTable.at(unreqRuleNum);
            else
                logStream << "Out of range rule number: " << unreqRuleNum;
            logStream << endl;
        }
        logStream << endl;
        
    }
    logStream << "Package rules (Unrequired/Total): "
              << totalUnrequiredDirRLURules << "/"
              << totalDirRLURules << endl;
    logStream << "Program unit rules (Unrequired/Total): "
              << totalUnrequiredUnitRLURules << "/"
              << totalUnitRLURules << endl;
    return;
}
