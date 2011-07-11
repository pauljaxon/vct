//==========================================================================
//==========================================================================
// MAIN.CC
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
// TOP LEVEL CODE

#include <locale.h>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ifstream;

#include <algorithm>

#include "utility.hh"  

#include "node.hh"

#include "pdriver.hh"
#include "formatter.hh"

#include "processor.hh"

#include "smt-driver.hh"

#include "smtlib-driver.hh"

#ifdef LINK_YICES
#include "yices-driver.hh"
#endif

#ifdef LINK_CVC3
#include "cvc-driver.hh"
#endif

#include "isab-driver.hh"

using namespace z;

//==========================================================================
// Local utility functions
//==========================================================================


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// readUnitList
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
Todo:

Strip trailing whitespaces.
Skip blank lines

Add trailing "/" if missing.

*/


vector<UnitInfo>
readUnitList(const string& filename) {

    vector<UnitInfo> result;

    ifstream ifs (filename.c_str() );
    if (ifs.fail()) {
        cerr << endl << "Error on trying to open file " << filename << endl;
        exit(1);
    }

    string unitPath;
    while (getline(ifs, unitPath)) {

        if (unitPath[0] == '#')  // Allow comment lines in listing.
            continue; 
        else if (tokeniseString(unitPath).size() == 0) // Allow blank lines
            continue; 
        else
            result.push_back(UnitInfo(unitPath));
    }
    ifs.close();

    return result;
}

// Needs updating to handle UnitInfo objects
/*
void
printUnitList(vector<string> v) {
    cout << "Units to be examined:" << endl; 
    for (vector<string>::iterator i = v.begin(); i != v.end(); i++) {
        cout << "#" << *i << "#" << endl;
    }
    cout << endl << endl;
}
*/

//==========================================================================
// Parse unit.
//==========================================================================
// Parse triple of FDL, RLS and VCG files.


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// parseUnit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Node* parseUnit(UnitInfo unitInfo) {
    string fileroot (unitInfo.getUnitName());

    pdriver driver;
    Node* unitAST = new Node(UNIT);

    if (option("scantrace")) driver.trace_scanning = true;
    if (option("parsetrace")) driver.trace_parsing = true;

    if (option("prefix")) fileroot.insert(0, optionVal("prefix") + "/");

    // -------------------------------------------------------------------
    // Read in and parse declarations files
    // -------------------------------------------------------------------

    // Assemble list of files to read

    vector<string> declFiles;

    if (! option("skip-unit-decls")) {
        declFiles.push_back(fileroot + ".fdl");
    }
    if (option("decls")) {
        vector<string> extraDeclFiles = optionVals("decls");
        // This causes a glibc error message:
        //   corrupted double-linked list!!
        //
        // copy(extraDeclFiles.begin(), extraDeclFiles.end(), declFiles.end());
        declFiles.insert(declFiles.end(),
                         extraDeclFiles.begin(),
                         extraDeclFiles.end());
    }
    
    vector<string> unitDeclFiles(unitInfo.getDeclFiles());
    declFiles.insert(declFiles.end(),
                     unitDeclFiles.begin(),
                     unitDeclFiles.end()
                     );


    // Read files in

    Node* decls = new Node(FDL_FILE);

    for (int i = 0; i != (int) declFiles.size(); i++) {
        string declFile = declFiles.at(i);
        
        printMessage(FINEm, "Reading declarations file " + declFile);

        if (driver.parseFDLFile(declFile) ) {
            printMessage(ERRORm, "Parse of declarations file "
                         + declFile + " failed");
            return 0;
        }
        Node* newDecls = driver.result;

        decls->appendChildren(newDecls);
    }
    unitAST->addChild(decls);

    // -------------------------------------------------------------------
    // Read in and parse rules files
    // -------------------------------------------------------------------

    vector<string> ruleFiles;


    // Assemble list of rules files to read

    ruleFiles.push_back(fileroot + ".rls");
    
    if (option("rules")) {
        vector<string> extraRuleFiles = optionVals("rules");
        ruleFiles.insert(ruleFiles.end(),
                         extraRuleFiles.begin(),
                         extraRuleFiles.end());
    }

    vector<string> unitRuleFiles(unitInfo.getRuleFiles());
    ruleFiles.insert(ruleFiles.end(),
                     unitRuleFiles.begin(),
                     unitRuleFiles.end()
                     );

    if (option("read-directory-rlu-files")) {
        vector<string> filerootParts = splitString(fileroot,"/");
        if (filerootParts.size() >=2) {
            filerootParts.pop_back();
            string dirRLUFile = concatStrings(filerootParts, "/") 
                + "/" + filerootParts.back() + ".rlu";
            if (readableFileExists(dirRLUFile))
                ruleFiles.push_back(dirRLUFile);
        }
            
    }

    if (option("read-unit-rlu-files")) {
        string unitRLUFile = fileroot + ".rlu";
        if (readableFileExists(unitRLUFile))
            ruleFiles.push_back(unitRLUFile);
    }

    Node* rules = new Node(RULE_FILE);

    // Do read of files

    for (int i = 0; i != (int) ruleFiles.size(); i++) {
        string ruleFile = ruleFiles.at(i);
        
        printMessage(FINEm, "Reading rule file " + ruleFile);

        if (driver.parseRuleFile(ruleFile) ) {
            printMessage(ERRORm, "Parse of rule file "
                         + ruleFile + " failed");
            return 0;
        }
        Node* newRules = driver.result;

        rules->appendChildren(newRules);
    }
    unitAST->addChild(rules);

    // -------------------------------------------------------------------
    // Read in and parse vcg or siv file
    // -------------------------------------------------------------------
        
    string vcFileExt(".vcg");
    if (option("siv")) vcFileExt = ".siv";

    printMessage(FINEm, "Reading VCG file");
    if (driver.parseVCGFile(fileroot + vcFileExt)) {
        printMessage(ERRORm, "Parse of VC file failed");
        return 0;
    }
    unitAST->addChild(driver.result);


    return unitAST;
}


