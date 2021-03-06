//==========================================================================
//==========================================================================
// UTILITY.HH
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


#ifndef UTILITY_HH
#define UTILITY_HH

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <utility>
using std::pair;

#include <ctime>

#include <csignal>
#include <csetjmp>

#include <algorithm>

#include <iostream>
#include <fstream>



//========================================================================
// String functions and constants 
//========================================================================

bool isIntString(const string& s);

string intToString(int i);
string uLongToString(unsigned long i);

int stringToInt(const string& s);
unsigned long stringToULong(const string& s);

string boolToString(bool b);

string doubleToFixPtString(double d, int prec);


extern const string ENDLs;

bool hasSuffix(const string& s, const string& t);  // s has suffix t
bool hasPrefix(const string& s, const string& t);  // s has prefix t
bool hasSubstring(const string& s, const string& t);  // s has substring t

void appendCommaString(string& s, const string& t);  // s=="" ? t : "s, t" 

// joinPaths:   if q == "/..." or p == "" then q else p + "/" + q
string joinPaths(const string& p, const string& q);

bool hasUpperCaseStart(const string& s);

bool member(char c, const string& s);  // c is char of s

// returns substring before first occurrence of c. 
// If no occurrence, returns whole string.    

string substringBefore(char c, const string& s);
string substringAfter(char c, const string& s);

vector<string> tokeniseString(const string& s);

string stripWhitespaceEnds(const string& s);

vector<string> splitString(const string& s, const string& delimiter);
string concatStrings (const vector<string>& strs, const string& delimiter);

string fixSuffix(const string& s,
                 const string& oldSuffix,
                 const string& newPrefix,
                 const string& newSuffix);

string mkLispSymbolString(const string& s);

bool stringMatch(const string& pat, const string& inst);

//========================================================================
// Set operations
// ========================================================================
// Wrap the set operations from <algorithm> to make their invocation
// more compact As needed, could make these more generic.

template <class T>
void setUnion(set<T>& a, set<T>& b, set<T>& c) {
    set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(c,c.begin()));
}

template <class T>
void setSymDiff(set<T>& a, set<T>& b, set<T>& c) {
    set_symmetric_difference(a.begin(), a.end(),
                             b.begin(), b.end(),
                             inserter(c,c.begin()));
}

template <class T>
void setIsect(set<T>& a, set<T>& b, set<T>& c) {
    set_intersection(a.begin(), a.end(),
                     b.begin(), b.end(),
                     inserter(c,c.begin()));
}

template <class T>
bool setMember(const T& x, const set<T>& a) {
    return a.find(x) != a.end();
}

//========================================================================
// Vector operations
//========================================================================

template <class T>
bool vectorMember(const T& x, const vector<T>& a) {
    return find(a.begin(), a.end(), x) != a.end();
}

//========================================================================
// Command line option processing
//========================================================================
// Expect options of form:

// -<key>
// -<key>=<val>

// Same key can be given multiple times.

// Records command line options and returns non-option arguments.
vector<string> processCommandArgs (int argc, char *argv[]);  

bool option(const string& s);     // Has option s been set?
// returns false both when option never seen, and when last value is
// one of "false", "default" or "none".

string optionVal(const string& s);// Get last string value of option s

vector<string> optionVals(const string& s); // Get all values provided for s

int intOptionVal(const string& s);// Get int value of option s



//========================================================================
// External executable and timeout
//========================================================================

string withTimeoutAndIO(const string &cmd,
                        const string &inputFileName,
                        const string &outputFileName,
                        const string &errorFileName);
// Returns new command with timeout support


//========================================================================
// Handling information on each unit.
//========================================================================
//

class UnitInfo {
private:
    enum Status {BEFORE_RANGE, IN_RANGE, AFTER_RANGE};
    static Status status;

    // unitName = D1/.../Dk/U   (as supplied on command line or .lis file)
    // program unit name U is w/o .fdl .rls .vcg suffix.
    // unitPathPrefix = 

    int unitNum;
    string unitPathPrefix;// P1/.../Pn    (as supplied with -prefix option)
    string unitName;      // D1/.../Dk/U
    string unitPath;  // D1/.../Dk  ("" if k = 0)
    string unitDirName;   // if k>0 then Dk else if n>0 then Pn else ""
    string unitFileName;  // U
    
