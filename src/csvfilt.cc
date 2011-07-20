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

// USAGE:

// csvfilt [-v] n str [filename]

// Filter csv records, returning just those with str a substring of field
// n (1-based).

// If -v provided, then return those records without str a substring of field
// n.

#include <cstdlib>

#include <iostream>
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::istream;

#include "utility.hh"

#include<fstream>
using std::ifstream;

int
main (int argc, char *argv[]) {

    vector<string> nonOptions = processCommandArgs(argc, argv);

    if (nonOptions.size() < 2) {
        cerr 
<< "Usage" << endl
<< endl
<< "  csvfilt [-v] n str [filename] " << endl
<< endl
<< "Filter csv (comma-separated-value) records on stdin, returning" << endl
<< "just those with str a substring of field n (1-based)." << endl
<< endl
<< "Options: " << endl
<< "  -v        Select records without str a substring of field n." << endl
<< "  filename  Read input from file filename rather than stdin" << endl;
        return 0;
    }
    istream* isp = &cin;
    
    ifstream ifs;

    if (nonOptions.size() == 3) {
            
        string fileName (nonOptions[2]);

        ifs.open( fileName.c_str() );
        if (!ifs) {
            cerr << "Unable to open file " << fileName << endl;
            exit(1);
        }
        isp = &ifs;
    }



    int pos = stringToInt(nonOptions[0]);
    string tag = nonOptions[1];

    string line;
    while (getline(*isp, line)) {
        vector<string> vs = csvDigest(line);
        if (pos > vs.size()) {
            cerr << "Position " << pos << " out of range for line" << endl;
            cerr << line << endl;
            continue;
        }
        bool tagfound = vs[pos-1].find(tag) != string::npos;

        if (tagfound ^ option("v")) {
            cout << line << endl;
        }
    }
    return 0;
}

/*

    string s1("abc,d \"e,f\", ghi   ");
    string s2("  123, 4 \"56");

    vector<string> v1 = csvDigest(s1);
    vector<string> v2 = csvDigest(s2);

    for (int i = 0; i != v1.size(); i++) {
        cout << "#" << v1[i] << "#" << endl;
    }
    cout << endl;
    for (int i = 0; i != v2.size(); i++) {
        cout << "#" << v2[i] << "#" << endl;
    }


*/