//==========================================================================
// Process unit.
//==========================================================================

void 
processUnit(UnitInfo unitInfo, SMTDriver* smtDriver) {

    if (! unitInfo.includeUnit()) return;

    // Set globals used in formatting message headers and in CSV reports
    
    initCurrentUnitInfo(unitInfo);

    if (option("utick")) {
        if (option("longtick")) {
            cout << endl << unitInfo.getUnitName();
        } else {
            cout << "*";
        }
        cout.flush();
    }


    printMessage(INFOm, "Starting unit processing");

    Node* unitAST = parseUnit(unitInfo);

    if (unitAST == 0) {
        printCSVRecord("error", "parsing failed");
        return;
    }

    Formatter::setFormatter(VanillaFormatter::getFormatter());


    string status = elaborateUnit(unitAST);


    if (status != "good") {
        printCSVRecord("error", status);
        return;
    }

    smtDriver->driveUnit(unitAST, unitInfo);

    if (!option("no-mm")) {
            Node::deletePool();
    }
    return;
}


//==========================================================================
// TOP LEVEL FUNCTION
//==========================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Main
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// See usage.txt for usage.

int
main (int argc, char *argv[]) {
    // In order to guarantee consistent timestamps no matter the
    // system used, we explicitly set the time locale to POSIX.
    setlocale(LC_TIME, "POSIX");
    
    // -------------------------------------------------------------------
    // Read in command line arguments
    // -------------------------------------------------------------------
    string fileroot;
    vector<string> nonOptions = processCommandArgs(argc, argv);

    if (nonOptions.size() > 0) {
        fileroot = nonOptions.front();
    }

    // ---------------------------------------------------------------------
    // Set tracing level
    // ---------------------------------------------------------------------
    // Make default level to just report warnings and errors.

    if (!option("level")) {
        messageThreshold = WARNINGm;
    }
    else {
        string s = optionVal("level");
        if (s == "6" || s == "error")        messageThreshold = 6;
        else if (s == "5" || s == "warning") messageThreshold = 5;
        else if (s == "4" || s == "info")    messageThreshold = 4;
        else if (s == "3" || s == "fine")    messageThreshold = 3;
        else if (s == "2" || s == "finer")   messageThreshold = 2;
        else if (s == "1" || s == "finest")  messageThreshold = 1;
        else messageThreshold = WARNINGm;
    }
 
    // ---------------------------------------------------------------------
    // Resolve Units to be processed.
    // ---------------------------------------------------------------------

    vector<UnitInfo> unitList;
    if (fileroot != "") {
        unitList.push_back(UnitInfo(fileroot));
    }
    else if (option("units")) {
        unitList = readUnitList(optionVal("units"));
    }
    else {
        cerr << "No units specified. "<< endl;
        exit(1);
    }

/*
    if (option("print-unit-list")) {
        printUnitList(unitList);
    }
*/
    // ---------------------------------------------------------------------
    // Open report files
    // ---------------------------------------------------------------------

    openReportFiles();

    // ---------------------------------------------------------------------
    // Initialise summary of results
    // ---------------------------------------------------------------------

    trueConcls = 0;
    unprovenConcls = 0;
    errorConcls = 0;

    // ---------------------------------------------------------------------
    // Get solver driver 
    // ---------------------------------------------------------------------

    SMTDriver* smtDriver = 0;

    if (option("interface-mode")) {
        if (optionVal("interface-mode") == "api"
            && option("prover")
            && optionVal("prover") == "cvc3"
            ) {

             #ifdef LINK_CVC3
                 smtDriver = newCVCDriver();
             #else
                 cerr << "Cannot run CVC3 via API: it was not linked in"
                      << endl;
                 exit(1);
             #endif
        }
        else if (optionVal("interface-mode") == "api"
            && option("prover")
            && optionVal("prover") == "yices"
            ) {

             #ifdef LINK_YICES
                 smtDriver = new YicesDriver();
             #else
                 cerr << "Cannot run Yices: it was not linked in"<< endl;
                 exit(1);
             #endif
        }
 /*
        else if (optionVal("interface-mode") == "simplify-old"
            && option("prover")
            && optionVal("prover") == "simplify"
            ) {
            cout << "Init for Simplify" << endl;
            smtDriver = new SimplifyDriver();
        }
        else if (optionVal("interface-mode") == "simplify-old"
            && option("prover")
            && optionVal("prover") == "z3"
            ) {
        
            smtDriver = new Z3Driver();
        }
 */
        else if (optionVal("interface-mode") == "smtlib"
                 || optionVal("interface-mode") == "simplify") {

            smtDriver = new SMTLibDriver();
        }
        else if (optionVal("interface-mode") == "isabelle") {

            smtDriver = new IsabDriver();
        }
        else if (optionVal("interface-mode") == "dummy") {

            smtDriver = new SMTDriver();
        }
        else {

            if (option("prover")) 
                cerr << "Unrecognised interface & prover options: "
                     << optionVal("interface-mode")
                     << " & "
                     << optionVal("prover") << endl;
            else 
                cerr << "Unrecognised interface option: "
                     << optionVal("interface-mode") << endl;
            exit(1);
        }

    } else {
        smtDriver = new SMTDriver();
    }

    smtDriver->initSession();
    // ---------------------------------------------------------------------
    // Process Units
    // ---------------------------------------------------------------------

    for (vector<UnitInfo>::iterator i = unitList.begin();
         i != unitList.end();
         i++) {
        processUnit(*i, smtDriver);
    }


    // ---------------------------------------------------------------------
    // Report summary of results
    // ---------------------------------------------------------------------
    printStats();

    // ---------------------------------------------------------------------
    // Tidy up
    // ---------------------------------------------------------------------
    smtDriver->finaliseSession();
    closeReportFiles();

    if (option("utick") || option("gtick") || option("ctick")) cout << endl;

    return 0;
}

