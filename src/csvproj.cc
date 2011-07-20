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

// csvproj n1 ... nk [filename]

// n1 ... nk are field numbers (1-based)
// filename is input filename to use.
// If filename not provided, cin is used instead.

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

    vector<string> nonOptions = processCommandArgs(argc, argv);

    if (nonOptions.size() < 1) {
        cerr 
<< "Usage" << endl
<< endl
<< "  csvproj n1 ... nk [filename]  " << endl
<< endl
<< "Project out selected fields of input records."<< endl
<< "n1 ... nk are the (1-based) numbers of the fields to select." << endl
<< "filename is input filename to use." << endl
<< "If filename is not provided, stdin is used instead." << endl
<< "The result is written to stdout." << endl;
        return 0;
    }

    istream* isp = &cin;
    
    ifstream ifs;

    if (! isIntString(nonOptions.back()) ) {
            
        string fileName (nonOptions.back());
        nonOptions.pop_back();

        ifs.open( fileName.c_str() );
        if (!ifs) {
            cerr << "Unable to open file " << fileName << endl;
            exit(1);
        }
        isp = &ifs;
    }

    string line;
    while (getline(*isp, line)) {
        vector<string> vs = csvDigest(line);
        vector<string> ws;
        
        for (int i = 0; i != nonOptions.size(); i++) {
            int pos = stringToInt(nonOptions[i]);
            if (pos > vs.size()) {
                cerr << "Position " << pos << " out of range for line" << endl;
                cerr << line << endl;
                continue;
            }
            ws.push_back(vs[pos-1]);
        }
        cout << csvConcat(ws) << endl;
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
