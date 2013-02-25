/*
This file is part of Victor: a SPARK VC Translator and Prover Driver.

Copyright (C) 2009-2013 University of Edinburgh

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

// csvmerge [-h] filename_1 n_1_1 ... n_1_k1 filename_2 n_2_1 ... n_2_k2 ...
//               filename_m n_m_1 ... n_m_km
// 
// Repeatedly read a CSV record from each file filename_1 ... filename_m
// and create new CSV record on stdout that combines, for each i from 1 to m, 
// the fields n_i_1 ... n_i_ki from the record from filename_i.
// If ki = 0 use all fields from the record from filename_i.
// The n_i_j are 1-based field positions.
//
// Options
//
// -h Add a header record to the output giving the filename origin of each
//    field.  To keep headings concise, filename suffixes after the
//    first '.' character are stripped.

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

    if (args.size() < 1) {
        cerr 
<< "USAGE" << endl
<< endl
<< "csvmerge [-h] filename_1 n_1_1 ... n_1_k1 filename_2 n_2_1 ... n_2_k2 ... " << endl
<< "              filename_m n_m_1 ... n_m_km" << endl
<< endl
<< "Repeatedly read a CSV record from each file filename_1 ... filename_m" << endl
<< "and create new CSV record on stdout that combines, for each i from 1 to m, " << endl
<< "the fields n_i_1 ... n_i_ki from the record from filename_i." << endl
<< "If ki = 0 use all fields from the record from filename_i." << endl
<< "The n_i_j are 1-based field positions." << endl
<< endl
<< "Options" << endl
<< endl
<< "-h Add a header record to the output giving the filename origin of each" << endl
<< "   field.  To keep headings concise, filename suffixes after the" << endl
<< "   first '.' character are stripped." << endl
;
        return 0;
    }


    // Use of ifstream* rather than ifstream type:
    // While ifstream objects are described as pointers, the copy
    // constructor is private, so it is not straightforward to add
    // ifstream objects to a vector.  Hence we use pointers, and
    // allocate the ifstream objects themselves on the heap.

    
    vector<ifstream*> ifstreams;
    vector<string> fileNames;
    vector< vector<int> > activeFields;

    int fileNum = -1;

    // Scan arguments. Gather names of input files and fields to select from each
    for (int i = 0; i != args.size(); i++) {

        if (!isIntString(args[i])) {
            fileNum++;

            // args[i] is name of input file

            string& fileName = args[i];
            fileNames.push_back(fileName);

            ifstream* inStreamP = new ifstream();
            ifstreams.push_back(inStreamP);

            inStreamP->open( fileName.c_str() );
            if (inStreamP->fail()) {
                cerr << "Unable to open file " << fileName << endl;
                exit(1);
            }
            activeFields.push_back( vector<int>() );
        }
        else {
            // args[i] is integer for field of current input file
            int fieldNum = stringToInt(args[i]);
            if (fileNum == -1) {
                cerr << "Unexpected integer before first file name " << fieldNum<< endl;
                exit(1);
            }
            activeFields.at(fileNum).push_back(fieldNum);
        }
    }
    int numFiles = fileNum + 1;

    // 
    if (numFiles < 1) {
        cerr << "At least 1 file to merge must be specified" << endl; 
        exit(1);
    }
    
    vector<string> lines(numFiles);
    vector<int> numFields(numFiles, -1);
    vector< vector<string> > fields(numFiles);
    
    int currentRecord = 0;  
    while (true) {
        currentRecord++;   // 1-based count for error messages

        bool notEOF0 = false;
        for (int i = 0; i != numFiles; i++) {
            // 
            bool notEOFi = getline(*(ifstreams.at(i)), lines.at(i));
            if (i == 0)
                notEOF0 = notEOFi;
            else {
                if (notEOFi != notEOF0 ) {
                    cerr << "Files of differing lengths" << endl;
                    exit(1);
                }
            }
        }
        // Invariant: All input streams at EOF, or all not at EOF and line from each read.

        if (!notEOF0) break; // Stop when all input streams at EOF.
        
        vector<string> headerFields;
        vector<string> outputFields;

        for (int i = 0; i != numFiles; i++) {
            
            vector<string> fields = csvDigest(lines.at(i));

            // Check number of columns in input file is constant for all records
            if (numFields.at(i) == -1)
                numFields.at(i) = fields.size();
            else if (numFields.at(i) != fields.size()) {
                cerr << "In File " << fileNames.at(i) << " at line " << currentRecord 
                     << ", number of fields changed from " << numFields.at(i)
                     << " to " << fields.size() << endl;
                exit(1);
            }
            // If no active fields specified, use all fields
            if (activeFields.at(i).size() == 0) {
                if (option("h") && currentRecord == 1) {
                    for (int k = 0; k != fields.size(); k++ ) {
                        headerFields.push_back(substringBefore('.',fileNames.at(i)));
                    }
                }
                outputFields.insert(outputFields.end(), fields.begin(), fields.end());

            }
            // Otherwise, copy over the active fields
            else {
                for (int j = 0; j != activeFields.at(i).size(); j++) {
                    if (activeFields.at(i).at(j) < 1 ||
                        activeFields.at(i).at(j) > fields.size()) {

                        cerr << "Position " << activeFields.at(i).at(j)
                             << " out of range for records from file "
                             << fileNames.at(i) << endl;
                        exit(1);
                    }

                    if (option("h") && currentRecord == 1) {
                        headerFields.push_back(substringBefore('.',fileNames.at(i)));
                    }
                    outputFields.push_back(fields.at(activeFields.at(i).at(j) - 1) );
                }
                    
            }
        } // End for i = 0 .. numFiles-1.

        if (option("h") && currentRecord == 1) {
            cout << csvConcat(headerFields) << endl;
        }
        cout << csvConcat(outputFields) << endl;
        
    } // End while true        

    return 0;
}

