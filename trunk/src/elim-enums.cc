//========================================================================
//========================================================================
// ELIM-ENUMS.cc
//========================================================================
//========================================================================

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



//========================================================================
// Enumeration type abstraction 
//========================================================================

void abstractEnums(FDLContext* ctxt, Node* unit) {
    
    // Add the enum const declarations to the const declarations

    for ( map<string, Node*>::iterator i = ctxt->enumConstMap.begin();
          i != ctxt->enumConstMap.end();
          i++ ) {

        ctxt->constMap.insert(*i);
    }


    for (int i = 0; i != ctxt->typeSeq.arity(); i++) {
        Node* typeDecl = ctxt->typeSeq.child(i);

        if ( typeDecl->kind == DEF_TYPE
             && typeDecl->arity() == 1
             && typeDecl->child(0)->kind == ENUM_TY) {

            // Make type declaration abstract
            typeDecl->clearChildren();

        }
    }
}


//========================================================================
//========================================================================
// Replace enumeration types with subranges
//========================================================================
//========================================================================

//--------------------------------------------------------------------
// isEnumRule
//--------------------------------------------------------------------

bool isPosOrValEnumOp (Node* n) {
    if (n->kind != FUN_AP) return false;
    const string& s = n->id;
    return
        s != "character__pos"
        && 
        s != "character__val"
        &&
        (hasSuffix(s, "__pos") || hasSuffix(s, "__val"));
}

bool isEnumRule(Node* rule) {
    //    return rule->orOver<Node*, bool (&)(Node*)>(&isPosOrValOp);
    return rule->orOver(isPosOrValEnumOp);
}

//--------------------------------------------------------------------
// fixEnumRel
//--------------------------------------------------------------------

// E__LE, E__LT

// FUN_AP{<E>__LE}(X,Y) --> I_LE(X, Y)
// FUN_AP{<E>__LT}(X,Y) --> I_LT(X, Y)
// 
// ( EQ X Y <E>  will get fixed automatically by redefinition of <E>)

void fixEnumRel(Node* n) {
    if (n->kind != FUN_AP) return;

    if (hasSuffix(n->id, "__LE")) {
        n->id = "";
        n->kind = I_LE;
    }
    if (hasSuffix(n->id, "__LT")) {
        n->id = "";
        n->kind = I_LT;
    }
    return;
}

//--------------------------------------------------------------------
// enumsToIntSubranges
//--------------------------------------------------------------------


