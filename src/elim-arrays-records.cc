//========================================================================
//========================================================================
// ELIM-ARRAYS-RECORDS.CC
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
//========================================================================
// Abstracting interpreted arrays
//========================================================================
//========================================================================


//========================================================================
// Eliminate subrange expressions in array indexes
//========================================================================

Node* elimSubrangeExp(Node* n) {
    if (n->kind == SUBRANGE) {
        printMessage(WARNINGm, 
                     "Unsoundly eliminating array subrange expression");
        return n->child(0);
    }
    else 
        return n;
}


void elimSubranges(FDLContext* ctxt, Node* unit) {

    unit->mapOver1(elimSubrangeExp);
    return;
}



//========================================================================
// Eliminate array constructors
//========================================================================

Node* 
elimMkArray(Node* oldN, Node* defaultArray, string constArray) {


    // 1. MK_ARRAY{arr-typeid}  e0  ASSIGN(index<1>, e1) ...
    //                              ASSIGN(index<n>, en)   
    //    --> node<n>
    //
    // 2. MK_ARRAY{arr-typeid} ASSIGN(index<1>, e1) ... ASSIGN(index<n>, en) 
    //    --> node<n>
    //
    // where
    //
    // index<i> is an index expression: a tuple of either all index values
    // or all index ranges.
    //
    // node<0> = FUN_AP{constArray} e0 (case 1)
    // node<0> = defaultArray (case 2)
    // node<i> = ARR_UPDATE{arr-typeid}(node<i-1>, index<i>, e<i>)  
    //         | ARR_BOX_UPDATE{arr-typeid}(node<i-1>, index<i>, e<i>)  
    //   i = 1..n
    // 
    // ARR_BOX_UPDATE used when index<i> is tuple of index ranges.
    // 
    // constArray should name a function which maps expressions to 
    //   constant arrays with value that expression:
    //   Typically constArray(x) = \lambda y . x

    Node* newN;
    string arrTypeId = oldN->id;
    int start;
    if (oldN->child(0)->kind == z::ASSIGN) {
        newN = defaultArray->copy();
        start = 0;
    } else {
        newN = new Node(FUN_AP, constArray, oldN->child(0));
        start = 1;
    }
    for (int i = start; i != oldN->arity(); i++) {
        Node* assignN = oldN->child(i);

        Kind updateKind =
            (assignN->child(0)->child(0)->kind == SUBRANGE)
            ? z::ARR_BOX_UPDATE
            : z::ARR_UPDATE;

        newN = new Node(updateKind,
                        arrTypeId,
                        newN,
                        assignN->child(0),
                        assignN->child(1)
                        );
    }
    return newN;
}

Node* elimArrayConstructors(Node* n) {

    if (n->kind == MK_ARRAY) {

        string arrayName = n->id;

        string defaultArrName = arrayName + "___default_arr";
        string constArrName = arrayName + "___mk_const_arr";

        Node* defaultArray = new Node(CONST, defaultArrName);
        return elimMkArray(n, defaultArray, constArrName);
    }
    return n;
}


void elimMkArrays(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);
    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Iterate over each array type declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int i = 0; i != ctxt->typeSeq.arity(); i++) {
        Node* typeDecl = ctxt->typeSeq.child(i);

        if ( typeDecl->kind == DEF_TYPE
             && typeDecl->arity() == 1
             && typeDecl->child(0)->kind == ARRAY_TY) {

            // For array type ArrTy = Array(IndexTy1,...,IndexTyn, ValTy),

            // Add declarations
            //   default_arr  : ArrTy
            //   mk_const_arr : (ValTy)ArrTy
            // and add rule
            //   forall i1:IndexTy1.
            //   ...
            //   forall in:IndexTyn.
            // forall v:ValTy.  mk_const_arr(v)[i1...in] = v


            string arrayName = typeDecl->id;
            Node* IndexTySeq = typeDecl->child(0)->child(0);
            int arrayDim =  IndexTySeq->arity();
            Node* ValTy  = typeDecl->child(0)->child(1);

            string defaultArrName = arrayName + "___default_arr";
            string constArrName = arrayName + "___mk_const_arr";
                    
            ctxt->insert(new Node( DEF_CONST,
                                  defaultArrName, 
                                  new Node (TYPE_ID,arrayName)));
            ctxt->insert(new Node( DECL_FUN,
                                  constArrName, 
                                  new Node (SEQ,ValTy->copy()),
                                  new Node (TYPE_ID,arrayName)));

            Node* qDecls = new Node(SEQ);

            Node* arrIndex = new Node(SEQ);
            
            for (int j = 1; j <= arrayDim; j++) {
                qDecls->addChild(new Node (DECL,
                                           "i" + intToString(j),
                                           IndexTySeq->child(j-1)->copy()));
                arrIndex->addChild(new Node(VAR,
                                            "i" + intToString(j)));
            }
            qDecls->addChild(new Node (DECL,
                                       "v",
                                       ValTy->copy()));
            Node* constArrAxiom = 
                 new Node (FORALL,
                           qDecls,
                           new Node (EQ,"",
                                     new Node(ARR_ELEMENT,
                                              arrayName,
                                              new Node(FUN_AP,
                                                       constArrName,
                                                       new Node (VAR, "v")),
                                              arrIndex),
                                     new Node(VAR, "v"),
                                     ValTy->copy()
                                     ));

            rules->addChild(nRULE("arr " + intToString(arrayDim)
                                  + " mk_const def (" + arrayName + ")",
                                  constArrAxiom));
                
        } // END if is array type decl
    } // END for loop over type decl seq.

    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Eliminate all occurrences of constructors
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    unit->mapOver1(elimArrayConstructors);
        
}

//========================================================================
// Eliminate record constructors
//========================================================================

Node* 
elimMkRecord(Node* oldN, Node* defaultRecord) {

    // MK_RECORD{rcd-typeid} (ASSIGN{i1}, e1) ... (ASSIGN{in}, en) 
    // --> node<n>
    //
    // where
    //
    // node<0> = defaultRecord
    // node<i> = RCD_UPDATE{i1}(node<i-1>, e1, TYPE_PARAM{rcd-typeid}) i = 1..n

    Node* newN = defaultRecord->copy();
    for (int i = 0; i != oldN->arity(); i++) {
        Node* assignN = oldN->child(i);
        newN = new Node(z::RCD_UPDATE,
                        assignN->id,
                        newN,
                        assignN->child(0),
                        new Node(z::TYPE_PARAM, oldN->id)
                        );
    }
    return newN;
}

Node* elimRecordConstructors(Node* n) {

    if (n->kind == MK_RECORD) {

        string recordName = n->id;
        string defaultRecordName = recordName + "___default_rcd";

        Node* defaultRecord = new Node(CONST, defaultRecordName);
        return elimMkRecord(n, defaultRecord);
    }
    return n;
}


void elimMkRecords(FDLContext* ctxt, Node* unit) {

    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Iterate over each record type declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int i = 0; i != ctxt->typeSeq.arity(); i++) {
        Node* typeDecl = ctxt->typeSeq.child(i);

        if ( typeDecl->kind == DEF_TYPE
             && typeDecl->arity() == 1
             && typeDecl->child(0)->kind == RECORD_TY) {

            // Add declaration
            //   default_rcd  : RecordTy

            string recordName = typeDecl->id;

            string defaultRecordName = recordName + "___default_rcd";
                    
            ctxt->insert(new Node( DEF_CONST,
                                  defaultRecordName, 
                                  new Node (TYPE_ID,recordName)));

        } // END if is record type decl
    } // END for loop over type decl seq.

    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Eliminate all occurrences of constructors
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    unit->mapOver1(elimRecordConstructors);
        
}


