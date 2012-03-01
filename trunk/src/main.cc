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
#include <unistd.h>
#include <sys/param.h>
#include <libgen.h>

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
#include "smtlib2-driver.hh"

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


/*
rulefamily ::= RULE_FAMILY (SEQ typeassum+) (SEQ rule+)
rlsfile ::=    RLS_FILE rulefamily+
*/

int countRules(Node* ruleFileAST) {
    int count = 0;
    for (int i = 0; i != ruleFileAST->arity(); i++) {
        Node* ruleFamily = ruleFileAST->child(i);
        count += ruleFamily->child(1)->arity();
    }
    return count;
}

Node* readRuleFile(pdriver& driver, const string& ruleFile) {
        
        printMessage(FINEm, "Reading rule file " + ruleFile);

        if (driver.parseRuleFile(ruleFile) ) {
            printMessage(ERRORm, "Parse of rule file "
                         + ruleFile + " failed");
            exit(1);
        }
        return driver.result;
}



//==========================================================================
// Parse unit.
//==========================================================================
// Parse triple of FDL, RLS and VCG files.


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// parseUnit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Node* parseUnit(UnitInfo* unitInfo) {

    // fullUnitName == P1/.../Pn/D1/.../Dk/U
    string fullUnitName (unitInfo->getFullUnitName());

    pdriver driver;
    Node* unitAST = new Node(UNIT);

    if (option("scantrace")) driver.trace_scanning = true;
    if (option("parsetrace")) driver.trace_parsing = true;

    // -------------------------------------------------------------------
    // Read in and parse declarations files
    // -------------------------------------------------------------------

    // Assemble list of files to read

    vector<string> declFiles;

    if (! option("skip-unit-decls")) {
        if (option("read-all-decl-files-in-dir")) {

            // unitDir == P1/.../Pn/D1/.../Dk
            string unitDir(joinPaths(unitInfo->getUnitPathPrefix(),
                                     unitInfo->getUnitDirName()));

            vector<string> dirContents = listDir(unitDir);
            for (int i = 0; i != (int) dirContents.size(); i++) {
                string dirItem = dirContents.at(i);
                if (hasSuffix(dirItem, ".fdl")) {
                    string fullFDLFileName = unitDir + "/" + dirItem;
                    declFiles.push_back(fullFDLFileName);
                }
            }
        }
        else {
            // Add P1/.../Pn/D1/.../Dk/U.fdl
            declFiles.push_back(fullUnitName + ".fdl");
        }
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
    
    vector<string> unitDeclFiles(unitInfo->getDeclFiles());
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


    Node* rules = new Node(RULE_FILE);


    if (option("read-directory-rlu-files")) {
      string dirRLUFile;

      if (unitInfo->getUnitDirName() != "" ) {

	dirRLUFile  // == P1/.../Pn/D1/.../Dk/Dk.rlu  if k>0
                    // == P1/.../Pn/Pn.rlu  if n>0
                    // == .rlu  o/w
	  = joinPaths(unitInfo->getUnitPathPrefix(),
		      joinPaths(unitInfo->getUnitPath(),
				unitInfo->getUnitDirName()))
	  + ".rlu";
      } else {
	// We don't have a directory in our unit name if the unit is
	// in the current directory. In this case the name of the
	// global user rule can be worked out from the current
	// directory.
	char cwd[MAXPATHLEN];
	getcwd(cwd, MAXPATHLEN);
	dirRLUFile = basename(cwd);
	dirRLUFile += ".rlu";
      }

      if (readableFileExists(dirRLUFile)) {

	Node* ruleFile = readRuleFile(driver, dirRLUFile);
	unitInfo->dirRLURulesEnd = countRules(ruleFile);
	unitInfo->unitRLURulesEnd = unitInfo->dirRLURulesEnd;

	rules->appendChildren(ruleFile);
      }
    }

    if (option("read-unit-rlu-files")) {


        string unitRLUFile // == P1/.../Pn/D1/.../Dk/U.rlu
            = fullUnitName + ".rlu";
        if (readableFileExists(unitRLUFile)) {

            Node* ruleFile = readRuleFile(driver, unitRLUFile);
            unitInfo->unitRLURulesEnd
                = unitInfo->dirRLURulesEnd + countRules(ruleFile);

            rules->appendChildren(ruleFile);
        }
    }

    vector<string> ruleFiles;
    ruleFiles.push_back(fullUnitName + ".rls");
    
    if (option("rules")) {
        vector<string> extraRuleFiles = optionVals("rules");
        ruleFiles.insert(ruleFiles.end(),
                         extraRuleFiles.begin(),
                         extraRuleFiles.end());
    }

    // Assemble list of rest of rules files to read
    vector<string> unitRuleFiles(unitInfo->getRuleFiles());
    ruleFiles.insert(ruleFiles.end(),
                     unitRuleFiles.begin(),
                     unitRuleFiles.end()
                     );

    
    // Do read of files

    for (int i = 0; i != (int) ruleFiles.size(); i++) {
        string ruleFile = ruleFiles.at(i);
        rules->appendChildren(readRuleFile(driver, ruleFile));
    }

    unitAST->addChild(rules);

    // -------------------------------------------------------------------
    // Read in and parse vcg or siv file
    // -------------------------------------------------------------------
        
    string vcFileExt(".vcg");
    if (option("siv")) vcFileExt = ".siv";

    printMessage(FINEm, "Reading VCG file");
    if (driver.parseVCGFile(fullUnitName + vcFileExt)) {
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
processUnit(UnitInfo* unitInfo, SMTDriver* smtDriver) {

    if (! unitInfo->includeUnit()) return;

    // Set globals used in formatting message headers and in CSV reports
    
    initCurrentUnitInfo(unitInfo);

    Timer unitTimer;
    
    if (option("utick")) {
        if (option("longtick")) {
            cout << endl << unitInfo->getUnitName();
        } else {
            cout << "*";
        }
        cout.flush();
    }


    printMessage(INFOm, "Starting unit processing");

    Node* unitAST = parseUnit(unitInfo);

    if (unitAST == 0) {
        printCSVRecord("error", "parsing failed");

        unitInfo->addRemark("parsing failed");
        printUnitSummary(unitInfo);
        return;
    }
    unitInfo->parseTreeSize = unitAST->treeSize();

    Formatter::setFormatter(VanillaFormatter::getFormatter());


    string status = elaborateUnit(unitAST, unitInfo);

    unitInfo->translatedUnitSize = unitAST->treeSize();

    if (status != "good") {
        printCSVRecord("error", status);

        unitInfo->addRemark(status);
        printUnitSummary(unitInfo);
        return;
    }

    smtDriver->driveUnit(unitAST, unitInfo);

    unitInfo->unitTime = unitTimer.getTime();
    unitInfo->nodeAllocCount = Node::getPoolAllocCount();
    printUnitSummary(unitInfo);

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
    string unitName;
    vector<string> nonOptions = processCommandArgs(argc, argv);

    if (nonOptions.size() > 0) {
        unitName = nonOptions.front();
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
    if (unitName != "") {
        unitList.push_back(UnitInfo(unitName));
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
        else if (optionVal("interface-mode") == "smtlib2") {
            
            smtDriver = new SMTLib2Driver();
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
        processUnit(&(*i), smtDriver);
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

