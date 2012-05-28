//==========================================================================
//==========================================================================
// UTILITY.CC
//==========================================================================
//==========================================================================
/*
This file is part of Victor: a SPARK VC Translator and Prover Driver.

Copyright (C) 2009, 2010 University of Edinburgh

Author(s): Paul Jackson, Altran Praxis, AdaCore

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


#include "utility.hh"

// Moved to utility.hh
// #include <iostream>
using std::endl;
using std::cout;
using std::cerr;

#include <sstream>
using std::ostringstream;
using std::istringstream;

// Moved to utility.hh
// #include <fstream>
using std::ofstream;

#include <map>
using std::map;
using std::make_pair;
using std::pair;

#include <iomanip>
using std::setw;
using std::setprecision;

#include <ios>
using std::fixed;

#include <algorithm>  // for find.

#include <cctype>
using std::isdigit;

#include <cstdlib> // For atoi, getenv

#include <stdexcept>  // for runtime_error

extern "C" {

#include <sys/stat.h> // For mkdir
#include <unistd.h> // For access, unlink, gethostname, getpid
#include <dirent.h> // For opendir, readdir, closedir
#include <errno.h>  // For errno static var and error codes
#include <string.h> // For strerror - printing error codes

#ifdef _WIN32

#include <windows.h>

#else

#include <sys/resource.h> // For getrusage() resource name argument
#include <sys/times.h>    // For getrusage()

#endif
}




//========================================================================
// String functions and constants
//========================================================================

// Utility function for debugging. Normal C++ STL always inserts
// conversion automatically from c strings to string type.

string cStringToString(char* c) { return string(c); }


bool isIntString(const string& s) {
    if (s.size() == 0) return false;
    int start = 0;
    if (s[0] == '+' || s[0] == '-') start = 1;

    for (int i = start; i != (int) s.size(); i++) {
        if (! isdigit(s[i])) return false;
    }
    return true;
}

string
intToString(int i) {

    ostringstream oss;
    oss << i;
    return oss.str();
}

string
uLongToString(unsigned long i) {

    ostringstream oss;
    oss << i;
    return oss.str();
}

// c_str returns (const char* const) value.
// atoi expects a const char* argument. i.e. it promises not to change it.

int
stringToInt(const string& s) {
    const char* cs = s.c_str();
    int i = atoi(cs);
    return i;
}

// Assume string is non-empty series of digits.

unsigned long
stringToULong(const string& s) {
    if (s.size() > 10 || (s.size() == 10 && s.compare("4294967296") >= 0)) {
        string msg = "stringToULong: number too large: ";
        msg.append(s);
        throw std::runtime_error(msg);
    }

    unsigned long i;
    istringstream iss(s);
    iss >> i;
    return i;
}

string
boolToString(bool b) {
    if (b)
        return string("true");
    else
        return string("false");
}

string 
doubleToFixPtString(double d, int prec) {
    ostringstream oss;
    oss << setprecision(prec) << fixed << d;
    return oss.str();
}


// Is there platform sensitive way of doing this?  Is endl platform sensitive?

const string ENDLs ("\n");


bool
hasSuffix(const string& s, const string& t) {

    string::size_type i = s.rfind(t, s.size());
    return  i != string::npos && (int) i == (int) s.size() - (int) t.size();

    // (int) casts essential, since t might be 1 longer than s.
    // also need explicit comparison with npos, since otherwise npos gets
    // cast to -1.
}

bool
hasPrefix(const string& s, const string& t) {

    string::size_type i = s.find(t, 0);
    return
        i != string::npos
        && i == 0;
}

bool
hasSubstring(const string& s, const string& t) {

    string::size_type i = s.find(t, 0);
    return i != string::npos;
}




void
appendCommaString(string& s, const string& t) {
    if (t.size() == 0) return;
    if (s.size() > 0) s.append(", ");
    s.append(t);
    return;
}


// if q == "/..." or p == "" then q else p + "/" + q
string joinPaths(const string& p, const string& q) {

    if ((q.size() > 0 && q[0] == '/') || p.size() == 0)
        return q;
    else
        return p + "/" + q;
}


bool
hasUpperCaseStart(const string& s) {
    return s.size() >= 0 && 'A' <= s[0] && s[0] <= 'Z';
}


bool
member(char c, const string& s) {
    string::size_type cpos = s.find(c);
    return cpos != string::npos;
}

// returns substring before first occurrence of c.
// If no occurrence, returns whole string.

string
substringBefore(char c, const string& s) {
    string result(s);
    string::size_type cpos = s.find(c);
    if (cpos != string::npos) {
        //  x    ... x    c       y  ...  y
        //  s[0]       s[cpos]        s[size-1]
        result.erase(cpos, s.size() - cpos);
    }
    return result;
}



// returns substring after first occurrence of c.
// If no occurrence, returns empty string.
string
substringAfter(char c, const string& s) {
    string result(s);
    string::size_type cpos = s.find(c);
    if (cpos != string::npos) {
        //  x    ... x    c       y  ...  y
        //  s[0]       s[cpos]        s[size-1]
        result.erase(0, cpos+1);
        return result;
    }
    return "";
}

// Pull out words in string delimited by whitespace.

bool isWhitespaceChar(const char& c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';;
}

vector<string> tokeniseString(const string& s) {

    vector<string> toks;
    string tok;

    bool inTok = false;

    for (int i = 0; i != (int) s.size(); i++) {

        if (inTok) {
            if (isWhitespaceChar(s[i])) {
                toks.push_back(tok);
                tok.clear();
                inTok = false;
            } else {
                tok.push_back(s[i]);
            }
        } else { // !inTok

            if (isWhitespaceChar(s[i])) {
                continue;
            } else {
                tok.push_back(s[i]);
                inTok = true;
            }
        } // END if inTok
    } // END for i

    if (inTok) toks.push_back(tok);
    return toks;
}


// Strip front and back whitespace from string

string stripWhitespaceEnds(const string& s) {
    string result(s);

    string::size_type start = result.find_first_not_of(" \n\t\r");
    if (start != string::npos && start != 0) {
        result.erase(0, start);
    }
    string::size_type end = result.find_last_not_of(" \n\t\r");
    if (end != string::npos && end + 1 != result.size()) {
        result.erase(end + 1, result.size() - (end + 1));
    }
    return result;
}



// Split s into components delimited by string delimiter
// If delimiter at start (end), returns empty string at start (end)

// Length of result always num delimiters + 1

vector<string> splitString(const string& s, const string& delimiter) {

    vector<string> result;

    size_t left = 0;
    while (true) {
	size_t right = s.find_first_of(delimiter, left);


	result.push_back(s.substr(left, right - left));

	if (right == string::npos) break;
	left = right + delimiter.size();
    }
    return result;
}

// Join together strs, separating them with delimiter.

string concatStrings (const vector<string>& strs, const string& delimiter) {

    if (strs.size() == 0) return "";

    string result = strs.at(0);
    for (int i = 1; i != (int) strs.size(); i++ ) {
        result += delimiter;
        result += strs.at(i);
    }
    return result;
}

// If s has oldSuffix, strip it and add on instead new prefix and suffix.

string fixSuffix(const string& s,
                 const string& oldSuffix,
                 const string& newPrefix,
                 const string& newSuffix) {

    string::size_type pos = s.rfind(oldSuffix, s.size());

    if (pos == string::npos
        || pos + oldSuffix.size() != s.size()) return s;

    string result(s);
    result.erase(pos);  // Take off old suffix

    return newPrefix + result + newSuffix;
    return s;
}

// For when || quoted symbols are not acceptable. (e.g. z3 3.2).
// Symbols can use A-Za-z0-9 and characters
// ~ ! @ $ % ^ & * _ - + = < > . ? /

// Convert all brackets to <>s, "," to "." and any other symbols to "-".

string mkLispSymbolString(const string& s) {
    string result;
    for (int i = 0; i != (int) s.size(); i++) {
        char c = s.at(i);
        char newC = '-';
        
        if ( ('a' <= c && c <= 'z') 
             || ('A' <= c && c <= 'Z') 
             || ('0' <= c && c <= '9')
             || member(c,"~!@$%^&*_-+=<>.?/") ) {

            newC = c;
        }
        else if (c == '[' || c == '(')  newC = '<';
        else if (c == ']' || c == ')')  newC = '>';
        else if (c == ',') newC = '.';

        result.push_back(newC);
    }
    return result;

}

// Simple matching routine.  
// Allows pattern to have

// "?"  matches any single character in 
// "*x" All strings matching [All-"x"]* x
// "*$" [All]*

// $ indicates end of string.  
// All = set of all characters.

bool stringMatch(const string& pat, const string& inst) {

    int ip = 0; // Index into instance string

    for (int ii = 0; ii != (int) inst.size(); ii++) {

        char ichar = inst.at(ii);

        if (ip >= (int) pat.size()) // Pattern has extra unmatched characters
            return false;

        // INVAR: ip and ii point to valid characters in pat and inst
        
        char pchar = pat.at(ip);

        if (pchar == '?') { // ? in pat matches any single char in inst.
            ip++;
        }
        else if (pchar == '*') {
            if (ip == (int) pat.size() - 1)  // ip->*$  ii->x
                return true;
            else {
                char next_pchar = pat.at(ip+1);
                if (ichar == next_pchar) // ip->*x  ii->x
                    ip += 2;
                else
                    ;    // ip->*x  ii->y  x != y.   Leave ip alone.
            }
        }
        else {
            if (pchar != ichar) return false;
            // pchar == ichar  && pchar != '*' or '?'
            ip++;
        }
            
    }
    // ii at end+1 of inst.
    // Have match just when
    // 1. ip at end+1 of pat, or
    // 2. ip points to * at end of pat.
 
    return (ip == (int) pat.size()
            || (ip + 1 == (int) pat.size()  && pat.at(ip) == '*')
        );
}

//========================================================================
// Command line option processing
//========================================================================

map<string, vector<string> > commandLineOptions;

vector<string> commandLineOptionsList;
vector<string> plainCommandLineOptionsList; // Same as above, but for
					    // showing the supplied
					    // options in -plain mode.
vector<string> nonOptionArgs;

/*
processCommandArgs initialises the commandLineOptions map:

Option             Effect

   -<key>            "" added to end of list of map entries for <key>

   -<key>=
 | -<key>=none
 | -<key>=default
 | -<key>=empty      Option map entry for <key> is erased

   -<key>=<val>      <val> added to end of list of map entries for <key>

If an entry list exists for a given key, it will always contain at
least one entry.

*/