//==========================================================================
// Add array axioms
//==========================================================================

//-------------------------------------------------------------------------
// mkEqAliasRel
//-------------------------------------------------------------------------

Node* mkEqAliasRel(const string& typeName, Node* lhs, Node* rhs) {

    Kind k = (option("bit-type")) ? PRED_AP : FUN_AP;

    return new Node(k, typeName + "___eq", lhs, rhs);
}


//-------------------------------------------------------------------------
// addArrayElementUpdateAxioms
//-------------------------------------------------------------------------

void
addArrayElementUpdateAxioms(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);
    
    for (map<string,Node*>::iterator i = ctxt->typeMap.begin();
         i != ctxt->typeMap.end();
         i++ ) {

        Node* typeDecl = i->second;
        //   DEF_TYPE {id}  type 
        // | DEF_TYPE {id}
        if (typeDecl->arity() == 0) continue;

        Node* type = typeDecl->child(0);

        if (type->kind != ARRAY_TY) continue;

        // typeDecl is
        // DEF_TYPE {A} (ARRAY_TY{A} (SEQ S1 ... Sn) T)

        string arrName = typeDecl->id;
        Node* indexTypes = type->child(0);
        Node* elementType = type->child(1);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Add axioms
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        // Build iterated components of axioms.

        Node* sDecls = new Node(SEQ);    // s1:S1, ... , sn:Sn,
        Node* spDecls = new Node(SEQ);   // s1p:S1, ... , snp:Sn,
        Node* sIndexes = new Node(SEQ);  // s1, ..., sn
        Node* spIndexes = new Node(SEQ); // s1p, ..., snp

        Node* eqIndexes = new Node(AND); // s1 =_S1 s1p /\ ... /\  sn = Sn snp

        for (int i = 0; i != indexTypes->arity(); i++) {

            string s = "s"+ intToString(i);
            string sp = s + "p";

            Node* indexType = indexTypes->child(i);
            
            sDecls->addChild(new Node (DECL, s, indexType->copy()));
            spDecls->addChild(new Node (DECL, sp, indexType->copy()));
            sIndexes->addChild(new Node (VAR, s));
            spIndexes->addChild(new Node (VAR, sp));
            eqIndexes->addChild(new Node (EQ,"",
                                          new Node(VAR, s),
                                          new Node(VAR, sp),
                                          indexType->copy()  ));
            
        }
        if (eqIndexes->arity() == 1) {
            eqIndexes = eqIndexes->child(0);
        }
        
        // RWEq axiom:
        //
        // All a:A, s1:S1, ... , sn:Sn, t:T.
        //  ARR_ELEMENT{A} (ARR_UPDATE{A}(a, (SEQ s1 ... sn), t)
        //                 (SEQ s1 ... sn)
        //  =_T t
        //  
        Node* rWEqDecls = new Node(SEQ,
                                   new Node(DECL, "a", nameToType(arrName)));
        rWEqDecls->appendChildren(sDecls);
        rWEqDecls->addChild(new Node (DECL, "t", elementType->copy()));

        Node* rWEqAxiom =
            new Node(FORALL,
                     rWEqDecls,
                     new Node(EQ,"",
                              new Node(ARR_ELEMENT,
                                       arrName,
                                       new Node(ARR_UPDATE,
                                                arrName,
                                                new Node(VAR, "a"),
                                                sIndexes,
                                                new Node(VAR, "t")),
                                       sIndexes->copy()
                                       ),
                              new Node(VAR, "t"),
                              elementType->copy()
                              ));

        rules->addChild(nRULE("arr " + intToString(indexTypes->arity())
                              + " RW eq (" + arrName + ")", rWEqAxiom));
        
        // RWNE axiom:
        //
        // All a:A, s1:S1, ... , sn:Sn, s1':S1, ... , sn':Sn, t:T.
        //   NOT (s1 =_S1 s1p /\ ... /\  sn = Sn snp)
        //  IMPLIES
        //   ARR_ELEMENT{A} (ARR_UPDATE{A}(a, (SEQ s1 ... sn), t)
        //                 (SEQ s1' ... sn')
        //  =_T ARR_ELEMENT{A} a (SEQ s1' ... sn')
        //
        // using OR

        Node* rWNEDecls = new Node(SEQ,
                                   new Node(DECL, "a", nameToType(arrName)));
        rWNEDecls->appendChildren(sDecls->copy());
        rWNEDecls->appendChildren(spDecls);
        rWNEDecls->addChild(new Node (DECL, "t", elementType->copy()));

        Node* rWNEAxiom =
            new Node(FORALL,
                     rWNEDecls,
                     new Node(OR,
                              eqIndexes,
                              new Node(EQ,"",
                                       new Node(ARR_ELEMENT,
                                                arrName,
                                                new Node(ARR_UPDATE,
                                                         arrName,
                                                         new Node(VAR, "a"),
                                                         sIndexes,
                                                         new Node(VAR, "t")),
                                                spIndexes
                                                ),
                                       new Node(ARR_ELEMENT,
                                                arrName,
                                                new Node(VAR, "a"),
                                                spIndexes->copy()
                                                ),
                                       elementType->copy()
                                       )
                              ));

        rules->addChild(nRULE("arr " + intToString(indexTypes->arity())
                              + " RW neq (" + arrName + ")", rWNEAxiom));

    } // END for loop over types in context

    return;
}

//-------------------------------------------------------------------------
// addArrayElementBoxUpdateAxioms
//-------------------------------------------------------------------------

// normT expected to be normalised type, 
// either ENUM_TY{type-id} k1 .. kn, TYPE_ID{type-id} or INT_TY.

// TYPE_ID{type-id} is assumed to be an abstract enumeration type.

Node* mkLE(Node* e1, Node* e2, Node* normT) {
    if (normT->kind == ENUM_TY || normT->kind == TYPE_ID)
        return new Node (FUN_AP, normT->id + "__LE", e1, e2);
    else if (normT->kind == INT_TY || normT->kind == SUBRANGE_TY)
        return new Node (I_LE, e1, e2);
    else {
        printMessage(ERRORm,
                     "Trying to make LE expression for type kind " +
                     kindString(normT->kind));
        return new Node(UNKNOWN);
    }
}

