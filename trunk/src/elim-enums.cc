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
    // With read-all-decl-files-in-dir option, Victor reads in enum type
    // declarations without corresponding rule sets, making the
    // expectedNumRules calculation incorrect, and this check pointless.
    
    if (!option("read-all-decl-files-in-dir")
        && erules != expectedNumRules) {
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

//========================================================================
//========================================================================
// Add in axioms for isomorphism between abstract type and integer subrange
//========================================================================
//========================================================================

// Eventually should use these macros elsewhere for clarity and conciseness

#define nDECL(id,T) new Node(DECL,(id),(T))
#define nSEQ1(x) new Node(SEQ,(x))
#define nSEQ2(x,y) new Node(SEQ,(x),(y))
#define nSEQ3(x,y,z) new Node(SEQ,"",(x),(y),(z))

#define nFUNAP1(f,a1) new Node(FUN_AP,(f),(a1))
#define nFUNAP2(f,a1,a2) new Node(FUN_AP,(f),(a1),(a2))
#define nFUNAP3(f,a1,a2,a3) new Node(FUN_AP,(f),(a1),(a2))

#define nNATNUM(n) new Node(NATNUM,(n))
#define nCONST(c) new Node(CONST,(c))
#define nVAR(v) new Node(VAR,(v))

#define nINT_TY new Node(INT_TY)
#define nTYPE_ID(id) new Node(TYPE_ID,(id))

#define nFORALL(ds,p) new Node(FORALL, (ds),(p))
#define nFORALL1(v,T,p) nFORALL(nSEQ1(nDECL((v),(T))), (p))
#define nFORALL2(v1,T1,v2,T2,p) \
    nFORALL( nSEQ2( nDECL((v1),(T1)), nDECL((v2),(T2))), (p))
#define nIMPLIES(p,q) new Node(IMPLIES,(p),(q))
#define nNOT(p) new Node(NOT,(p))
#define nAND(p,q) new Node(AND,(p),(q))
#define nIFF(p,q) new Node(IFF,(p),(q))

#define nI_PLUS(i,j) new Node(I_PLUS,(i),(j))
#define nI_MINUS(i,j) new Node(I_MINUS,(i),(j))
#define nI_LE(i,j) new Node(I_LE,(i),(j))
#define nI_LT(i,j) new Node(I_LT,(i),(j))

#define nEQ(a,b,T) new Node(EQ,"",(a),(b),(T))
#define nNE(a,b,T) nNOT(nEQ((a),(b),(T)))


void axiomatiseEnums(FDLContext* ctxt, Node* unit) {

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

            Node* enumType = typeDecl->child(0);
            int enumSize = enumType->arity();    // Also referred to as k below
            string enumSizeStr = intToString(enumSize);
            string enumSizeMinusOneStr = intToString(enumSize-1);
            string enumName = typeDecl->id;

            // Type introduces constants econst{0} ... econst{k-1} 

            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Rules for E__val of positions and E__pos of econsts
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // E__val(i) == econst{i}   i = 0 .. k-1
            // E__pos(econst{i}) == i     i = 0 .. k-1

            for (int i = 0; i != enumSize; i++) {
                rules->addChild(
                    nEQ( nFUNAP1( enumName + "__val",
                                  nNATNUM( intToString(i))),
                         nCONST( enumType->child(i)->id),
                         nTYPE_ID(enumName))
                    );
                rules->addChild(
                    nEQ( nFUNAP1( enumName + "__pos",
                                  nCONST( enumType->child(i)->id)),
                         nNATNUM( intToString(i)),
                         nINT_TY)
                    );
            }

            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Rules for E__pos of E__first and E__last
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // E__pos(E__first) == 0
            // E__pos(E__last) == k-1

            rules->addChild
                ( nEQ( nFUNAP1( enumName + "__pos",
                                nCONST( enumName + "__first")),
                       nNATNUM( "0"),
                       nINT_TY)
                    );
            
            rules->addChild
                ( nEQ( nFUNAP1( enumName + "__pos",
                                nCONST( enumName + "__last")),
                       nNATNUM( enumSizeMinusOneStr),
                       nINT_TY)
                    );
            
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Rules for E__pos of E__succ and E__pred
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // All X:E. X != const(k-1) 
            //    ==> E__pos(E__succ(X)) == E__pos(X) + 1

            rules->addChild(
                nFORALL1(
                    "X", nTYPE_ID(enumName),
                    nIMPLIES(
                        nNE( nVAR("X"),
                             nCONST( enumName + "__last"),
                             nTYPE_ID(enumName)),
                        nEQ( nFUNAP1( enumName + "__pos",
                                      nFUNAP1( enumName + "__succ",
                                               nVAR("X"))),
                             nI_PLUS( nFUNAP1( enumName + "__pos",
                                               nVAR("X")),
                                      nNATNUM("1")),
                             nINT_TY)))
                );

            // All X:E. X != const(0)   ==> E__pos(E__pred(X)) == E__pos(X) - 1

            rules->addChild(
                nFORALL1(
                    "X", nTYPE_ID(enumName),
                    nIMPLIES(
                        nNE( nVAR("X"),
                             nCONST( enumName + "__first"),
                             nTYPE_ID(enumName)),
                        nEQ( nFUNAP1( enumName + "__pos",
                                      nFUNAP1( enumName + "__pred",
                                               nVAR("X"))),
                             nI_MINUS( nFUNAP1( enumName + "__pos",
                                                nVAR("X")),
                                       nNATNUM("1")),
                             nINT_TY)))
                );

            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Rules for E__pos bounds
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // All X:E. 0 <= E__pos(X)
            // All X:E. E__pos(X) <= k-1

            rules->addChild(
                nFORALL1("X", nTYPE_ID(enumName),
                         nI_LE( nNATNUM("0"),
                                nFUNAP1( enumName + "__pos", nVAR("X"))))
                );

            rules->addChild(
                nFORALL1("X", nTYPE_ID(enumName),
                         nI_LE( nFUNAP1( enumName + "__pos", nVAR("X")),
                                nNATNUM(enumSizeMinusOneStr)))
                );

            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Rules for E__succ and E__pred of E__val
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // All I:Int. 0 <= I & I < k-1
            //   ==> E__succ(E__val(I)) == E__val(I+1)

            rules->addChild(
                nFORALL1(
                    "I", nINT_TY,
                    nIMPLIES(
                        nAND( nI_LE( nNATNUM("0"), nVAR("I")),
                              nI_LT( nVAR("I"),
                                     nNATNUM( enumSizeMinusOneStr))),
                        nEQ( nFUNAP1( enumName + "__succ",
                                      nFUNAP1( enumName + "__val", nVAR("I"))),
                             nFUNAP1( enumName + "__val",
                                      nI_PLUS( nVAR("I"), nNATNUM("1"))),
                             nTYPE_ID(enumName))))
                );


            // All I:Int. 0 < I & I <= k-1
            //   ==> E__pred(E__val(I)) == E__val(I-1)

            rules->addChild(
                nFORALL1(
                    "I", nINT_TY,
                    nIMPLIES(
                        nAND( nI_LT( nNATNUM("0"), nVAR("I")),
                              nI_LE( nVAR("I"),
                                     nNATNUM( enumSizeMinusOneStr))),
                        nEQ( nFUNAP1( enumName + "__pred",
                                      nFUNAP1( enumName + "__val",
                                               nVAR("I"))),
                             nFUNAP1( enumName + "__val",
                                      nI_MINUS( nVAR("I"), nNATNUM("1"))),
                             nTYPE_ID(enumName))))
                );

            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Rules for isomorphism
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // All I:Int. 0 <= I & I <= k-1 ==> E__pos(E__val(I)) == I

            rules->addChild(
                nFORALL1(
                    "I", nINT_TY,
                    nIMPLIES(
                        nAND( nI_LE( nNATNUM("0"), nVAR("I")),
                              nI_LE(
                                  nVAR("I"),
                                  nNATNUM( enumSizeMinusOneStr))),
                        nEQ( nFUNAP1( enumName + "__pos",
                                      nFUNAP1( enumName + "__val",
                                               nVAR("I"))),
                             nVAR("I"),
                             nINT_TY)))
                );

            // All X:E. E__val(E__pos(X)) == X

            rules->addChild(
                nFORALL1(
                    "X", nTYPE_ID(enumName),
                    nEQ( nFUNAP1( enumName + "__val",
                                  nFUNAP1( enumName + "__pos", nVAR("X"))),
                         nVAR("X"),
                         nTYPE_ID(enumName))
                    ) // END FORALL
                );
            
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Rules for inequalities
            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // All X,Y:E.  (X <= Y)   <==>  (E__pos(X) <= E__pos(Y))

            rules->addChild(
                nFORALL2(
                    "X", nTYPE_ID(enumName),"Y", nTYPE_ID(enumName),
                    nIFF( nFUNAP2(enumName + "__LE", nVAR("X"), nVAR("Y")),
                          nI_LE( nFUNAP1( enumName + "__pos", nVAR("X")),
                                 nFUNAP1( enumName + "__pos", nVAR("Y")))))
                );

            // All X,Y:E.  (X < Y)    <==>  (E__pos(X) <  E__pos(Y))
            rules->addChild(
                nFORALL2(
                    "X", nTYPE_ID(enumName), "Y", nTYPE_ID(enumName),
                    nIFF(
                        nFUNAP2( enumName + "__LT", nVAR("X"), nVAR("Y")),
                        nI_LT(
                            nFUNAP1( enumName + "__pos", nVAR("X")),
                            nFUNAP1( enumName + "__pos", nVAR("Y")))))
                );

            // All I,J:Int. 0 <= I & I <= k-1 & 0 <= J & J <= k-1 
            //   ==>   I <= J   <==>    E__val(I) <= E__val(J)

            rules->addChild(
                nFORALL2(
                    "I", nINT_TY,"J", nINT_TY,
                    nIMPLIES(
                        nAND( nAND( nI_LE( nNATNUM("0"), nVAR("I")),
                                    nI_LE( nVAR("I"),
                                           nNATNUM( enumSizeMinusOneStr))),
                              nAND( nI_LE( nNATNUM("0"), nVAR("J")),
                                    nI_LE( nVAR("J"),
                                           nNATNUM( enumSizeMinusOneStr)))),
                        nIFF( nI_LE( nVAR("I"), nVAR("J")),
                              nFUNAP2(
                                  enumName + "__LE",
                                  nFUNAP1( enumName + "__val", nVAR("I")),
                                  nFUNAP1( enumName + "__val", nVAR("J"))))))
                );


            // All I,J:Int. 0 <= I & I <= k-1 & 0 <= J & J <= k-1 
            //   ==>   I < J   <==>    E__val(I) < E__val(J)

            rules->addChild(
                nFORALL2(
                    "I", nINT_TY,"J", nINT_TY,
                    nIMPLIES(
                        nAND( nAND( nI_LE( nNATNUM("0"), nVAR("I")),
                                    nI_LE( nVAR("I"),
                                           nNATNUM( enumSizeMinusOneStr))),
                              nAND( nI_LE( nNATNUM("0"), nVAR("J")),
                                    nI_LE( nVAR("J"),
                                           nNATNUM( enumSizeMinusOneStr)))),
                        nIFF(
                            nI_LT( nVAR("I"), nVAR("J")),
                            nFUNAP2(
                                enumName + "__LT",
                                nFUNAP1( enumName + "__val", nVAR("I")),
                                nFUNAP1( enumName + "__val", nVAR("J"))))))
                );


            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Make type declaration abstract
            // - - - - - - - - - - - - - - - - - - - - - - - - - -

            typeDecl->clearChildren();

            // - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Update expected number of old enum type rules
            // - - - - - - - - - - - - - - - - - - - - - - - - - -

            expectedNumRules += 14 + 2*enumSize;

        } // END if is enum type def
    } // END for loop over FDL decls


    // - - - - - - - - - - - - -  - - - - - - - - - - - - -
    // Bring over all existing rules except those for enumeration types
    // - - - - - - - - - - - - -  - - - - - - - - - - - - -

    int numEliminatedRules = 0;
    for (int i = 0; i!=oldRules->arity(); i++) {
            
        Node* oldRule = oldRules->child(i);

        if ( isEnumRule(oldRule) ) {
            numEliminatedRules++;
        } else {
            rules->addChild(oldRule);
        }
    }
    // With read-all-decl-files-in-dir option, Victor reads in enum type
    // declarations without corresponding rule sets, making the
    // expectedNumRules calculation incorrect, and this check pointless.
    
    if (!option("read-all-decl-files-in-dir")
        && numEliminatedRules != expectedNumRules) {
        printMessage(WARNINGm, "elim-enums: found "
                     + intToString(numEliminatedRules) + " rules, expected "
                     + intToString(expectedNumRules) + " rules"
            );
    }

    return;
}