vector<string>
processCommandArgs (int argc, char *argv[]) {


    for (int i = 1; i != argc; i++) {
        string arg(argv[i]);

        if (arg[0] == '-') {

            string key(arg.replace(0,1,""));

            commandLineOptionsList.push_back(key);

            string val;
            string::size_type eqpos = key.find('=');
            bool valSupplied = eqpos != string::npos;
            if (valSupplied) {
                val = key;
                //  k    ... k    =       v  ...  v
                //  c[0]        c[eqpos]        c[size-1]
                key.erase(eqpos, key.size() - eqpos);
                val.erase(0, eqpos + 1);
            }
            //  cout << "key: #" << key <<"# val: #" << val <<"#" << endl;

            if (valSupplied
                && (val == ""
                    || val == "false"
                    || val == "none"
                    || val == "default")) {

                commandLineOptions.erase(key);
                continue;
            }
            pair<map<string,vector<string> >::iterator, bool> insResult =
                commandLineOptions.insert
                (
                 make_pair(key, vector<string>(1,val))
                 );

	    // Check if the value is looking like a path; if so don't
	    // show it in -plain mode.
	    bool is_path = (val.find("/") != string::npos) || (val.find("\\") != string::npos);
	    if (is_path) {
	      plainCommandLineOptionsList.push_back(key + "=[SUPRESSED IN PLAIN MODE]");
	    } else {
	      plainCommandLineOptionsList.push_back(arg);
	    }

            if (! insResult.second) {
                // If already one or more entries for key in map,
                // insertion did not take place.
                // Add new value to vector of existing values.
                map<string,vector<string> >::iterator oldMapEntry =
                    insResult.first;
                oldMapEntry->second.push_back(val);
            }


        } else {
            nonOptionArgs.push_back(arg);
        }
    }
    return nonOptionArgs;
}

// Return true just when option set and value is not
// "false", "none" or "default".

bool
option(const string& s) {
    map<string, vector<string> >::iterator i = commandLineOptions.find(s);

    return i != commandLineOptions.end();
}

string
optionVal(const string& s) {
    map<string, vector<string> >::iterator i = commandLineOptions.find(s);
    if (i != commandLineOptions.end()) {
        if (i->second.size() == 0) {
            printMessage(ERRORm,
                         "optionVal: Found empty val list for option " + s);
            return string("");
        }
        return i ->second.back();
    }
    else {
        printMessage(ERRORm,
                     "optionVal: Erroneous lookup of value of option " + s);
        return string("");
    }
}

vector<string>
optionVals(const string& s) {
    map<string, vector<string> >::iterator i = commandLineOptions.find(s);
    if (i != commandLineOptions.end()) {
        return i ->second;
    }
    else {
        return vector<string>();
    }
}