void
addArrayElementBoxUpdateAxioms(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);
    
    for (map<string,Node*>::iterator i = ctxt->typeMap.begin();
         i != ctxt->typeMap.end();
         i++ ) {

        Node* typeDecl = i->second;
        //   DEF_TYPE {id}  type 
        // | DEF_TYPE {id}
        if (typeDecl->arity() == 0) continue;

        Node* type = typeDecl->child(0);

        if (type->kind != ARRAY_TY) continue;

        // typeDecl is
        // DEF_TYPE {A} (ARRAY_TY{A} (SEQ S1 ... Sn) T)

        string arrName = typeDecl->id;
        Node* indexTypes = type->child(0);
        Node* elementType = type->child(1);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Add axioms
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        // Build iterated components of axioms.

        Node* sDecls = new Node(SEQ);   // s1:S1, ... , sn:Sn,
        Node* spDecls = new Node(SEQ);  // s1p:S1, ... , snp:Sn,
        Node* sqDecls = new Node(SEQ);  // s1q:S1, ... , snq:Sn,
        Node* sIndexes = new Node(SEQ); // s1, ..., sn
        Node* rIndexes = new Node(SEQ); // SUBRANGE(s1p,s1q), ..., 
                                        // SUBRANGE(snp,snq)

        Node* indexBoxPred = new Node(AND); // s1p <=_S1 s1 /\ s1 <=_S1 s1q
                                            //  /\ ... 
                                            // /\  snp <=_Sn sn /\ sn <=_Sn snq

        for (int i = 0; i != indexTypes->arity(); i++) {

            string s = "s"+ intToString(i);
            string sp = s + "p";
            string sq = s + "q";

            Node* indexType = indexTypes->child(i);
            Node* normIndexType = ctxt->normaliseType(indexType);
            
            sDecls->addChild(new Node (DECL, s, indexType->copy()));
            spDecls->addChild(new Node (DECL, sp, indexType->copy()));
            sqDecls->addChild(new Node (DECL, sq, indexType->copy()));
            sIndexes->addChild(new Node (VAR, s));
            rIndexes->addChild(new Node (SUBRANGE,
                                         new Node (VAR, sp),
                                         new Node (VAR, sq)));

            indexBoxPred->addChild(mkLE(new Node(VAR, sp),
                                          new Node(VAR, s),
                                        normIndexType));

            indexBoxPred->addChild(mkLE(new Node(VAR, s),
                                          new Node(VAR, sq),
                                        normIndexType));
            
        }

        // RInW axiom (Read inside of write box)
        //
        // All a:A, 
        //  s1:S1, ... , sn:Sn, 
        //  s1p:S1, ... , snp:Sn, 
        //  s1q:S1, ... , snq:Sn, 
        //  t:T.
        //   s1p <=_S1 s1 /\ s1 <=_S1 s1q
        //        /\ ... /\  snp <=_Sn sn /\ sn <=_Sn snq
        //  IMPLIES
        //   ARR_ELEMENT{A} (ARR_BOX_UPDATE{A}(a, (SEQ (r1 ... rn), t)
        //                 (SEQ s1 ... sn)
        //  =_T t
        //
        // where rj = SUBRANGE sjp sjq
        //
        // using OR


        Node* rInWDecls = new Node(SEQ,
                                   new Node(DECL, "a", nameToType(arrName)));
        rInWDecls->appendChildren(sDecls);
        rInWDecls->appendChildren(spDecls);
        rInWDecls->appendChildren(sqDecls);
        rInWDecls->addChild(new Node (DECL, "t", elementType->copy()));

        Node* rInWAxiom =
            new Node(FORALL,
                     rInWDecls,
                     new Node(IMPLIES,
                              indexBoxPred,
                              new Node(EQ,"",
                                       new Node(ARR_ELEMENT,
                                                arrName,
                                                new Node(ARR_BOX_UPDATE,
                                                         arrName,
                                                         new Node(VAR, "a"),
                                                         rIndexes,
                                                         new Node(VAR, "t")),
                                                sIndexes
                                                ),
                                       new Node(VAR, "t"),
                                       elementType->copy()
                                       )
                              )
                     );

        rules->addChild(nRULE("arr " + intToString(indexTypes->arity())
                              + " R in W box (" + arrName + ")", rInWAxiom));
        
        // ROutW axiom (Read outside of write box)
        //
        // All a:A, 
        //  s1:S1, ... , sn:Sn, 
        //  s1p:S1, ... , snp:Sn, 
        //  s1q:S1, ... , snq:Sn, 
        //  t:T.
        //   NOT (s1p <=_S1 s1 /\ s1 <=_S1 s1q
        //        /\ ... /\  snp <=_Sn sn /\ sn <=_Sn snq
        //  IMPLIES
        //   ARR_ELEMENT{A} (ARR_BOX_UPDATE{A}(a, (SEQ (r1 ... rn), t)
        //                 (SEQ s1 ... sn)
        //  =_T ARR_ELEMENT{A} a (SEQ s1 ... sn)
        //
        // where rj = SUBRANGE sjp sjq
        //
        // using OR

        Node* rOutWDecls = rInWDecls->copy();

        Node* rOutWAxiom =
            new Node(FORALL,
                     rOutWDecls,
                     new Node(OR,
                              indexBoxPred->copy(),
                              new Node(EQ,"",
                                       new Node(ARR_ELEMENT,
                                                arrName,
                                                new Node(ARR_BOX_UPDATE,
                                                         arrName,
                                                         new Node(VAR, "a"),
                                                         rIndexes->copy(),
                                                         new Node(VAR, "t")),
                                                sIndexes->copy()
                                                ),
                                       new Node(ARR_ELEMENT,
                                                arrName,
                                                new Node(VAR, "a"),
                                                sIndexes->copy()
                                                ),
                                       elementType->copy()
                                       )
                              ));

        rules->addChild(nRULE("arr " + intToString(indexTypes->arity())
                              + " R out of W box (" + arrName + ")",
                              rOutWAxiom));

    } // END for loop over types in context

    return;
}


//-------------------------------------------------------------------------
// addArrayExtensionalityAxioms
//-------------------------------------------------------------------------

void
addArrayExtensionalityAxioms(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);
    
    for (map<string,Node*>::iterator i = ctxt->typeMap.begin();
         i != ctxt->typeMap.end();
         i++ ) {

        Node* typeDecl = i->second;
        //   DEF_TYPE {id}  type 
        // | DEF_TYPE {id}
        if (typeDecl->arity() == 0) continue;

        Node* type = typeDecl->child(0);

        if (type->kind != ARRAY_TY) continue;

        // typeDecl is
        // DEF_TYPE {A} (ARRAY_TY{A} (SEQ S1 ... Sn) T)

        string arrName = typeDecl->id;
        Node* indexTypes = type->child(0);
        Node* elementType = type->child(1);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Add axioms
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        // Build iterated components of axioms.

        Node* sDecls = new Node(SEQ);    // s1:S1, ... , sn:Sn,

        Node* sIndexes = new Node(SEQ);  // s1, ..., sn



        for (int i = 0; i != indexTypes->arity(); i++) {

            string s = "s"+ intToString(i);
            string sp = s + "p";

            Node* indexType = indexTypes->child(i);
            
            sDecls->addChild(new Node (DECL, s, indexType->copy()));
            sIndexes->addChild(new Node (VAR, s));
        }
        
        // Extensionality axiom:
        // 
        // All a,a':A
        //   ( All  s1:S1, ... , sn:Sn. 
        //        ARR_ELEMENT{A} a (SEQ s1 ... sn)
        //        =_T
        //        ARR_ELEMENT{A} a' (SEQ s1 ... sn)
        //   )
        //   =>  a =_A a'

        Node* extAxiom =
            new Node
            (FORALL,
             new Node
             (SEQ,
              new Node(DECL, "a", nameToType(arrName)),
              new Node(DECL, "ap", nameToType(arrName))
              ),
             new Node(IMPLIES,
                      new Node(FORALL,
                               sDecls,
                               new Node(EQ,"",
                                        new Node(ARR_ELEMENT,
                                                 arrName,
                                                 new Node(VAR, "a"),
                                                 sIndexes
                                                 ),
                                        new Node(ARR_ELEMENT,
                                                 arrName,
                                                 new Node(VAR, "ap"),
                                                 sIndexes->copy()
                                                 ),
                                        elementType->copy()
                                        )
                               ),
                      new Node(EQ,"",
                               new Node(VAR,"a"),
                               new Node(VAR,"ap"),
                               nameToType(arrName))
                      )
             );
        rules->addChild(nRULE("arr " + intToString(indexTypes->arity())
                              + " ext (" + arrName + ")",
                              extAxiom));

    } // END for loop over types in context

    return;
}



//==========================================================================
// Abstract array select and update operators
//==========================================================================



//-------------------------------------------------------------------------
// abstractArrayOp
//-------------------------------------------------------------------------

