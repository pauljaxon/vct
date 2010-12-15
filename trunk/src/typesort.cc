//==========================================================================
//==========================================================================
// TYPESORT.CC
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


#include <cassert>


#include <stdexcept>  

#include "typesort.hh"
using std::vector;
using std::string;
using std::set;

#include "utility.hh"   // For string manipulation functions

#include <algorithm>

void
TypeSort::dfs(vector<Wrapper*> vs) {

    for (vector<Wrapper*>::iterator i = vs.begin();
         i != vs.end();
         i++ ) {

        Wrapper* v = *i;

        if (v->colour == GREY)
            throw std::runtime_error("Circular dependency in type decls");
        else if (v->colour == BLACK)
            continue;
        else {
            v->colour = GREY;

            dfs(v->children);

            v->colour = BLACK;

            outNodes->addChild(v->node);
        }

    }
}


map<std::string, TypeSort::Wrapper*> TypeSort::wmap;
Node* TypeSort::outNodes;



Node*
TypeSort::sort(Node* typeDecls) {

    vector<Wrapper*> inVertices;

    Node* newDecls = new Node(z::SEQ);
    
    // Initialise 

    wmap.clear();
    outNodes = new Node(z::SEQ);

    for (int i = 0; i != typeDecls->arity(); i++) {
        Node* typeDecl = typeDecls->child(i);

        Wrapper* w = new Wrapper(typeDecl);
        inVertices.push_back(w);
        wmap.insert(std::make_pair(typeDecl->id, w));
    }

    // Build dependency graph

    // Iterate over wrapped initial declarations, adding child info
    // Further abstract declarations are added when missing 

    for (int i = 0; i != typeDecls->arity(); i++) {
        Wrapper* w = inVertices.at(i);
        
        set<string> typeIds = w->node->getIds(z::TYPE_ID);

        for (std::set<string>::const_iterator j = typeIds.begin();
             j != typeIds.end();
             j++) {
            string id = *j;

            // Create declaration for undeclared type ids

            if ( wmap.count(id) == 0) {

                printMessage(WARNINGm,
                             "Creating abstract type decl for type id: " + id);

                Node* newDecl = new Node(z::DEF_TYPE, id);
                
                newDecls->addChild(newDecl);
                
                Wrapper* newW = new Wrapper(newDecl);
                wmap.insert(std::make_pair(id, newW));
                inVertices.push_back(newW);

            } 
            
            Wrapper* wrappedIdDecl = wmap.find(id)->second;
            w->children.push_back(wrappedIdDecl);


        }
    }

    // Do sort.
    
    try {

        dfs(inVertices);

    } catch (std::runtime_error r) {
        
        for (vector<Wrapper*>::iterator i = inVertices.begin();
             i != inVertices.end();
             i++ ) {
            delete *i;
        }
        throw;
    }

    // Cleanup

    for (vector<Wrapper*>::iterator i = inVertices.begin();
         i != inVertices.end();
         i++ ) {
        delete *i;
    }

    return new Node(z::SEQ, outNodes, newDecls);
}



