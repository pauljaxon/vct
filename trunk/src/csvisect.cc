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

// csvisect filename1 filename2
// 
// Only return list of those in filename1 that also occur in filename2.

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

    if (args.size() < 2) {
        cerr 
<< "Usage" << endl
<< "" << endl
<< "csvisect filename1 filename2" << endl
<< "" << endl
<< "Only return list of those records in filename1 that also " << endl
<< "occur in filename2" << endl;
        exit(1);
    }
    
    string fileName1(args[0]);
    string fileName2(args[1]);

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

    set<string> set1;
    
    while (getline(ifs1, line1)) {
        set1.insert(line1);
    }
    while (getline(ifs2, line2)) {
        if (set1.find(line2) != set1.end())
            cout << line2 << endl;
    }
    return 0;
}