Node* 
abstractArrayOp(Node* n) {
    if (n->kind == ARR_ELEMENT) {
        // ARR_ELEMENT{A} a (SEQ i1 ... in)
        // --> FUN_AP{<A>___arr_element} a i1 ... in

        n->kind = FUN_AP;
        n->id = n->id + "___arr_element";
        Node* indexes = n->child(1);
        n->popChild();
        n->appendChildren(indexes);
    }
    else if (n->kind == ARR_UPDATE) {
        // ARR_UPDATE{A} a (SEQ i1 ... in) v
        // --> FUN_AP{<A>___arr_update} a i1 ... in v

        n->kind = FUN_AP;
        n->id = n->id + "___arr_update";
        Node* indexes = n->child(1);
        Node* val = n->child(2);
        n->popChild();
        n->popChild();
        n->appendChildren(indexes);
        n->addChild(val);
    }
    return n;
}

//-------------------------------------------------------------------------
// abstractArraySelectUpdateOps
//-------------------------------------------------------------------------

void
abstractArraySelectUpdateOps(FDLContext* ctxt, Node* unit) {


    
    for (map<string,Node*>::iterator i = ctxt->typeMap.begin();
         i != ctxt->typeMap.end();
         i++ ) {

        Node* typeDecl = i->second;
        //   DEF_TYPE {id}  type 
        // | DEF_TYPE {id}
        if (typeDecl->arity() == 0) continue;

        Node* type = typeDecl->child(0);

        if (type->kind != ARRAY_TY) continue;

        // typeDecl is
        // DEF_TYPE {A} (ARRAY_TY{A} (SEQ S1 ... Sn) T)

        string arrName = typeDecl->id;
        Node* indexTypes = type->child(0);
        Node* elementType = type->child(1);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Add declarations for element and update functions
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


        // DECL_FUN{<A>___arr_element} (SEQ A S1 ... Sn) T

        Node* argTypes = new Node(SEQ, nameToType(arrName));
        argTypes->appendChildren(indexTypes->copy());
        
        Node* elementDecl = new Node(DECL_FUN,
                                     arrName + "___arr_element",
                                     argTypes,
                                     elementType->copy()
                                     );
        ctxt->insert(elementDecl);
             
        // DECL_FUN{<A>___arr_update} (A S1 ... Sn T) A

        argTypes = argTypes->copy();
        argTypes->addChild(elementType->copy());
        
        Node* updateDecl = new Node(DECL_FUN,
                                    arrName + "___arr_update",
                                    argTypes,
                                    nameToType(arrName)
                                     );
        ctxt->insert(updateDecl);
        


        
    }  // END of for loop over type decls

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Abstract array operators in unit 
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Done after axiom addition, since axioms use array operators
    
    unit->mapOver(abstractArrayOp);



    return;
}

//-------------------------------------------------------------------------
// abstractArrayBoxUpdate
//-------------------------------------------------------------------------

Node* 
abstractArrayBoxUpdate(Node* n) {
    if (n->kind == ARR_BOX_UPDATE) {
        // ARR_BOX_UPDATE{A} a (SEQ r1 ... rn) v   where  rj = SUBRANGE ijp ijq
        // --> FUN_AP{<A>___arr_box_update} a i1p i1q ... inp inq v

        n->kind = FUN_AP;
        n->id = n->id + "___arr_box_update";
        Node* ranges = n->child(1);
        Node* val = n->child(2);
        n->popChild();
        n->popChild();
        for (int j = 0; j != ranges->arity(); j++) {
            n->appendChildren(ranges->child(j));
        }
        n->addChild(val);
    }
    return n;
}

//-------------------------------------------------------------------------
// abstractArrayBoxUpdates
//-------------------------------------------------------------------------

void
abstractArrayBoxUpdates(FDLContext* ctxt, Node* unit) {


    
    for (map<string,Node*>::iterator i = ctxt->typeMap.begin();
         i != ctxt->typeMap.end();
         i++ ) {

        Node* typeDecl = i->second;
        //   DEF_TYPE {id}  type 
        // | DEF_TYPE {id}
        if (typeDecl->arity() == 0) continue;

        Node* type = typeDecl->child(0);

        if (type->kind != ARRAY_TY) continue;

        // typeDecl is
        // DEF_TYPE {A} (ARRAY_TY{A} (SEQ S1 ... Sn) T)

        string arrName = typeDecl->id;
        Node* indexTypes = type->child(0);
        Node* elementType = type->child(1);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Add declaration for box update function
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

        // DECL_FUN{<A>___arr_box_update} (A S1 S1 ... Sn Sn T) A


        Node* argTypes = new Node(SEQ, nameToType(arrName));
        for (int j = 0; j != indexTypes->arity(); j++) {
            argTypes->addChild(indexTypes->child(j)->copy());
            argTypes->addChild(indexTypes->child(j)->copy());
        }
        argTypes->addChild(elementType->copy());

        Node* boxUpdateDecl = new Node(DECL_FUN,
                                       arrName + "___arr_box_update",
                                       argTypes,
                                       nameToType(arrName)
                                       );
        ctxt->insert(boxUpdateDecl);

        
    }  // END of for loop over type decls

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Abstract array operators in unit 
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Done after axiom addition, since axioms use array operators
    
    unit->mapOver(abstractArrayBoxUpdate);

    return;
}




//========================================================================
// Array extensionality treatment
//========================================================================


/*


We introduce a user-defined relation to alias equality over arrays to
help along the typical quantifier instantiation algorithms which will
not match against raw equalities.  (At least this seems to be the case with
Z3, indeed a remark in the Z3 FAQ talks about this).

Keep option just to add an extensionality axiom as above.  But also
allow for the following:


For each array type A = ARRAY(S1, ... Sn) T



1. Add strong extensionality axiom for relation    
All a,a':A
  ( All  s1:S1, ... , sn:Sn. 
     ARR_ELEMENT{A} a (SEQ s1 ... sn)
       =_T
     ARR_ELEMENT{A} a' (SEQ s1 ... sn)
   )
   =>  <A>___ext_eq(a,a') 

  Does it matter whether we use <=> or =>?

  - (For <=>): It might get converted into a boolean equality.  This might
    be handled more efficiently. 
       But then again, have predicate logic structure on one side, so 
    need anyway to drop down to logic reasoning.

  - (For =>): More spartan.  Not stating a redundant fact.

  Go for => for now.  (Allow <=> as option?)


 [Done above in addArrayAxioms() function]

2a Everywhere replace   a =_A a' with <A>___ext_eq(a,a') 

OR

2b Only do replacement in contexts where equality occurs +vely.
   (e.g. in concl under even number of negations)


3. Declare equality relation:

   <A>___ext_eq : (A,A) Bool

   
4. Add defining axiom for relation

  All a,a':A.  <A>___ext_eq(a,a')  <=> a =_A a'


Ordering constraints:

* Important that 4 is after 2a!

* 2a is simplest to do by local map if the array type has not first
  been abstracted.  So need to also fix order of standard abstraction. 

* Do we do 2a *after* introducing array read over write axioms?
  Seems like yes, so those axioms can be also updated as necessary (if for
  example we have nested arrays and records).



This strategy is realised together for both arrays and records below.

*/







//========================================================================
//========================================================================
// Abstracting interpreted records
//========================================================================
//========================================================================
//
// THIS NEEDS UPDATING!!!!
//
//
// 1. Eliminate constructor ops using updates 
//    Add abstract default record constant  [DONE]
//
// 2. Axiomatise and abstract update ops
//
// 3. Axiomatise type with select and update ops
//    Abstract type, select and update.
//    [PRECOND: 1 done]

