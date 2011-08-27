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


#include <iostream>
using std::endl;
using std::cout;
using std::cerr;

#include <sstream>
using std::ostringstream;
using std::istringstream;

#include <fstream>
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
    if (s.size() > 0) s.append(", ");
    s.append(t);
    return;
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




//========================================================================
// Command line option processing
//========================================================================

map<string, vector<string> > commandLineOptions;

vector<string> commandLineOptionsList;
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

UnitInfo::UnitInfo(string s) {
    vector<string> toks(tokeniseString(s));

    unitName = toks[0];

    { 
        vector<string> unitNameParts = splitString(unitName,"/");
        unitFileName = unitNameParts.back();
        unitNameParts.pop_back();
        unitPathName = concatStrings(unitNameParts, "/");
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

bool
UnitInfo::includeUnit() {

    return unitIncluded;
}

//========================================================================
// Goal origins formatting
//========================================================================

string currentUnit;         // [<path>/]<fileroot>
string currentUnitPath;     //  <path>
string currentUnitFile;     //  <fileroot>
string currentUnitKind;     //  procedure | function | task_type 
string currentGoalNumStr;      //  <goal number> 
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

void initCurrentUnitInfo(UnitInfo unitInfo) {  

    currentUnit = unitInfo.getUnitName();
    currentUnitPath = unitInfo.getUnitPathName();
    currentUnitFile = unitInfo.getUnitFileName();

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
    logStream << "Host: " << hostname << endl;

    logStream << "Non-option args: " << endl;
    for (int i = 0; i != (int) nonOptionArgs.size(); i++) {
       logStream << "    " << nonOptionArgs.at(i) << endl;
    }

    logStream << "Option args: " << endl;

    for (int i = 0; i != (int) commandLineOptionsList.size(); i++) {

        logStream << "    -"
                 << commandLineOptionsList.at(i)
                 << "   \\"
                 << endl;
    }
    logStream << endl;

    return;
}

void closeReportFiles() {
    csvStream.close();
    logStream.close();
    sumStream.close();
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

    s += ": " + messageTimer.toString() + "s ";

    s += "unit: " + currentUnit;


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
    csvStream << currentUnitPath << ","  //  1
              << currentUnitFile << ","  //  2
              << kindString << ","       //  3
              << originsString << ","    //  4, 5
              << goalNumString << ","    //  6
              << conclString << ","      //  7
              << status << ","           //  8
              << queryTime  << ","       //  9
              << quotedRemarks << ","    // 10
              << currentHypsKinds << "," // 11
              << currentConclKinds       // 12
              << endl;
    return;
}



//========================================================================
// Reporting statistics
//========================================================================
// Solver interface code responsible for updating globals.

int trueConcls;
int unprovenConcls;
int errorConcls;

int timeoutConcls; // Count also included in unproven Concls

Timer totalTime;

void
printStats() {
    bool plain_mode = option("plain");

    string reportName("report");
    if (option("report")) reportName = optionVal("report");


    ostringstream outStream;

    outStream << endl << endl
              << "Total ERROR   messages: " << numErrorMessages << endl
              << "Total WARNING messages: " << numWarningMessages << endl
              << endl;

    int total = trueConcls + unprovenConcls + errorConcls;
    float ftotal = total;

    float fTrueConcls     = trueConcls * 100 / ftotal;
    float fUnprovenConcls = unprovenConcls * 100 / ftotal;
    float fTimeoutConcls  = timeoutConcls * 100 / ftotal;
    float fErrorConcls    = errorConcls * 100 / ftotal;

    outStream << setprecision(1); // 1 decimal place for floats.
    outStream << fixed;

    outStream << "Summary Stats: " << endl;

    outStream << "    true: " << setw(4) << trueConcls
         << "  (" << setw(4) << fTrueConcls << "%)" << endl;

    outStream << "unproven: " << setw(4) << unprovenConcls
         << "  (" << setw(4) << fUnprovenConcls << "%)    ";   //  << endl;

    if (! plain_mode) {
        outStream << "[timeout: " << setw(4) << timeoutConcls
             << "  (" << setw(4) << fTimeoutConcls  << "%) ]" << endl;
    } else {
      outStream << endl;
    }

    outStream << "   error: " << setw(4) << errorConcls
         << "  (" << setw(4) << fErrorConcls << "%)" << endl;

    outStream << "   total: " << setw(4) << total << endl << endl;

    if (! plain_mode ) {
        outStream << "Time: " << totalTime.toLongString() << endl;
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

    sumStream << trueConcls << "," ;
    sumStream << unprovenConcls << "," ;
    sumStream << timeoutConcls << "," ;
    sumStream << errorConcls << "," ;

    sumStream << setw(4) << fTrueConcls << "," ;
    sumStream << setw(4) << fUnprovenConcls << "," ;
    sumStream << setw(4) << fTimeoutConcls << "," ;
    sumStream << setw(4) << fErrorConcls << "," ;

    sumStream << totalTime.toString();
    sumStream << endl;



}


//========================================================================
// Timing
//========================================================================
// A previous implementation used val returned by clock() and 
// divided by CLOCKS_PER_SEC.  with CLOCKS_PER_SEC = 1000000,
// this had the disadvantage that time would wrap in under an hour.
//
// clock() also excludes time spent in child processes, which is needed
// in some cases.

// clock_t val returned by times() calls is an elapsed real time and
// not of much use.

// Get the OS times (user, kernel) for the current process.

static void
GetOSTimes (struct tms *tOS)
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
  tOS->tms_utime = (long long) conv.QuadPart;

  conv.u.LowPart = KernelTime.dwLowDateTime;
  conv.u.HighPart = KernelTime.dwHighDateTime;
  tOS->tms_stime = (long long) conv.QuadPart;
  // There is no notion of child process on windows, set this to 0
  tOS->tms_cutime = 0LL;
  tOS->tms_cstime = 0LL;
#else
  times(tOS);
#endif
}

Timer::Timer() { GetOSTimes(&startTimeTuple); }

void
Timer::restart() { GetOSTimes(&startTimeTuple); }

void
Timer::grabTimes() {
    struct tms endTimeTuple;
#ifdef _WIN32
    // On Windows the granularity of a FILETIME is 100-nanosecond.
    int ticks_per_sec = 10000000;
#else
    int ticks_per_sec = sysconf(_SC_CLK_TCK);
#endif
    GetOSTimes(&endTimeTuple);
    
    uTime =
        ((double) (endTimeTuple.tms_utime - startTimeTuple.tms_utime))
        / ticks_per_sec;

    sTime =
        ((double) (endTimeTuple.tms_stime - startTimeTuple.tms_stime))
        / ticks_per_sec;

    cuTime =
        ((double) (endTimeTuple.tms_cutime - startTimeTuple.tms_cutime))
        / ticks_per_sec;

    csTime =
        ((double) (endTimeTuple.tms_cstime - startTimeTuple.tms_cstime))
        / ticks_per_sec;

}

string 
Timer::toString() {

    grabTimes();
    ostringstream oss;
    oss << (uTime + sTime + cuTime + csTime);
    return oss.str();
}

string 
Timer::toLongString() {
    
    grabTimes();

    double totalTime = uTime + sTime + cuTime + csTime;
    double childTime = cuTime + csTime;
    double sysTime = sTime + csTime;

    double percentChild = childTime * 100 / totalTime;
    double percentSys = sysTime * 100 / totalTime;
    
    ostringstream oss;
    oss << setprecision(2)
        << fixed
        << totalTime
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