int
intOptionVal(const string& s) {
    map<string, vector<string> >::iterator i = commandLineOptions.find(s);
    if (i != commandLineOptions.end()) {
        if (i->second.size() == 0) {
            printMessage(ERRORm,
                         "intOptionVal: Found empty val list for option " + s);
            return 0;
        }
        return stringToInt(i ->second.back());
    }
    else {
        printMessage(ERRORm,
                     "intOptionVal: Erroneous lookup of value of option " + s);
        return 0;
    }
}

//========================================================================
// External executable and timeout
//========================================================================

// On Windows ulimit is not usable, the way to support timeout on this
// platform is to set the timeout-driver option to some executable that
// will monitor the executable and kill it when necessary. The timeout is
// the first parameter and is specificed with the shell-timeout parameter.

string withTimeoutAndIO(const string &cmd,
                        const string &inputFileName,
                        const string &outputFileName,
                        const string &errorFileName)
{
    string new_cmd = cmd;
    string timeout_driver = "./timeout.sh";

    if (option("timeout-driver")) {
      timeout_driver = optionVal("timeout-driver");
    }

    if (option("shell-timeout")) {
        // Use shell-level timeout utility
        // This will accept integer or fixed point time in sec.

        new_cmd = timeout_driver + " "
            + optionVal("shell-timeout") + " " + new_cmd;
    }

    if (option("ulimit-timeout") || option("ulimit-memory")) {
#ifdef _WIN32
        cerr << endl << "Error ulimit not supported on Windows " << endl;
        exit(1);
#else
        // Use bash built-in timeout facility
        // This accepts integer time in sec and integer memory limit in kbytes.

        // Need to run in a subshell so we can catch output to stderr on
        // termination
	string tmp = "( ulimit";
	if (option("ulimit-timeout")) {
	  tmp += " -t " + optionVal("ulimit-timeout");
	}
	if (option("ulimit-memory")) {
	  tmp += " -v " + optionVal("ulimit-memory");
	}
	tmp += "; " + new_cmd + inputFileName + " )";

	new_cmd = tmp;
#endif
    } else {

        new_cmd += inputFileName ;
    }

    new_cmd +=
        " 1> "  + outputFileName
        + " 2> "  + errorFileName;

    return new_cmd;
}

//========================================================================
// Handling information on each unit.
//========================================================================

// token           Selected Set entry     Meaning
// <gstr>.<cstr>      <g>,<c>            Select goal <g> concl <c>
// <gstr>             <g>,0              Select goal <g> all concls.

// <unit-info> ::= <unit-name> <unit-option>*
// <unit-option> ::=
//  <tag>?<val>
// | <val>
//
// <val> =
//    <goal>
//   | <goal>.<concl>
//   | <file-name>.fdl
//   | <file-name>.rlu
//   | <file-name>.rul

//
// Allocate storage for static fields of class.

UnitInfo::Status UnitInfo::status = UnitInfo::BEFORE_RANGE;

UnitInfo::UnitInfo(int uNum, const string& unitInfoStr)

    : unitNum(uNum),

      dirRLURulesEnd(0),
      unitRLURulesEnd(0),
      numExcludedDirRLURules(0),
      numExcludedUnitRLURules(0),
      numExcludedSystemRules(0),

      auditedUserRules(0),

      parseTreeSize(0),
      translatedUnitSize(0),
      nodeAllocCount(0),
      trivialGoals(0),
      trueQueries(0),
      unknownQueries(0),
      falseQueries(0),
      timeoutQueries(0),
      errorQueries(0),
      excludedConcls(0),

      inconsistentSysRuleSets(0),
      inconsistentUserRuleSets(0),
      inconsistentUserRules(0),
      derivableUserRules(0),
      interdependentUserRules(0),

      unitTime(0.0),
      unprovenQueriesTime(0.0),
      provenQueriesTime(0.0),
      maxProvenQueryTime(0.0)
{

    vector<string> toks(tokeniseString(unitInfoStr));

    unitName = toks[0];

    {

        if (option("prefix")) unitPathPrefix = optionVal("prefix");
        // else unitPathPrefix == "" by default

        vector<string> unitNameParts = splitString(unitName,"/");
        unitFileName = unitNameParts.back();
        unitNameParts.pop_back();
        unitPath = concatStrings(unitNameParts, "/");
        if (unitNameParts.size() > 0) {
            unitDirName = unitNameParts.back();
        }
        else if (unitPathPrefix.size() > 0) {
            vector<string> prefixParts = splitString(unitPathPrefix, "/");
            unitDirName = prefixParts.back();
        }
        // else unitDirName == "" by default

    }

    for (int i = 1; i != (int) toks.size(); i++ ) {

        string token(toks[i]);
        string goalConcl;

        vector<string> splitToken = splitString(token, "?");
        if (splitToken.size() > 1) {

            // tag has been specified
            // Process option if tag in active tags
            // Otherwise skip option

            string tag (splitToken.at(0));

            vector<string> activeTags;
            if (option("active-unit-tags")) {
                activeTags = splitString(optionVal("active-unit-tags"), ":");
            }
            if (find(activeTags.begin(),
                     activeTags.end(),
                     tag) != activeTags.end()) {

                goalConcl = splitToken.at(1);
            }
            else {
                continue;
            }

        } else { // No tag specified for unit option
            goalConcl = token;
        }

        string goal;
        string concl;
        if (goalConcl.find(".",0) != string::npos) {
            goal  = substringBefore('.', goalConcl);
            concl = substringAfter('.', goalConcl);
        } else {
            goal  = goalConcl;
            concl = "0";
        }
        if (isIntString(goal) && isIntString(concl))
            selectedSet.insert(make_pair(stringToInt(goal),
                                         stringToInt(concl)));
        else if (concl == "fdl")
            declFiles.push_back(goalConcl);
        else if (concl == "rul" || concl == "rlu")
            ruleFiles.push_back(goalConcl);
        else
            printMessage(ERRORm, "Unrecognised unit listing info: "
                         + goalConcl);
    }
    bool fromUnitGiven = option("from-unit");
    bool atFromLimit = fromUnitGiven ? optionVal("from-unit") == unitName
                                     : false;
    bool toUnitGiven = option("to-unit");
    bool atToLimit = toUnitGiven ? optionVal("to-unit") == unitName
                                 : false;


    // Logic here is to have unitIncluded true in range between from-unit
    // and to-unit inclusive, with range extended to end when respective
    // limit is missing.

    if (status == BEFORE_RANGE) {
        if (!fromUnitGiven || atFromLimit) {

            unitIncluded = true;

            if (toUnitGiven && atToLimit) {
                status = AFTER_RANGE;
            } else {
                status = IN_RANGE;
            }
        } else {
            unitIncluded = false;
        }

    } else if (status == IN_RANGE) {

        unitIncluded = true;
        if (toUnitGiven && atToLimit) status = AFTER_RANGE;

    } else { // status == AFTER_RANGE
        unitIncluded = false;
    }

    startGoal = (option("from-goal") && atFromLimit)
                ? intOptionVal("from-goal")
                : 1;


}