// 4. Axiomatise type with select and constructor ops
//    Abstract type, select and constructor
//    [PRECOND: 2 done]
//
//  Common functionality:
//
//  A: Adding declarations
//  B: mapping over unit, updating it
//
//  1 already written.
//
//  Can we save effort by building multipurpose utilities for 2,3,4?
//
//  B: 
//
//  Notation:
//
//  Type:
//    R = RECORD_TY{R}(D1 ... Dn) where   Di = DECL{fi} Ti
//
//  Operators:
//    MK_RECORD{R} A1 ... An   where Ai = ASSIGN{fi} ti
//    RCD_ELEMENT{fi} r TYPE_PARAM{R}
//    RCD_UPDATE{fi} r t TYPE_PARAM{R}

//========================================================================
// Utilities for record abstraction and elimination
//========================================================================

// 
//   Assuming assignments in proper order.  Should check in each instance
//
// For i = 1 .. n
//   RCD_ELEMENT{fi} r TYPE_PARAM{R} --> FUN_AP{<R>___<fi>___rcd_element r} r
//   RCD_UPDATE{fi} r ti TYPE_PARAM{R} 
//   --> FUN_AP{<R>___<fi>___rcd_update r} r ti
//
// Declarations for abstract operators:
//
// For i = 1 .. n
//     FUN_DECL{<R>___<fi>___rcd_element} (SEQ R) Ti
//     FUN_DECL{<R>___<fi>___rcd_update} (SEQ R Ti) R
// FUN_DECL{<R>___mk_rcd} (SEQ T1 ... Tn) R
//

//----------------------------------------------------------------------------
// abstractMkRecord
//----------------------------------------------------------------------------
// MK_RECORD{R} A1 ... An  --> FUN_AP{<R>___mk_rcd} t1 ... tn 
//  with check that assignments in expected order.

Node* abstractMkRecord(FDLContext* c, Node* n) {

    if (n->kind != MK_RECORD) return n;

    string rcdName = n->id;

    // rcdType := RECORD_TY{R}(D1 ... Dn) where   Di = DECL{fi} Ti
    Node* rcdType = c->lookupType(rcdName)->child(0);

    if (n->arity() != rcdType->arity()) {
        printMessage(ERRORm,
                     "Constructor for record type " + rcdName
                     + " found with bad arity " + intToString(n->arity()));
        return n;
    }


    for (int i = 0; i != rcdType->arity(); i++) {

        if (n->child(i)->id != rcdType->child(i)->id) {
            printMessage(ERRORm,
                         "Constructor for record type " + rcdName
                         + " found with field " + n->child(i)->id
                         + " out of order");
            return n;
        }
        n->child(i) = n->child(i)->child(0);
    }

    n->kind = FUN_AP;
    n->id = rcdName + "___mk_rcd";

    return n;
}

//----------------------------------------------------------------------------
// abstractRcdElement
//----------------------------------------------------------------------------

//   RCD_ELEMENT{fi} r TYPE_PARAM{R} --> FUN_AP{<R>___<fi>___rcd_element r} r

Node* abstractRcdElement(Node* n) {

    if (n->kind != RCD_ELEMENT) return n;

    string rcdName = n->child(1)->id;
    string fieldName = n->id;
    
    n->popChild();
    n->kind = FUN_AP;
    n->id = rcdName + "___" + fieldName + "___rcd_element";

    return n;
}

//----------------------------------------------------------------------------
// abstractRcdUpdate
//----------------------------------------------------------------------------

// RCD_UPDATE{fi} r ti TYPE_PARAM{R} 
//   --> 
// FUN_AP{<R>___<fi>___rcd_update r} r ti

Node* abstractRcdUpdate(Node* n) {

    if (n->kind != RCD_UPDATE) return n;

    string rcdName = n->child(2)->id;
    string fieldName = n->id;
    
    n->popChild();
    n->kind = FUN_AP;
    n->id = rcdName + "___" + fieldName + "___rcd_update";

    return n;
}


//----------------------------------------------------------------------------
// abstractRcdUpdates
//----------------------------------------------------------------------------
// For record type R = RECORD_TY(D1 ... Dn) where  Di = DECL{fi} Ti
//  
// Add n axioms, where declaration i is
// and add a declaration:
//
//   FUN_DECL{<R>___<fi>___rcd_update} (SEQ R Ti) R
//
// and axiom i is 
//  
//  FORALL r:R, t:Ti
//  
//    RCD_UPDATE{fi} r t TYPE_PARAM{R}
//    =_R
//    MK_RECORD{R}
//      (ASSIGN{f1}      (RCD_ELEMENT{f1}     r TYPE_PARAM{R}) )
//      (ASSIGN{f(i-1)}  (RCD_ELEMENT{f(i-1)} r TYPE_PARAM{R}) )
//      (ASSIGN{fi}      t
//      (ASSIGN{f(i+1)}  (RCD_ELEMENT{f(i+1)} r TYPE_PARAM{R}) )
//      (ASSIGN{fn}      (RCD_ELEMENT{fn}     r TYPE_PARAM{R}) )
//  
//  
//  Then abstract the RCD_UPDATE expression: 
//  
//  RCD_UPDATE{fi} r t TYPE_PARAM{R}  --> <R>___<fi>___rcd_update
// 
//

void abstractRcdUpdates(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);
    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Iterate over each record type declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int j = 0; j != ctxt->typeSeq.arity(); j++) {

        Node* typeDecl = ctxt->typeSeq.child(j);

        if (!( typeDecl->kind == DEF_TYPE
               && typeDecl->arity() == 1
               && typeDecl->child(0)->kind == RECORD_TY)) {
            continue;
        }

        // rcdType = RECORD_TY{R}(D1 ... Dn) where  Di = DECL{fi} Ti
        Node* rcdType = typeDecl->child(0);
        string typeName = rcdType->id;




        for (int i = 0; i != rcdType->arity(); i++) {
            string fieldName_i = rcdType->child(i)->id;
            Node* fieldType_i = rcdType->child(i)->child(0);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Add declaration
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            //   DECL_FUN{<R>___<fi>___rcd_update} (SEQ R Ti) R

            Node* decl = new Node(DECL_FUN,
                                  typeName + "___"
                                  + fieldName_i + "___rcd_update",
                                  new Node(SEQ,
                                           nameToType(typeName),
                                           fieldType_i),
                                  nameToType(typeName)
                                  );
            ctxt->insert(decl);
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Add axiom         
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            
            Node* mkRecordExp = new Node(MK_RECORD, typeName);
            
            for (int k = 0; k != rcdType->arity(); k++) {
                string fieldName_k = rcdType->child(k)->id;

                Node* assignVal =
                    (k == i)
                    ? new Node(VAR, "t")
                    : new Node(RCD_ELEMENT,
                               fieldName_k,
                               new Node(VAR, "r"),
                               new Node(TYPE_PARAM, typeName));
                
                mkRecordExp->addChild(new Node(ASSIGN,fieldName_k,assignVal));
            }


            Node* updateElimAxiom =
                new Node(FORALL,
                         new Node(SEQ,
                                  new Node(DECL, "r", nameToType(typeName)),
                                  new Node(DECL, "t", fieldType_i->copy())
                                  ),
                         new Node (EQ,"",
                                   new Node(RCD_UPDATE,
                                            fieldName_i,
                                            new Node(VAR, "r"),
                                            new Node(VAR, "t"),
                                            new Node(TYPE_PARAM, typeName)
                                            ),
                                   mkRecordExp,
                                   nameToType(typeName)
                                   )
                         );

            rules->addChild(nRULE("upd elim rcd ax (" 
                                  + fieldName_i + ","
                                  + typeName + ")",
                                  updateElimAxiom));

        } // END For i over record components

    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Abstract record update operators
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    unit->mapOver1(abstractRcdUpdate);
    return;
}