void enumsToIntSubranges(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);

    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add the enum const declarations to the const declarations
    // - - - - - - - - - - - - - - - - - - - - - - - - - -

    for ( map<string, Node*>::iterator i = ctxt->enumConstMap.begin();
          i != ctxt->enumConstMap.end();
          i++ ) {

        Node* decl = i->second;

        ctxt->insert(decl);
        //        ctxt->constMap.insert(*i);
    }

    int expectedNumRules = 0;

    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Save existing rules. Get ready to build new rule list
    // - - - - - - - - - - - - - - - - - - - - - - - - - -

    Node* oldRules = new Node(*rules);  // Use C++-defined copy constructor
    rules->clearChildren();

    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Iterate over each enum type declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int i = 0; i != ctxt->typeSeq.arity(); i++) {
        Node* typeDecl = ctxt->typeSeq.child(i);

        if ( typeDecl->kind == DEF_TYPE
             && typeDecl->arity() == 1
             && typeDecl->child(0)->kind == ENUM_TY) {

            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Add rules defining E__pos and E__val as identity functions
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            string enumSizeStr
                = intToString(typeDecl->child(0)->arity());
            string enumName = typeDecl->id;

                    
            rules->addChild
                ( new Node
                  ( FORALL,
                    new Node(SEQ, new Node(DECL, "i", nameToType(enumName))),
                    new Node(EQ,
                             "",
                             new Node(FUN_AP,
                                      enumName + "__pos",
                                      new Node(VAR, "i")
                                      ),
                             new Node(VAR, "i"),
                             Node::int_ty
                             )
                    ) // END FORALL
                  );
            rules->addChild
                ( new Node
                  ( FORALL,
                    new Node(SEQ, new Node(DECL, "i", nameToType(enumName))),
                    new Node(EQ,
                             "",
                             new Node(FUN_AP,
                                      enumName + "__val",
                                      new Node(VAR, "i")
                                      ),
                             new Node(VAR, "i"),
                             Node::int_ty
                             )
                    ) // END FORALL
                  );

            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Add rules defining E__succ and E__pred
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            string enumSizeMinusOneStr
                = intToString(typeDecl->child(0)->arity() - 1);

                    
            rules->addChild
                ( new Node
                  ( FORALL,
                    new Node(SEQ,
                             new Node(DECL, "i", nameToType(enumName))),
                    new Node(IMPLIES,
                             new Node(I_LT,
                                      new Node (VAR, "i"),
                                      new Node (NATNUM, enumSizeMinusOneStr)
                                      ),
                             new Node(EQ,"",
                                      new Node(FUN_AP,
                                               enumName + "__succ",
                                               new Node(VAR, "i")
                                               ),
                                      new Node(I_SUCC,
                                               new Node(VAR, "i")
                                               ),
                                      Node::int_ty
                                      )
                             ) // END IMPLIES
                    ) // END FORALL
                  );
            rules->addChild
                ( new Node
                  ( FORALL,
                    new Node(SEQ,
                             new Node(DECL, "i", nameToType(enumName))),
                    new Node(IMPLIES,
                             new Node(I_LT,
                                      new Node (NATNUM, "0"),
                                      new Node (VAR, "i")
                                      ),
                             new Node(EQ,"",
                                      new Node(FUN_AP,
                                               enumName + "__pred",
                                               new Node(VAR, "i")
                                               ),
                                      new Node(I_PRED,
                                               new Node(VAR, "i")
                                               ),
                                      Node::int_ty
                                      )
                             ) // END IMPLIES
                    ) // END FORALL
                  );
        
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // For each enum const, add a rule defining int value 
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            Node* enumConstNames = typeDecl->child(0);
                
            for (int j = 0;
                 j != enumConstNames->arity();
                 j++) {

                string enumConstId = enumConstNames->child(j)->id;  

                rules->addChild(
                                new Node (EQ,"", // MAY_BE_REPLACED_BY,
                                          new Node (CONST, enumConstId),
                                          new Node (NATNUM,
                                                    intToString(j)),
                                          Node::int_ty
                                          )
                                );
            }

            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Fix enumeration type decl to use int or integer subrange
            // - - - - - - - - - - - - - - - - - - - - - - - - - -

            int typeSize = typeDecl->child(0)->arity();
            Node* newEnumTy =
                new Node(SUBRANGE_TY,
                         new Node (NATNUM, "0"),
                         new Node (NATNUM, intToString(typeSize - 1))
                         );
                
            // Use new type in enum type decl
            typeDecl->child(0) = newEnumTy;

            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Compute expected number of rules
            // - - - - - - - - - - - - - - - - - - - - - - - - - -

            expectedNumRules += 14 + 2*typeSize;

        } // END if is enum type def
    } // END for loop over FDL decls


    // - - - - - - - - - - - - -  - - - - - - - - - - - - -
    // Bring over all existing rules except those for enumeration types
    // - - - - - - - - - - - - -  - - - - - - - - - - - - -

    int erules = 0;
    for (int i = 0; i!=oldRules->arity(); i++) {
            
        Node* rule = oldRules->child(i);

        if ( isEnumRule(rule) ) {
            erules++;
        } else {
            rules->addChild(rule);
        }
    }

    if (erules != expectedNumRules) {
        printMessage(WARNINGm, "elim-enums: found "
                     + intToString(erules) + " rules, expected "
                     + intToString(expectedNumRules) + " rules"
                     );
    }
    // - - - - - - - - - - - - -  - - - - - - - - - - - - - -
    // Replace E__LT, E__LE with integer versions
    // - - - - - - - - - - - - -  - - - - - - - - - - - - - -

    // FUN_AP{<E>__LE}(X,Y) --> I_LE(X, Y)
    // FUN_AP{<E>__LT}(X,Y) --> I_LT(X, Y)

    unit->mapOver(fixEnumRel);

    return;
}