bool
UnitInfo::include(int goal, int concl) {

    bool goalSliceSelected =
        selectedSet.find(make_pair(goal, concl)) != selectedSet.end()
        || selectedSet.find(make_pair(goal, 0)) != selectedSet.end();


    if (option("include-selected-goals")) {
        return
            unitIncluded
            && goal >= startGoal
            && goalSliceSelected;
    }
    else if (option("exclude-selected-goals")) {
        return
            unitIncluded
            && goal >= startGoal
            && ! goalSliceSelected;
    }
    else {
        return
            unitIncluded
            && goal >= startGoal;
    }
}

void 
UnitInfo::addExcludedRule(int rNum) {
    excludedRules.insert(rNum);

    if (isUnitUserRule(rNum))
        numExcludedUnitRLURules++;
    else if (isDirUserRule(rNum))
        numExcludedDirRLURules++;
    else
        numExcludedSystemRules++;
    return;
}


bool
UnitInfo::includeUnit() {

    return unitIncluded;
}

//========================================================================
// Goal origins formatting
//========================================================================

int currentUnitNum;         // Unit number, starting at 1.
string currentUnit;         // [<path>/]<fileroot>
string currentUnitPath;     //  <path>
string currentUnitFile;     //  <fileroot>
string currentUnitKind;     //  procedure | function | task_type
string currentGoalNumStr;   //  <goal number>
string currentGoalOrigins;  //  Info about where in program goal comes from


/*
updateGoalInfo expects string of form:

[function_|procedure_|task_type_]<name>_<num> <desc>

where

<desc> ::= path(s) from <a> to <b>
         | checks of refinement integrity
<a> ::=

   start
 | from assertion of line 49
 | from default assertion of line 49

<b> ::=
   finish
 | run-time check associated with [\n][ ]+ statement of line 201
 | assertion of line 49
 | default assertion of line 49
 | precondition check associated with [\n][ ]+ statement of line xx:
;
Current version just captures minimum.  Could also do abbreviations and
line numbers.

POGS .sum files use

From::=
  start
 | <nn>
 | <>           (empty) (in case of refinement)
;

To::=

   rtc check @ <nn>
 | assert @ finish
 | assert @ <nn>
 | refinement
 | pre check @ 1514
;
POGS doesn't seem to distinguish between assertions and default assertions.

*/


// Called at start of processUnit in main.cc

void initCurrentUnitInfo(UnitInfo* unitInfo) {
    currentUnitNum = unitInfo->getUnitNum();
    currentUnit = unitInfo->getUnitName();
    currentUnitPath = unitInfo->getUnitPath();
    currentUnitFile = unitInfo->getUnitFileName();

    currentGoalNumStr = "";
    currentGoalOrigins = ",";  // "," since two fields in report
    currentUnitKind = "";

    currentConcl = 0;

    return;
}

void updateCurrentGoalInfo(const string& s) {
    extractGoalInfo(s, currentUnitKind, currentGoalNumStr, currentGoalOrigins);
    return;
}

void extractGoalInfo(const string& s,
                     string& unitKind,
                     string& goalNum,
                     string& goalOrigins) {

     vector<string> ss = tokeniseString(s);
     string currentGoalName = ss[0];     //  <kind>_<fileroot>_<goal number>

     vector<string> goalNameParts = splitString(currentGoalName, "_");

     // Could grab unit kind instead from VCG/SIV file header. Cleaner

     unitKind = goalNameParts.front();
     if (unitKind == "task") unitKind = "task_type";

     goalNum = goalNameParts.back();

     string from;
     string to;

     if (ss[1] == "path(s)") {
         vector<string>::iterator i = std::find(ss.begin(), ss.end(), "to");

         if (i == ss.end()) {
             from = "*** path missing destination ***";
         } else {
             i--;
             from = *i;
             i++;i++;
             if (*i == "finish") to = "assert @ finish";
             else if (*i == "run-time") to = "rtc check @ " + ss.back();
             else if (*i == "assertion" || *i == "default")
                 to = "assert @ " + ss.back();
             else if (*i == "precondition") to = "pre check @ " + ss.back();
             else to = "*** unrecognised path destination ***";
         }
     } else if (ss[1] == "checks" && ss[3] == "refinement") {
         to = "refinement";
     } else {
         to = "*** Unrecognised goal description ***";
     }

     goalOrigins = from + "," + to;
     return;
}

//========================================================================
// Report file management
//========================================================================

ofstream csvStream;
ofstream logStream;
ofstream sumStream;
ofstream unitSumStream;

extern string unitSumHeader; // Defined below in code that writes records
                             // unitSumStream.
void openReportFiles() {
    string reportName("report");
    if (option("report")) reportName = optionVal("report");

    if (option("report-dir")) {
        ensureDirPathExists(optionVal("report-dir"));
        reportName = optionVal("report-dir") + "/" + reportName;
    }
    string csvFile(reportName + ".vct");
    string logFile(reportName + ".vlg");
    string sumFile(reportName + ".vsm");
    string unitSumFile(reportName + ".vus");

    csvStream.open(csvFile.c_str());
    if (csvStream.fail()) {
        cerr << endl << "Error on trying to open file " << csvFile << endl;
        exit(1);
    }
    logStream.open(logFile.c_str());
    if (logStream.fail()) {
        cerr << endl << "Error on trying to open file " << logFile << endl;
        exit(1);
    }
    sumStream.open(sumFile.c_str());
    if (sumStream.fail()) {
        cerr << endl << "Error on trying to open file " << sumFile << endl;
        exit(1);
    }
    unitSumStream.open(unitSumFile.c_str());
    if (unitSumStream.fail()) {
        cerr << endl << "Error on trying to open file " << unitSumFile << endl;
        exit(1);
    }

    // Write a prelude to log file identifying run.

    if (! option("plain")) {
	std::time_t rawtime;
	struct std::tm * timeinfo;
	std::time (&rawtime);
	timeinfo = std::localtime(&rawtime);
	char the_time[100];
	std::strftime(the_time, sizeof (the_time),
		      "%d-%b-%Y, %H:%M:%S",
		      timeinfo);

	logStream << "Date: " << the_time << endl;
    } else {
        logStream << endl;
    }

    char hostname[100];
#ifdef _WIN32
    DWORD len = 100;
    GetComputerName(hostname, &len);
#else
    gethostname(hostname, 100);
#endif
    if (! option("plain")) {
        logStream << "Host: " << hostname << endl;
    }

    logStream << "Non-option args: " << endl;
    for (int i = 0; i != (int) nonOptionArgs.size(); i++) {
       logStream << "    " << nonOptionArgs.at(i) << endl;
    }

    logStream << "Option args: " << endl;

    if (option("plain")) {
      for (int i = 0; i != (int) plainCommandLineOptionsList.size(); i++) {
        logStream << "    -"
		  << plainCommandLineOptionsList.at(i)
		  << "   \\"
		  << endl;
      }
    } else {
      for (int i = 0; i != (int) commandLineOptionsList.size(); i++) {
        logStream << "    -"
		  << commandLineOptionsList.at(i)
		  << "   \\"
		  << endl;
      }
    }
    logStream << endl;

    // Write header for unit summary file

    unitSumStream << unitSumHeader << endl;

    return;
}