//----------------------------------------------------------------------------
// addRcdElementUpdateAxioms
//----------------------------------------------------------------------------
//
// For all i = 1 .. n
//   For all k = 1 .. n
//     Add axiom:
//
//     All r:R.  All t:Tk
//        RCD_ELEMENT{fi} (RCD_UPDATE{fk} r t R) R =_Ti  t      if  i = k
//        RCD_ELEMENT{fi} (RCD_UPDATE{fk} r t R) R
//                         =_Ti  RCD_ELEMENT{fi} r R            if  i != k
// 


void addRcdElementUpdateAxioms(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);
    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Iterate over each record type declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int j = 0; j != ctxt->typeSeq.arity(); j++) {

        Node* typeDecl = ctxt->typeSeq.child(j);

        if (!( typeDecl->kind == DEF_TYPE
               && typeDecl->arity() == 1
               && typeDecl->child(0)->kind == RECORD_TY)) {
            continue;
        }

        // rcdType = RECORD_TY{R}(D1 ... Dn) where  Di = DECL{fi} Ti
        Node* rcdType = typeDecl->child(0);
        string typeName = rcdType->id;

        int rcdSize = rcdType->arity();
        
        printMessage(INFOm,
                     "Generating " + intToString(rcdSize * rcdSize)
                     + " element-update axioms for record type " + typeName
                     + " with " + intToString(rcdSize) + " fields");
        

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Loop i over fields
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        for (int i = 0; i != rcdType->arity(); i++) {
            string fieldName_i = rcdType->child(i)->id;
            Node* fieldType_i = rcdType->child(i)->child(0);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Loop k over fields
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            
            for (int k = 0; k != rcdType->arity(); k++) {
                string fieldName_k = rcdType->child(k)->id;
                Node* fieldType_k = rcdType->child(k)->child(0);

                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
                // Add element update axiom
                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                Node* lhs =
                    new Node(RCD_ELEMENT,
                             fieldName_i,
                             new Node(RCD_UPDATE,
                                      fieldName_k,
                                      new Node(VAR, "r"),
                                      new Node(VAR, "t"),
                                      new Node(TYPE_PARAM, typeName)
                                      ),
                             new Node(TYPE_PARAM, typeName)
                             );

                Node* rhs =
                    (i == k)
                    ? new Node(VAR, "t")
                    : new Node(RCD_ELEMENT,
                               fieldName_i,
                               new Node(VAR, "r"),
                               new Node(TYPE_PARAM, typeName)
                               );

                Node* elementUpdateAxiom = 
                    new Node(FORALL,
                             new Node(SEQ,
                                      new Node(DECL,
                                               "r",
                                               nameToType(typeName)),
                                      new Node(DECL,
                                               "t",
                                               fieldType_k->copy())
                                      ),
                             new Node(EQ,"",
                                      lhs,
                                      rhs,
                                      fieldType_i->copy())
                             );


                rules->addChild(nRULE("rcd elt upd ("
                                      + fieldName_i + ","
                                      + fieldName_k + ","
                                      + typeName + ")",
                                      elementUpdateAxiom));

            } // END For k over fields

        } // END For i over fields


        
    } // END For j over type definitions
    
    return;
}

//----------------------------------------------------------------------------
// abstractRcdElementsUpdates
//----------------------------------------------------------------------------
// Assumes MK_RECORD operators already eliminated
//
//
// For all i = 1 .. n
//   add declarations:
//
//     DECL_FUN{<R>___<fi>___rcd_element} (SEQ R) Ti
//     DECL_FUN{<R>___<fi>___rcd_update} (SEQ R Ti) R
//
// 
// Abstract all occurrences of element and update operators


void abstractRcdElementsUpdates(FDLContext* ctxt, Node* unit) {

    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Iterate over each record type declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int j = 0; j != ctxt->typeSeq.arity(); j++) {

        Node* typeDecl = ctxt->typeSeq.child(j);

        if (!( typeDecl->kind == DEF_TYPE
               && typeDecl->arity() == 1
               && typeDecl->child(0)->kind == RECORD_TY)) {
            continue;
        }

        // rcdType = RECORD_TY{R}(D1 ... Dn) where  Di = DECL{fi} Ti
        Node* rcdType = typeDecl->child(0);
        string typeName = rcdType->id;


        

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Loop i over fields
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        for (int i = 0; i != rcdType->arity(); i++) {
            string fieldName_i = rcdType->child(i)->id;
            Node* fieldType_i = rcdType->child(i)->child(0);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Add declarations
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            //   DECL_FUN{<R>___<fi>___rcd_element} (SEQ R) Ti
            //   DECL_FUN{<R>___<fi>___rcd_update} (SEQ R Ti) R

            Node* declE = new Node(DECL_FUN,
                                   typeName + "___"
                                   + fieldName_i + "___rcd_element",
                                   new Node(SEQ,
                                            nameToType(typeName)
                                            ),
                                   fieldType_i
                                   );
            ctxt->insert(declE);
            Node* declU = new Node(DECL_FUN,
                                   typeName + "___"
                                   + fieldName_i + "___rcd_update",
                                   new Node(SEQ,
                                            nameToType(typeName),
                                            fieldType_i
                                            ),
                                   nameToType(typeName)
                                   );
            ctxt->insert(declU);


        } // END For i over fields
        
    } // END For j over type definitions
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Abstract record element and update operators
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    unit->mapOver1(abstractRcdElement);
    unit->mapOver1(abstractRcdUpdate);
    return;
}



//----------------------------------------------------------------------------
// addRcdEqElementsExtAxioms
//----------------------------------------------------------------------------
//
// For each record type
//
// Add axiom 
//      All r,r': R.
//        (RCD_ELEMENT{f1} r R =_T1 RCD_ELEMENT{f1} r' R
//         /\ ... /\ ...
//         RCD_ELEMENT{fn} r R =_Tn RCD_ELEMENT{fn} r' R
//        )
//        IMPLIES  r =_R r'
// 

void addRcdEqElementsExtAxioms(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);
    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Iterate over each record type declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int j = 0; j != ctxt->typeSeq.arity(); j++) {

        Node* typeDecl = ctxt->typeSeq.child(j);

        if (!( typeDecl->kind == DEF_TYPE
               && typeDecl->arity() == 1
               && typeDecl->child(0)->kind == RECORD_TY)) {
            continue;
        }

        // rcdType = RECORD_TY{R}(D1 ... Dn) where  Di = DECL{fi} Ti
        Node* rcdType = typeDecl->child(0);
        string typeName = rcdType->id;


        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Initialise core part of extensionality axiom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        Node* extAxiomHyps = new Node(AND);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Loop i over fields
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        for (int i = 0; i != rcdType->arity(); i++) {
            string fieldName_i = rcdType->child(i)->id;
            Node* fieldType_i = rcdType->child(i)->child(0);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Augment core of extensionality axiom
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            Node* elementEq =
                new Node(EQ,"",
                         new Node(RCD_ELEMENT,
                                  fieldName_i,
                                  new Node(VAR, "r"),
                                  new Node(TYPE_PARAM, typeName)
                                  ),
                         new Node(RCD_ELEMENT,
                                  fieldName_i,
                                  new Node(VAR, "rp"),
                                  new Node(TYPE_PARAM, typeName)
                                  ),
                         fieldType_i->copy()
                         );

            extAxiomHyps->addChild(elementEq);

        } // END For i over fields

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Add extensionality axiom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


        Node* extAxiom =
            new Node(FORALL,
                     new Node(SEQ,
                              new Node (DECL, "r", nameToType(typeName)),
                              new Node (DECL, "rp", nameToType(typeName))
                              ),
                     mkGeneralImplies(
                                      extAxiomHyps,
                                      new Node(EQ,"",
                                               new Node (VAR,"r"),
                                               new Node (VAR,"rp"),
                                               nameToType(typeName)
                                               )
                                      )
                     );

        rules->addChild(nRULE("rcd eq elts ext (" + typeName + ")",
                              extAxiom));

        
    } // END For j over type definitions
    
    return;
}




