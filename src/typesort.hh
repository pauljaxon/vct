//==========================================================================
//==========================================================================
// TYPESORT.HH
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


#ifndef TYPESORT_HH
#define TYPESORT_HH

// Class for doing topological sort on type declarations.

#include "node.hh"

#include <map>
using std::map;


class TypeSort {
public:
    static Node* sort(Node* typeDecls);

private:


    enum Colour {WHITE, GREY, BLACK};

    class Wrapper {
    public:

        Colour colour;
        Node* node;
        std::vector<Wrapper*> children;

        Wrapper(Node* n) {
            colour = WHITE;
            node = n;
        };

    };

    static map<std::string, Wrapper*> wmap;
    static Node* outNodes;
    
    static void dfs(std::vector<Wrapper*> vs);
};

#endif // ! defined TYPESORT_HH