void closeReportFiles() {
    csvStream.close();
    logStream.close();
    sumStream.close();
    unitSumStream.close();
    return;
}


//========================================================================
// Message reporting
//========================================================================

// See utility.hh for documentation.

const int ERRORm = 6;
const int WARNINGm = 5;
const int INFOm = 4;
const int FINEm = 3;
const int FINERm = 2;
const int FINESTm = 1;

int messageThreshold;


int currentConcl;

string currentHypsKinds;
string currentConclKinds;

string goalSliceTime;

Timer messageTimer;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// printMessageWithHeader
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void printMessageWithHeader(const string& header, const string& message) {

    string s(header);

    // Message origins line

    messageTimer.sample();
    s += ": " + messageTimer.toString() + "s ";

    s += "unit: " + intToString(currentUnitNum) + " - " + currentUnit;


    if (currentGoalNumStr.size() > 0) {

        s += "  goal: " + currentGoalNumStr;
    }
    if (currentConcl > 0  && !option("fuse-concls") ) {
        s += "  concl: " + intToString(currentConcl);
    }

    if (header == "ERROR") cerr << endl << s << endl;

    s += ENDLs;

    // Message contents line + extra blank line separator after.

    s += message + ENDLs + ENDLs;

    logStream << s;

    return;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// printMessageAux
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


int printMessageAux1(int messageLevel, const string& message) {

    string header;
    if (messageLevel == FINESTm)       header = "FINEST";
    else if (messageLevel == FINERm)   header = "FINER";
    else if (messageLevel == FINEm)    header = "FINE";
    else if (messageLevel == INFOm)    header = "INFO";
    else if (messageLevel == WARNINGm) header = "WARNING";
    else                               header = "ERROR";

    printMessageAux2(messageLevel);

    printMessageWithHeader(header, message);
    return 0;
}

int numWarningMessages = 0;
int numErrorMessages = 0;

int printMessageAux2(int messageLevel) {
    if (messageLevel == WARNINGm) numWarningMessages++;
    if (messageLevel == ERRORm) numErrorMessages++;
    return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// printMessageOnOptionAux
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int printMessageOnOptionAux(const string& opt, const string& message) {

    string header("OPTION(" + opt + ")");
    printMessageWithHeader(header, message);
    return 0;
}


//========================================================================
// CSV formatting
//========================================================================
// For CSV format, see http://en.wikipedia.org/wiki/Comma-separated_values

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// csvDigest
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Break up CSV string into its components

// Basic. Should also:
// - handle quoted " chars
// - strip head and tail whitespace from each value.

vector<string> csvDigest(const string& s) {



    vector<string> toks;
    string tok;

    bool inQuote = false;

    for (int i = 0; i != (int) s.size(); i++) {
        char c = s[i];
        if (inQuote) {
            if (c == '"') {
                inQuote = false;
            } else {
                tok.push_back(c);
            }
        } else { // !inQuote

            if (c == '"') {
                inQuote = true;
            } else if (c == ',') {
                toks.push_back(stripWhitespaceEnds(tok));
                tok.clear();
            } else {
                tok.push_back(c);
            }
        } // END if inQuote
    } // END for i

    toks.push_back(stripWhitespaceEnds(tok));
    return toks;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// csvConcat
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Concatenate together strings to form CSV record


string csvConcat(const vector<string>& ss) {
    string result;
    for (int i = 0; i != (int) ss.size(); i++) {

        if (ss[i].find(',') != string::npos) {
            result.push_back('"');
            result.append(ss[i]);
            result.push_back('"');
        } else {
            result.append(ss[i]);
        }
        if (i != (int) ss.size() - 1) {
            result.append(",");
        }
    }
    return result;
}



//========================================================================
// CSV file reporting
//========================================================================



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// printCSVRecord
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Print a comma separated value record.
// Quote remarks field, since might have embedded ","s.

void
printCSVRecord(const string& status, const string& remarks) {
    printCSVRecordAux(currentUnitKind,
                      currentGoalOrigins,
                      currentGoalNumStr,
                      currentConcl,
                      status,
                      goalSliceTime,
                      remarks);
    return;
}

void
printCSVRecordAux(const string& unitKind,
                  const string& goalOrigins,
                  const string& goalNumString,
                  int conclNum,
                  const string& status,
                  const string& queryTime,
                  const string& remarks) {

    if (! option("gstime")) goalSliceTime = "";

    string kindString(option("csv-reports-include-unit-kind")
                      ? unitKind
                      : "");

    string originsString(option("csv-reports-include-goal-origins")
                       ? goalOrigins
                       : ",");

    string conclString(conclNum > 0 && !option("fuse-concls")
                       ? intToString(conclNum)
                       : "");

    string quotedRemarks(remarks.size() > 0
                         ? "\"" + remarks + "\""
                         : remarks);

                                         // Fields
    csvStream << currentUnitNum << ","   //  1
              << currentUnitPath << ","  //  2
              << currentUnitFile << ","  //  3
              << kindString << ","       //  4
              << originsString << ","    //  5, 6
              << goalNumString << ","    //  7
              << conclString << ","      //  8
              << status << ","           //  9
              << queryTime  << ","       // 10
              << quotedRemarks << ","    // 11
              << currentHypsKinds << "," // 12
              << currentConclKinds       // 13
              << endl;
    return;
}

//========================================================================
// Unit Summary reporting
//========================================================================

int lastNumWarningMessages = 0;
int lastNumErrorMessages = 0;

string unitSumHeader(
    "unit num,"
    "unit,"
    "ERRORs,"
    "WARNINGs,"
    "total,"
    "trivial,"
    "true,"
    "unproven,"
    "timeout,"
    "false,"
    "error,"
    "urules,"
    "exc. dir urules,"
    "exc. unit urules,"
    "exc. sys rules,"
    "parse tree,"
    "tx tree,"
    "alloced nodes,"
    "total time,"
    "proven time,"
    "av. proven time,"
    "max proven time,"
    "unproven time,"
    "av. unproven time,"
    "incon wo urules,"
    "incon w urules,"
    "incon urules,"
    "deriv urules,"
    "interdep urules,"
    "remarks"
    );

void printUnitSummary(UnitInfo* ui) {

    int numUnitWarningMessages = numWarningMessages - lastNumWarningMessages;
    int numUnitErrorMessages = numErrorMessages - lastNumErrorMessages;

    lastNumWarningMessages = numWarningMessages;
    lastNumErrorMessages = numErrorMessages;

    int unprovenQueries =
        ui->unknownQueries + ui->falseQueries
        + ui->timeoutQueries + ui->excludedConcls;

    int totalConcls =
        ui->trivialGoals
        + ui->trueQueries
        + unprovenQueries
        + ui->errorQueries;

    unitSumStream                      
        << ui->getUnitNum() << ","          
        << ui->getUnitName() << ","          

        << numUnitErrorMessages << ","
        << numUnitWarningMessages << ","

        << totalConcls << ","

        << ui->trivialGoals << ","    
        << ui->trueQueries << ","       
        << unprovenQueries << ","   
        << ui->timeoutQueries << ","    
        << ui->falseQueries << ","   
        << ui->errorQueries << ","      

        << ui->unitRLURulesEnd << "," // Number of user rules (dir + unit)

        << ui->numExcludedDirRLURules << ","
        << ui->numExcludedUnitRLURules << ","
        << ui->numExcludedSystemRules << ","

        << ui->parseTreeSize << ","
        << ui->translatedUnitSize << ","
        << ui->nodeAllocCount << ",";

    unitSumStream
        << setprecision(3) << fixed
        << ui->unitTime << ","
        << ui->provenQueriesTime << ",";

    if (ui->trueQueries != 0) {
        unitSumStream << ui->provenQueriesTime / ui->trueQueries;
    }
    unitSumStream
        << ","
        << ui->maxProvenQueryTime << ","
        << ui->unprovenQueriesTime << ",";
    if (unprovenQueries != 0) {
        unitSumStream << ui->unprovenQueriesTime / unprovenQueries;
    }
    unitSumStream << ",";
    if (option("do-rule-audit")) {
        unitSumStream << ui->inconsistentSysRuleSets << ","
                      << ui->inconsistentUserRuleSets << ","
                      << ui->inconsistentUserRules << ","
                      << ui->derivableUserRules << ","
                      << ui->interdependentUserRules << ",";
    } else {
        unitSumStream << ",,,,,";
    }
    unitSumStream  << "\"" << ui->remarks << "\"" << endl;
    return;
}



//========================================================================
// Reporting statistics
//========================================================================
// Solver interface code responsible for updating globals.

// Global variable holding accumulated statistics

SessionInfo sessionInfo;

SessionInfo::SessionInfo() : 
    trivialConcls(0),
    trueConcls(0),
    falseConcls(0),
    unknownConcls(0),
    errorConcls(0),
    timeoutConcls(0), 
    excludedConcls(0), 

    inconsistentSysRuleSets(0),
    inconsistentUserRuleSets(0),
    inconsistentUserRules(0),
    derivableUserRules(0),
    interdependentUserRules(0),

    auditedUserRules(0),

    provenTime(0.0),         
    unprovenTime(0.0),       
    maxProvenQueryTime(0.0)
{}

void
SessionInfo::update(UnitInfo* unitInfo) {
    trivialConcls  += unitInfo->trivialGoals;
    trueConcls     += unitInfo->trueQueries;
    unknownConcls  += unitInfo->unknownQueries;
    falseConcls    += unitInfo->falseQueries;
    timeoutConcls  += unitInfo->timeoutQueries;
    errorConcls    += unitInfo->errorQueries;
    excludedConcls += unitInfo->excludedConcls;

    inconsistentSysRuleSets  += unitInfo->inconsistentSysRuleSets;
    inconsistentUserRuleSets += unitInfo->inconsistentUserRuleSets;
    inconsistentUserRules    += unitInfo->inconsistentUserRules;  
    derivableUserRules       += unitInfo->derivableUserRules;  
    interdependentUserRules  += unitInfo->interdependentUserRules;

    auditedUserRules += unitInfo->auditedUserRules;
    
    provenTime += unitInfo->provenQueriesTime;
    unprovenTime += unitInfo->unprovenQueriesTime;
    if (unitInfo->maxProvenQueryTime
        > maxProvenQueryTime) {
        
        maxProvenQueryTime = unitInfo->maxProvenQueryTime;
    }
    return;
}


void
SessionInfo::printStats() {

    timer.sample();

    bool plain_mode = option("plain");

    string reportName("report");
    if (option("report")) reportName = optionVal("report");


    ostringstream outStream;

    outStream << endl << endl
              << "Total ERROR   messages: " << numErrorMessages << endl
              << "Total WARNING messages: " << numWarningMessages
              << endl << endl;


    if (option("do-rule-audit")) {

        outStream
            << "User Rule Audit: (" << auditedUserRules
            << " user rules considered)" << endl << endl
            << "  inconsistent units, user rules excluded:"
            << setw(6) << inconsistentSysRuleSets << endl
            << "  inconsistent units, user rules included:"
            << setw(6) << inconsistentUserRuleSets << endl
            << "                  inconsistent user rules:"
            << setw(6) << inconsistentUserRules << endl
            << "                     derivable user rules:"
            << setw(6) << derivableUserRules << endl    
            << "                interdependent user rules:"
            << setw(6) << interdependentUserRules
            << endl << endl;
    }
    int unprovenConcls =
        unknownConcls + timeoutConcls + falseConcls + excludedConcls;
    int total = trivialConcls + trueConcls + unprovenConcls + errorConcls;
    float ftotal = total;

    float fTrivialConcls  = trivialConcls * 100 / ftotal;
    float fTrueConcls     = trueConcls * 100 / ftotal;
    float fUnprovenConcls = unprovenConcls * 100 / ftotal;
    float fTimeoutConcls  = timeoutConcls * 100 / ftotal;
    float fFalseConcls    = falseConcls * 100 / ftotal;
    float fErrorConcls    = errorConcls * 100 / ftotal;

    outStream << setprecision(1); // 1 decimal place for floats.
    outStream << fixed;

    outStream << "Summary Stats: " << endl << endl;

    outStream << "   trivial:" << setw(6) << trivialConcls
         << "  (" << setw(4) << fTrivialConcls << "%)" << endl;

    outStream << "      true:" << setw(6) << trueConcls
         << "  (" << setw(4) << fTrueConcls << "%)" << endl;

    outStream << "  unproven:" << setw(6) << unprovenConcls
         << "  (" << setw(4) << fUnprovenConcls << "%)  ";   //  << endl;

    if (! plain_mode) {
        outStream << "[timeout:" << setw(6) << timeoutConcls
             << "  (" << setw(4) << fTimeoutConcls  << "%)  false:"
             << setw(6) << falseConcls
             << "  (" << setw(4) << fFalseConcls  << "%) ]" << endl;
    } else {
      outStream << endl;
    }

    outStream << "     error:" << setw(6) << errorConcls
         << "  (" << setw(4) << fErrorConcls << "%)" << endl;

    outStream << "     total:" << setw(6) << total << endl << endl;

    
    string totalTrueTimeStr(doubleToFixPtString(provenTime,2));
    string averageTrueTimeStr;
    if (trueConcls > 0) {
        averageTrueTimeStr = doubleToFixPtString(provenTime/trueConcls,3);
    }
    string maxTrueTimeStr(doubleToFixPtString(maxProvenQueryTime,3));
    string totalUnprovenTimeStr(doubleToFixPtString(unprovenTime,2));

    double vctOverheadTime = timer.getTime() - provenTime - unprovenTime; 
    string vctOverheadTimeStr(doubleToFixPtString(vctOverheadTime, 2));

    string vctOverheadPercentStr;
    if (provenTime > 0) {
        vctOverheadPercentStr = 
        "(" + doubleToFixPtString(vctOverheadTime * 100 / provenTime,0)
            + "% of total true)";
    }
    
    if (! plain_mode ) {
        outStream << "Times: " << endl

                  << "      total true:" << setw(7) << totalTrueTimeStr
                  << "s  (average true: " << averageTrueTimeStr
                  << "s  max true: " << maxTrueTimeStr << "s)" << endl

                  << "  total unproven:" << setw(7) << totalUnprovenTimeStr << "s" << endl

                  << "    vct overhead:" << setw(7) << vctOverheadTimeStr << "s  "
                  << vctOverheadPercentStr << endl

                  << "           total:" << timer.toLongString() << endl;
    }

    logStream << outStream.str();

    if (option("echo-final-stats")) {

        cout << outStream.str() << endl;

        // Add on root of report files.

        string reportName("report");
        if (option("report")) reportName = optionVal("report");

        if (option("report-dir")) {
            reportName = optionVal("report-dir") + "/" + reportName;
        }
        cout << "Report files: " << reportName << ".*" << endl;
    }

    sumStream << reportName << ",";
    sumStream << numErrorMessages << "," << numWarningMessages << ",";
    sumStream << setprecision(1);
    sumStream << fixed;

    sumStream << total << "," ;

    sumStream << trivialConcls << "," ;
    sumStream << trueConcls << "," ;
    sumStream << unprovenConcls << "," ;
    sumStream << timeoutConcls << "," ;
    sumStream << falseConcls << "," ;
    sumStream << errorConcls << "," ;

    sumStream << setw(4) << fTrueConcls << "," ;
    sumStream << setw(4) << fUnprovenConcls << "," ;
    sumStream << setw(4) << fTimeoutConcls << "," ;
    sumStream << setw(4) << fFalseConcls << "," ;
    sumStream << setw(4) << fErrorConcls << "," ;

    if (!option("plain")) {
        sumStream << timer.toString() << ","
                  << totalTrueTimeStr << ","
                  << averageTrueTimeStr << ","
                  << maxTrueTimeStr << ","
                  << totalUnprovenTimeStr << ",";
    } else {
        sumStream << ",,,,,";
    }
    if (option("do-rule-audit")) {
        sumStream << inconsistentSysRuleSets << ","
                  << inconsistentUserRuleSets << ","
                  << inconsistentUserRules << ","
                  << derivableUserRules << ","
                  << interdependentUserRules ;
    } else {
        sumStream << ",,,,";
    }
        

    sumStream << endl;



}


//========================================================================
// Timing
//========================================================================
/*
Previous versions
-----------------
1. Using clock (3).
This returned a clock_t value (32 bit unsigned on 32-bit system, 64
bit unsigned on 64-bit system it seems from experiments.

For time in sec had to divide by CLOCKS_PER_SEC = 1000000, with
disadvantage that time would wrap in under an hour on 32 bit
systems.

clock() also doesn't give breakdown of time into user and system
time, and doesn't give time spent in child processes.

2. Using times (2).

Returns a
           struct tms {
               clock_t tms_utime;  // user time
               clock_t tms_stime;  // system time
               clock_t tms_cutime; // user time of children
               clock_t tms_cstime; // system time of children
           };

where must divide by sysconf(_SC_CLK_TCK) to get time in seconds.
With SL5 and SL6, this is 100, giving a resolution of 0.01sec.
This is rather coarse, given some SMT solver run times are of this
order.

Current version
---------------
Uses getrusage (2).

time (7) remarks this has a clock resolution of HZ which can be
0.01, 0.001 sec a couple of values inbetween, depending on kernel
configuration.  The SL6 appears to use 0.001, so better than
times().

A local class Time is defined, similar to struct tms, except that it
definitely has 64 bit values for the fields.

struct rusage {
     struct timeval ru_utime; // user time used
     struct timeval ru_stime; // system time used
     ...
}
struct timeval
  {
    __time_t tv_sec;		// Seconds.
    __suseconds_t tv_usec;	// Microseconds.
  };

*/

void
Timer::getOSTimes (Timer::Time *tOS)
{
#ifdef _WIN32
  FILETIME CreationTime;
  FILETIME ExitTime;
  FILETIME KernelTime;
  FILETIME UserTime;
  ULARGE_INTEGER conv;

  GetProcessTimes
    (GetCurrentProcess(),
     &CreationTime, &ExitTime, &KernelTime, &UserTime);

  conv.u.LowPart = UserTime.dwLowDateTime;
  conv.u.HighPart = UserTime.dwHighDateTime;
  tOS->uTime = (long long) conv.QuadPart;

  conv.u.LowPart = KernelTime.dwLowDateTime;
  conv.u.HighPart = KernelTime.dwHighDateTime;
  tOS->sTime = (long long) conv.QuadPart;
  // There is no notion of child process on windows, set this to 0
  tOS->cuTime = 0ULL;
  tOS->csTime = 0ULL;
#else
// To use times() again.
/*
  struct tms tms_time;
  times(&tms_time);
  tOS->sTime = tms_time->tms_stime;
  tOS->uTime = tms_time->tms_utime;
  tOS->csTime = tms_time->tms_cstime;
  tOS->cuTime = tms_time->tms_cutime;
*/
  struct rusage selfRUsage;
  struct rusage childRUsage;

  getrusage(RUSAGE_SELF, &selfRUsage);
  getrusage(RUSAGE_CHILDREN, &childRUsage);

  timeval sT = selfRUsage.ru_stime;
  timeval uT = selfRUsage.ru_utime;
  timeval csT = childRUsage.ru_stime;
  timeval cuT = childRUsage.ru_utime;

  tOS->sTime = ((unsigned long long int) sT.tv_sec) * 1000000 + sT.tv_usec;
  tOS->uTime = ((unsigned long long int) uT.tv_sec) * 1000000 + uT.tv_usec;
  tOS->csTime = ((unsigned long long int) csT.tv_sec) * 1000000 + csT.tv_usec;
  tOS->cuTime = ((unsigned long long int) cuT.tv_sec) * 1000000 + cuT.tv_usec;

#endif
}

Timer::Timer() {
#ifdef _WIN32
    // On Windows the granularity of a FILETIME is 100-nanosecond.
    ticksPerSec = 10000000;
#else
    // If using times()
    // ticksPerSec = sysconf(_SC_CLK_TCK);
    // If using getrusage()
    ticksPerSec = 1000000;     // This is the resolution of Time values,
                               // *not* the timer resolution

#endif
    getOSTimes(&startTimeTuple);
}
void
Timer::restart() { getOSTimes(&startTimeTuple); }

void
Timer::sample() {
    Time endTimeTuple;
    getOSTimes(&endTimeTuple);

    uTime =
        ((double) (endTimeTuple.uTime - startTimeTuple.uTime))
        / ticksPerSec;

    sTime =
        ((double) (endTimeTuple.sTime - startTimeTuple.sTime))
        / ticksPerSec;

    cuTime =
        ((double) (endTimeTuple.cuTime - startTimeTuple.cuTime))
        / ticksPerSec;

    csTime =
        ((double) (endTimeTuple.csTime - startTimeTuple.csTime))
        / ticksPerSec;

}

double
Timer::getTime() {
    return uTime + sTime + cuTime + csTime;
}

string
Timer::toString() {
    ostringstream oss;
    oss << setprecision(3) << fixed;
    oss << (uTime + sTime + cuTime + csTime);
    return oss.str();
}

string
Timer::toLongString() {

    double totalTime = uTime + sTime + cuTime + csTime;
    double childTime = cuTime + csTime;
    double sysTime = sTime + csTime;

    double percentChild = childTime * 100 / totalTime;
    double percentSys = sysTime * 100 / totalTime;

    ostringstream oss;
    oss << setprecision(2)
        << fixed
        << setw(7) << totalTime
        << "s  (u: "  << uTime
        <<   "s, s: "  << sTime
        <<   "s, cu: " << cuTime
        <<   "s, cs: " << csTime
        <<   "s,  "
        << setprecision(1)
        << percentChild << "% ch, "
        << percentSys   << "% sys)"
        << endl;

    return oss.str();
}


//==========================================================================
// File system access & manipulating names for working files
//==========================================================================

vector<string> listDir (const string& dir) {
    DIR *dp;
    struct dirent *ep;
    vector<string> contents;

    dp = opendir (dir.c_str());

    if (dp == NULL) {
        cerr << "Error on trying to list directory " << dir << endl;
        return contents;
    }

    while ( (ep = readdir (dp)) ) {
        contents.push_back(ep->d_name);
    }
    (void) closedir (dp);
    return contents;
}

// Returns  0 if remove successful.
// Returns -1 on failure and sets errNumber and errorMessage.

int removeFile (const string& file, int& errNumber, string& errorMessage) {

    int failure = unlink(file.c_str());
    if (failure != 0) {
        errorMessage = string( strerror(errno));
    }
    errNumber = errno;
    return failure;
}

// Attempt to remove file.  Silent if file doesn't exist.

void tryRemoveFile (const string& file) {

    int errNumber;
    string errMessage;
    int failure = removeFile(file, errNumber, errMessage);
    if (failure != 0 && errNumber != ENOENT) {

        printMessage(ERRORm, "Attempt at removing file " + file +
                     " failed with error: " + errMessage);
    }
    return;
}




bool readableFileExists(const string& s) {
    return (access(s.c_str(), R_OK) == 0);
}

void ensureDirPathExists(const string& s) {

    // Ensure dirPath != ""
    string dirPath(s.size() == 0 ? "." : s);

    vector<string> parts = splitString(dirPath, "/");

    bool absolute = parts.at(0).size() == 0;
    if (absolute)
        parts.erase(parts.begin());

    string prefix = absolute ? "/" : "";

    for (int i = 0; i < (int) parts.size(); i++) {
	prefix.append(parts.at(i));

	if (access(prefix.c_str(), F_OK) != 0) {  // prefix does not exists

#ifdef _WIN32
	    int status = mkdir (prefix.c_str());
#else
	    int status = mkdir (prefix.c_str(), S_IRWXU);
#endif

	    if (status != 0) { //
                throw std::runtime_error("Failed to make directory " +
                                         prefix);
            }
	}
	prefix.append("/");
    }
}

// Return variation on input s containing no '/' characters.

string flattenPathName(const string& s) {
    return concatStrings( splitString(s, "/"), "-");
}

string
getFullGoalFileRoot(const string& defaultFileRoot,
                    const string& unitName, // = dir1/.../dirn/unitFileRoot
                    int goalNum,
                    int conclNum) {


    string goalRootDir = "/tmp";
    if (option("working-dir")) goalRootDir = optionVal("working-dir");
    if (goalRootDir.size() == 0) goalRootDir = ".";


    string goalName;
    if (option("hier-working-files") || option("flat-working-files") ) {

        goalName = unitName + "-" + intToString(goalNum);

        if (!option("fuse-concls")) {
           goalName += "-" + intToString(conclNum);
        }
        if (option("flat-working-files")) {
            goalName  = flattenPathName(goalName);
        }
    }
    else if (option("unique-working-files")) {
        char hostname[100];
#ifdef _WIN32
	DWORD len = 100;
	GetComputerName(hostname, &len);
#else
        gethostname(hostname, 100);
#endif
        goalName = defaultFileRoot + "-"
                   + hostname + "-"
                   + intToString(getpid());
    }
    else {
        goalName = defaultFileRoot;
    }

    // Join working-dir to goalName

    string fullGoalName = goalRootDir + "/" + goalName;

    // Ensure existence of directories mentioned in fullGoalName

    size_t lastSlash = fullGoalName.find_last_of('/');
    string goalDirPath = fullGoalName.substr(0, lastSlash);
    ensureDirPathExists(goalDirPath);


    return fullGoalName;
}