//----------------------------------------------------------------------------
//  addRcdElementMkRcdAxioms
//----------------------------------------------------------------------------
//  
// For i = 1 .. n 
// 
//   add axiom
//
//     All t1:T1, ..., tn:Tn.
//        RCD_ELEMENT{fi} (MK_RECORD{R} ASSIGN{f1} t1 ... ASSIGN{fn} tn) R
//        =_Ti
//        ti

void addRcdElementMkRcdAxioms(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);
    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Iterate over each record type declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int j = 0; j != ctxt->typeSeq.arity(); j++) {

        Node* typeDecl = ctxt->typeSeq.child(j);

        if (!( typeDecl->kind == DEF_TYPE
               && typeDecl->arity() == 1
               && typeDecl->child(0)->kind == RECORD_TY)) {
            continue;
        }

        // rcdType = RECORD_TY{R}(D1 ... Dn) where  Di = DECL{fi} Ti
        Node* rcdType = typeDecl->child(0);
        string typeName = rcdType->id;

        for (int i = 0; i != rcdType->arity(); i++) {
            string fieldName_i = rcdType->child(i)->id;
            Node* fieldType_i = rcdType->child(i)->child(0);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Add element constructor axiom         
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            
            Node* axiomMkRcd = new Node(MK_RECORD, typeName);
            Node* axiomDecls = new Node(SEQ);
            
            for (int k = 0; k != rcdType->arity(); k++) {
                string fieldName_k = rcdType->child(k)->id;
                Node* fieldType_k = rcdType->child(k)->child(0);

                string var_k = "t" + intToString(k);

                axiomDecls->addChild(new Node(DECL,
                                              var_k,
                                              fieldType_k->copy()
                                              )
                                     );

                axiomMkRcd->addChild(new Node(ASSIGN,
                                              fieldName_k,
                                              new Node(VAR, var_k)
                                              )
                                     );
            }
            Node* elementConstructorAxiom =
                new Node(FORALL,
                         axiomDecls,
                         new Node (EQ,"",
                                   new Node(RCD_ELEMENT,
                                            fieldName_i,
                                            axiomMkRcd,
                                            nameToType(typeName)
                                            ),
                                   new Node(VAR, "t" + intToString(i)),
                                   fieldType_i->copy()
                                   )
                         );

            rules->addChild(nRULE("rcd elt mk ("
                                  + fieldName_i + ","
                                  + typeName + ")",
                                  elementConstructorAxiom));

        } // END For i over record components

    } // END For j over type defs
    return;
}


//----------------------------------------------------------------------------
// abstractRcdElementsMkRcds
//----------------------------------------------------------------------------
//
// Add declaration
//   DECL_FUN{<R>___mk_rcd} (SEQ T1 ... Tn) R
//
//  
// For i = 1 .. n 
// 
//   add declaration
//
//     DECL_FUN{<R>___<fi>___rcd_element} (SEQ R) Ti
//
// Abstract all occurrences of the element and mk_rcd operators

void abstractRcdElementsMkRcds(FDLContext* ctxt, Node* unit) {

    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Iterate over each record type declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int j = 0; j != ctxt->typeSeq.arity(); j++) {

        Node* typeDecl = ctxt->typeSeq.child(j);

        if (!( typeDecl->kind == DEF_TYPE
               && typeDecl->arity() == 1
               && typeDecl->child(0)->kind == RECORD_TY)) {
            continue;
        }

        // rcdType = RECORD_TY{R}(D1 ... Dn) where  Di = DECL{fi} Ti
        Node* rcdType = typeDecl->child(0);
        string typeName = rcdType->id;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Initialise argTypes for constructor declaration
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        Node* mkRcdArgTypes = new Node(SEQ);
                                     
        for (int i = 0; i != rcdType->arity(); i++) {
            string fieldName_i = rcdType->child(i)->id;
            Node* fieldType_i = rcdType->child(i)->child(0);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Add declaration for element operator
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            //   DECL_FUN{<R>___<fi>___rcd_element} (SEQ R) Ti

            Node* decl = new Node(DECL_FUN,
                                  typeName + "___"
                                  + fieldName_i + "___rcd_element",
                                  new Node(SEQ, nameToType(typeName)),
                                  fieldType_i
                                  );
            ctxt->insert(decl);
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Augment argTypes for constructor declaration
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            mkRcdArgTypes->addChild(fieldType_i->copy());

        } // END For i over record components

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Add declaration for record constructor
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        Node* mkRcdDecl = new Node(DECL_FUN,
                                   typeName + "___mk_rcd",
                                   mkRcdArgTypes,
                                   nameToType(typeName)
                                   );

        ctxt->insert(mkRcdDecl);
        
    } // END For j over type defs


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Abstract record element and constructor operators
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    unit->mapOver1(abstractRcdElement);
    mapOverWithContext(abstractMkRecord, ctxt, unit);
    
    return;
}

//----------------------------------------------------------------------------
// addMkRcdElementExtAxioms
//----------------------------------------------------------------------------
// For each record type
//
// Add extensionality axiom
//
// All r:R. 
//   MK_RECORD{R}(
//      ASSIGN{f1} (RCD_ELEMENT{f1} r R),
//      ...
//      ASSIGN{fn} (RCD_ELEMENT{fn} r R)
//   )
//   =_R r
//  

void addMkRcdElementExtAxioms(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);
    // - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Iterate over each record type declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - -

    for (int j = 0; j != ctxt->typeSeq.arity(); j++) {

        Node* typeDecl = ctxt->typeSeq.child(j);

        if (!( typeDecl->kind == DEF_TYPE
               && typeDecl->arity() == 1
               && typeDecl->child(0)->kind == RECORD_TY)) {
            continue;
        }

        // rcdType = RECORD_TY{R}(D1 ... Dn) where  Di = DECL{fi} Ti
        Node* rcdType = typeDecl->child(0);
        string typeName = rcdType->id;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Initialise ext axiom core
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

        Node* extAxiomLHS = new Node(MK_RECORD, typeName);
                                     
        for (int i = 0; i != rcdType->arity(); i++) {
            string fieldName_i = rcdType->child(i)->id;
            // Node* fieldType_i = rcdType->child(i)->child(0);

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Augment extensionality axiom core
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            extAxiomLHS->addChild(new Node(ASSIGN,
                                           fieldName_i,
                                           new Node(RCD_ELEMENT,
                                                    fieldName_i,
                                                    new Node(VAR, "r"),
                                                    nameToType(typeName)
                                                    )
                                           )
                                  );

        } // END For i over record components

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Add extensionality axiom 
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        Node* extAxiom = new Node(FORALL,
                                  new Node(SEQ, new Node(DECL,
                                                         "r",
                                                         nameToType(typeName))
                                           ),
                                  new Node(EQ,"",
                                           extAxiomLHS,
                                           new Node (VAR,"r"),
                                           nameToType(typeName)
                                           )
                                  );

        rules->addChild(nRULE("rcd mk elts ext (" + typeName + ")",
                              extAxiom));

    } // END For j over type defs

    return;
}



