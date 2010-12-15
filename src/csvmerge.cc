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

// USAGE

// csvmerge filename1 m1 ... mj filename2 n1 ... nk
// 
// Create new csv record on stdout using
// fields m1 .. mj from filename1 and fields n1 .. nk from filename2.
// If j = 0 use all fields from filename1.
// If k = 0 use all fields from filename2.

// filename1 and filename2 must be same length.

#include <cstdlib>

#include <iostream>
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::istream;

#include<fstream>
using std::ifstream;

#include "utility.hh"

int
main (int argc, char *argv[]) {

    vector<string> args = processCommandArgs(argc, argv);

    
    ifstream ifs1;
    ifstream ifs2;

    bool readingSecondFileInfo = false;

    if (args.size() < 2) {
        cerr << "Too few args" << endl;
        exit(1);
    }
    
    string fileName1(args[0]);
    string fileName2;
    
    vector<int> cols1;
    vector<int> cols2;

    for (int i = 1; i != args.size(); i++) {
        if (isIntString(args[i])) {
            int p = stringToInt(args[i]);
            if (readingSecondFileInfo)
                cols2.push_back(p);
            else
                cols1.push_back(p);

        } else {
            readingSecondFileInfo = true;
            fileName2 = args[i];
        }
    }

    ifs1.open( fileName1.c_str() );
    if (!ifs1) {
        cerr << "Unable to open file 1: " << fileName1 << endl;
        exit(1);
    }
    ifs2.open( fileName2.c_str() );
    if (!ifs2) {
        cerr << "Unable to open file 2: " << fileName2 << endl;
        exit(1);
    }

    string line1;
    string line2;
    int numCols1(-1);
    int numCols2(-1);
    
    while (true) {
        bool notEOF1 = getline(ifs1, line1);
        bool notEOF2 = getline(ifs2, line2);
        if (notEOF1 != notEOF2) {
            cerr << "Files of differing lengths" << endl;
            exit(1);
        }
        if (!notEOF1) break;

        // cerr << "Read line pair " << endl;

        vector<string> vs1 = csvDigest(line1);
        vector<string> vs2 = csvDigest(line2);

        //      cerr << "vs1 size " << vs1.size() << endl;
        //      cerr << "vs2 size " << vs2.size() << endl;

        if (numCols1  < 0) numCols1 = vs1.size();
        else if (numCols1 != vs1.size()) {
            cerr << "File 1 number of cols changed from " << numCols1
                 << " to " << vs1.size() << " at line " << endl;
            cerr << line1 << endl;
            exit(1);
        }
        if (numCols2  < 0) numCols2 = vs2.size();
        else if (numCols2 != vs2.size()) {
            cerr << "File 2 number of cols changed from " << numCols2
                 << " to " << vs2.size() << " at line " << endl;
            cerr << line2 << endl;
            exit(1);
        }

        vector<string> ws;

        if (cols1.size() == 0) {
            ws = vs1;
        }
        for (int i = 0; i != cols1.size(); i++) {
            if (cols1[i] < 1 || cols1[i] > vs1.size()) {
                cerr << "Position " << cols1[i]
                     << " out of range for line from file 1" << endl;
                cerr << line1 << endl;
                continue;
            }
            ws.push_back(vs1[cols1[i] - 1]);
        }

        if (cols2.size() == 0) {
            ws.insert(ws.end(), vs2.begin(), vs2.end());
        }

        for (int i = 0; i != cols2.size(); i++) {
            if (cols2[i] < 1 || cols2[i] > vs2.size()) {
                cerr << "Position " << cols2[i]
                     << " out of range for line from file 2" << endl;
                cerr << line2 << endl;
                continue;
            }
            ws.push_back(vs2[cols2[i] - 1]);
        }

        cout << csvConcat(ws) << endl;
    } // while (true)
    return 0;
}