    set<int> excludedRules;
    set<pair<int,int> > selectedSet;
    vector<string> declFiles;
    vector<string> ruleFiles;
    bool unitIncluded;
    int startGoal;

public:
    // (With 0-based rule numbering)
    // dir RLU Rules are  [0 .. dirRLURulesEnd-1]  
    // unit RLU Rules are  [dirRLURulesEnd .. unitRLURulesEnd -1]  
    // System rules (from Examiner-generated RLS files, extra Victor
    // RLS files or added during Victor's translation) are 
    // [unitRLURulesEnd .. #Rules - 1]

    int dirRLURulesEnd;
    int unitRLURulesEnd;

    int numExcludedDirRLURules;
    int numExcludedUnitRLURules;
    int numExcludedSystemRules;

    int auditedUserRules;       
                                 
    // Statistics on unit.  

    int parseTreeSize;
    int translatedUnitSize;
    int nodeAllocCount;

    int trivialGoals;
    int trueQueries;
    int unknownQueries;
    int falseQueries;
    int timeoutQueries;
    int errorQueries;
    int excludedConcls;

    int inconsistentSysRuleSets;  
    int inconsistentUserRuleSets;
    int inconsistentUserRules;
    int derivableUserRules;      // Num user rules derivable without help of
                                 // other user rules
    int interdependentUserRules; // Num user rules derivable with help of other
                                 // user rules

    double unitTime;
    double unprovenQueriesTime; 
    double provenQueriesTime; 
    double maxProvenQueryTime; 
    string remarks;

    UnitInfo(int n, const string& s);

    int getUnitNum() {return unitNum;}
    string getUnitPathPrefix() {return unitPathPrefix;}
    string getUnitName() { return unitName;}
    string getUnitPath() {return unitPath;}
    string getUnitDirName() {return unitDirName;}
    string getUnitFileName() {return unitFileName;}

    string getFullUnitName() { //  P1/.../Pn/D1/.../Dk/U
        return joinPaths(unitPathPrefix, unitName);
    }
    set<int> getExcludedRules() {return excludedRules;}
    bool isExcludedRule(int rNum) {return setMember(rNum, excludedRules);}
    void addExcludedRule(int rNum);
    bool isUserRule(int rNum)    {return rNum < unitRLURulesEnd; }
    bool isDirUserRule(int rNum) {return rNum < dirRLURulesEnd; }
    bool isUnitUserRule(int rNum) {
        return dirRLURulesEnd <= rNum && rNum < unitRLURulesEnd;
    }

    vector<string> getDeclFiles() {return declFiles; }
    vector<string> getRuleFiles() {return ruleFiles; }
    bool includeUnit();
    bool include(int goal, int concl);

    void addRemark(const string& s) {
        appendCommaString(remarks, s);
        return;
    }
};

//========================================================================
// Goal origins formatting
//========================================================================

void formatGoalOrigins(const string& s, string& goalNumber, string& origins);

//========================================================================
// Report file management
//========================================================================

extern std::ofstream logStream;

void openReportFiles();
void closeReportFiles();



//========================================================================
// Message reporting
//========================================================================
// Defines:
//
//   printMessage(int messageLevel, const string& message)
//
// Levels are severity levels, the higher the level, the more severe the
// message.

// Only messages with priority level greater than or equal to set
// threshold are output

// Levels are:

extern const int ERRORm;  // Highest
extern const int WARNINGm;
extern const int INFOm;
extern const int FINEm;
extern const int FINERm;
extern const int FINESTm;  // Lowest

// Guide to usage of levels: 

// 1 message per unit  INFO
// tracing phases of unit 4 messages / unit  FINE
// 1 brief message per goal/concl FINER
// details of every transaction.  FINEST



// Message reporting relies on users initialising and maintaining the 
// following global variables:

extern int messageThreshold; 

extern string currentGoalNumStr; 
extern int currentConcl;

extern string currentHypsKinds;
extern string currentConclKinds;

extern string goalSliceTime;

// Functions updating state of global variables used by message reporting

void initCurrentUnitInfo(UnitInfo* unitInfo);
void updateCurrentGoalInfo(const string& s);

// Functional version of updateCurrentGoalInfo
void extractGoalInfo(const string& s,
                     string& unitKind,
                     string& goalNum,
                     string& goalOrigins);



// Primitive used by printMessage, but also callable stand-alone.