//========================================================================
// Abstracting arrays and records: functions common to both
//========================================================================

//------------------------------------------------------------------------
// Introduce aliases for array and record equalities
//------------------------------------------------------------------------

Node* introArrayEqAlias(FDLContext* ctxt, Node* n) {

    if (n->kind != EQ) return n;

    // n->kind == EQ

    Node* eqType = ctxt->normaliseType(n->child(2));

    if (eqType->kind == ARRAY_TY) {

        return mkEqAliasRel(eqType->id, n->child(0), n->child(1));
    }
    else {
        return n;
    }
}
//------------------------------------------------------------------------
// Introduce aliases for array and record equalities
//------------------------------------------------------------------------

Node* introRecordEqAlias(FDLContext* ctxt, Node* n) {

    if (n->kind != EQ) return n;

    // n->kind == EQ

    Node* eqType = ctxt->normaliseType(n->child(2));

    if (eqType->kind == RECORD_TY) {

        return mkEqAliasRel(eqType->id, n->child(0), n->child(1));
    }
    else {
        return n;
    }
}

//---------------------------------------------------------------------------
// Add declarations and defining axioms for array and record equality aliases
//---------------------------------------------------------------------------

void
addDeclsAxiomsForArrRecEqAliases(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);
    
    for (map<string,Node*>::iterator i = ctxt->typeMap.begin();
         i != ctxt->typeMap.end();
         i++ ) {

        Node* typeDecl = i->second;
        //   DEF_TYPE {id}  type 
        // | DEF_TYPE {id}
        if (typeDecl->arity() == 0) continue;

        Node* type = typeDecl->child(0);

        if (! ( (type->kind == ARRAY_TY
                 && option("use-array-eq-aliases"))
                ||
                (type->kind == RECORD_TY
                 && option("use-record-eq-aliases"))
                )
            )
            continue;

        // typeDecl is
        // DEF_TYPE {A} (ARRAY_TY{A} (SEQ S1 ... Sn) T)
        // DEF_TYPE {A} (RECORD_TY{R} D1 ... Dn where Di = DECL{fi} Ti

        string typeName = typeDecl->id;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Declare eq alias function/relation
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        Node* eqAliasDecl = new Node(DECL_FUN,
                                     typeName + "___eq",
                                     new Node(SEQ,
                                              nameToType(typeName),
                                              nameToType(typeName)),
                                     Node::bool_ty
                                     );
        ctxt->insert(eqAliasDecl);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Add eq alias defining axiom
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


        Node* axiom =
            new Node(FORALL,
                     new Node (SEQ,
                               new Node (DECL,
                                         "x",
                                         nameToType(typeName)),
                               new Node (DECL,
                                         "y",
                                         nameToType(typeName))
                               ),
                     new Node(IFF,
                              mkEqAliasRel(typeName,
                                           new Node(VAR, "x"),
                                           new Node(VAR, "y")),
                              new Node(EQ,"",
                                       new Node(VAR, "x"),
                                       new Node(VAR, "y"),
                                       nameToType(typeName))
                              )
                     );

        string tKind(type->kind == ARRAY_TY ? "arr" : "rcd");

        rules->addChild(nRULE(tKind + "eq alias (" + typeName + ")",
                              axiom));

    } // END for loop

    return;
}
        

//------------------------------------------------------------------------
// abstractArrayTypeDecls
//------------------------------------------------------------------------

void abstractArrayTypeDecls(FDLContext* ctxt) {

    for (map<string,Node*>::iterator i = ctxt->typeMap.begin();
         i != ctxt->typeMap.end();
         i++ ) {

        Node* typeDecl = i->second;
        //   DEF_TYPE {id}  type 
        // | DEF_TYPE {id}

        if (typeDecl->arity() != 0 && typeDecl->child(0)->kind == ARRAY_TY) {
            typeDecl->popChild();
        }
    }
}

//------------------------------------------------------------------------
// abstractRecordTypeDecls
//------------------------------------------------------------------------

void abstractRecordTypeDecls(FDLContext* ctxt) {

    for (map<string,Node*>::iterator i = ctxt->typeMap.begin();
         i != ctxt->typeMap.end();
         i++ ) {

        Node* typeDecl = i->second;
        //   DEF_TYPE {id}  type 
        // | DEF_TYPE {id}

        if (typeDecl->arity() != 0 && typeDecl->child(0)->kind == RECORD_TY) {
            typeDecl->popChild();
        }
    }
}



    

//==========================================================================
// Array and record abtraction master function
//==========================================================================

void abstractArraysRecords(FDLContext* ctxt, Node* unit) {

    //-----------------------------------------------------------------------
    // Remove redundant array and record operators
    //-----------------------------------------------------------------------

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Eliminate array constructors.
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // And add decls for default array and const array constructors.
    // and add axiom for const array constructor.

    if (option("elim-array-constructors")) elimMkArrays(ctxt, unit);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Eliminate record constructors
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // And add default array decls

    if (option("elim-record-constructors")) elimMkRecords(ctxt, unit);


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Abstract record updates
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (option("abstract-record-updates")) abstractRcdUpdates(ctxt, unit);

    //-----------------------------------------------------------------------
    // Add array axioms
    //-----------------------------------------------------------------------
    // Axioms for axiomatically defining type and axioms for box updates
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add array select update axioms
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (option("add-array-select-update-axioms")) 
        addArrayElementUpdateAxioms(ctxt, unit);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add array extensionality axioms
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (option("add-array-extensionality-axioms"))
        addArrayExtensionalityAxioms(ctxt, unit);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add array box update axioms
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (option("add-array-select-box-update-axioms")) 
        addArrayElementBoxUpdateAxioms(ctxt, unit);


    //-----------------------------------------------------------------------
    // Add record axioms
    //-----------------------------------------------------------------------

    if (option("add-record-select-constructor-axioms")) {
        addRcdElementMkRcdAxioms(ctxt, unit);
    }
    if (option("add-record-constructor-extensionality-axioms")) {
        addMkRcdElementExtAxioms(ctxt, unit);
    }
    if (option("add-record-select-update-axioms")) {
        addRcdElementUpdateAxioms(ctxt, unit);
    }
    if (option("add-record-eq-elements-extensionality-axioms")) {
        addRcdEqElementsExtAxioms(ctxt, unit);
    }

    //-----------------------------------------------------------------------
    // Introduce aliases for array and record equalities
    //-----------------------------------------------------------------------

    if (option("use-array-eq-aliases")) {
        mapOverWithContext(introArrayEqAlias, ctxt, unit);
    }
    if (option("use-record-eq-aliases")) {
        mapOverWithContext(introRecordEqAlias, ctxt, unit);
    }

    if (option("use-array-eq-aliases") || option("use-record-eq-aliases")) {
        addDeclsAxiomsForArrRecEqAliases(ctxt, unit);
    }

    //--------------------------------------------------------------------
    // Abstract array operators and types
    //--------------------------------------------------------------------

    if (option("abstract-array-select-updates")) {
        abstractArraySelectUpdateOps(ctxt, unit);
    }
    if (option("abstract-array-box-updates")) {
        abstractArrayBoxUpdates(ctxt, unit);
    }

    if (option("abstract-array-types")) abstractArrayTypeDecls(ctxt);

    //--------------------------------------------------------------------
    // Abstract record operator and types
    //--------------------------------------------------------------------

    if (option("abstract-record-selects-constructors")) {
        abstractRcdElementsMkRcds(ctxt, unit);
    }

    if (option("abstract-record-selects-updates")) {
        abstractRcdElementsUpdates(ctxt, unit);
    }
    if (option("abstract-record-types")) abstractRecordTypeDecls(ctxt);

}