void printMessageWithHeader(const string& header, const string& message);

// Macro provided to avoid expense of Node printing when not needed.

// Use of `conditional' expression is a hack so that printMessage can be
// used as expression in expression statement:
//
//   printMessage(.., ..) ;
//
// Condition expressions don't allow void values of 2nd and 3rd args, so
// use dummy int values.

#define printMessage(messageLevel, message) \
    ( (messageThreshold <= (messageLevel))      \
      ?  printMessageAux1((messageLevel), (message)) \
      :  printMessageAux2((messageLevel)) )
     
int printMessageAux1(int messageLevel, const string& message);
int printMessageAux2(int messageLevel);

// Variation where printing is enabled by user-supplied option.
// By convention, use option names starting with P to make them stand
// out and avoid accidental clashes.

#define printOnOption(opt, message) \
      (option(opt) 				     \
      ?  printMessageOnOptionAux(opt, message)	     \
      :  0 )
     
int printMessageOnOptionAux(const string& opt, const string& message);

//========================================================================
// CSV formatting 
//========================================================================

vector<string> csvDigest(const string& s);

string csvConcat(const vector<string>& ss);

//========================================================================
// CSV file management
//========================================================================

void
printCSVRecordAux(const string& unitKind,
                  const string& goalOrigins,
                  const string& goalNumString,
                  int conclNum,
                  const string& status,
                  const string& queryTime,
                  const string& remarks);

void
printCSVRecord(const string& status, const string& remarks);

void printUnitSummary(UnitInfo* ui);

//========================================================================
// Timing
//========================================================================
// Timer set running when created.

class Timer {
private:
    class Time {
    public:
        // user, sys, child user and child sys times
        // Times as multiples of (1/ticksPerSec)
        unsigned long long int uTime;
        unsigned long long int sTime;
        unsigned long long int cuTime;
        unsigned long long int csTime;
    };
    Time startTimeTuple;

    void getOSTimes(Time *tOS);
        
    unsigned int ticksPerSec;

    // Times in sec since start time
    double uTime;
    double sTime;
    double cuTime;
    double csTime;

public:
    Timer();
    void restart();
    
    void sample();
    double getTime();
    string toString();      
    string toLongString();  
};

//========================================================================
// Reporting statistics
//========================================================================
// Globals initialised at definition point.
// Solver interface code responsible for updating globals.

// A `concl' here is each goal / goal slice that gets reported on a line
// of the VCT file

class SessionInfo {
public:
    int trivialConcls;   // Trivially true by Examiner
    int trueConcls;      // Prover says concl true (unsat)
    int falseConcls;     // Prover says concl false (sat)
    int unknownConcls;   // Prover returns "unknown" results
    int errorConcls;     // Some error occurred
    int timeoutConcls;   // Prover timed out or reached
                         // some other resource limit
    int excludedConcls;  // Concl excluded from consideration by
                         // command line options and units.lis file
                         // directives.

    int inconsistentSysRuleSets;  
    int inconsistentUserRuleSets;
    int inconsistentUserRules;
    int derivableUserRules;      // Num user rules derivable without help of
                                 // other user rules
    int interdependentUserRules; // Num user rules derivable with help of other
                                 // user rules

    int auditedUserRules;        // Num user rules considered by audit
                               
    

    double provenTime;         // Total time in prover when concls proven
    double unprovenTime;       // Total time in prover when concls unproven
    double maxProvenQueryTime; // Maximum run time for proven concl

    
    Timer timer;         // Timer for session

    SessionInfo();

    void update(UnitInfo* u);
    void printStats();
};

extern SessionInfo sessionInfo;


//==========================================================================
// Exception handling
//==========================================================================
// 

class VCTException {
private:
    string message;

public:

    VCTException() {}
    VCTException(string m) : message(m) {}

    string toString() {return message;}
    

};

//==========================================================================
// File system access & manipulating names for working files
//==========================================================================

vector<string> listDir (const string& dir);

void tryRemoveFile (const string& file);

bool readableFileExists(const string& s);

void ensureDirPathExists(const string& dirPath);

string getFullGoalFileRoot(const string& defaultFileRoot, 
                           const string& unitName, 
                           int goalNum, 
                           int conclNum);


//==========================================================================
// Polymorphic map and reduce functions for vectors
//==========================================================================



#endif // ! defined UTILITY_HH
