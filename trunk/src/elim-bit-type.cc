//========================================================================
//========================================================================
// ELIM-BIT-TYPE.CC
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
// Eliminating/abstracting bit type, ops, consts and rels.
//========================================================================
//========================================================================

// Translations in groups.  Ordering of groups should be followed, since 
// earlier groups use primitives that are removed by later groups.

// Group 1
// * Bit ops:
// 
//   Add decl + axiom for each.  Map away.
//   Add decl + axiom.  Map away
//
// * TERM_EQ:    Add decl + axiom.  Map away TERM_EQ first to id.  Must
//   do for every declared and primitive type.

//   But TERM_EQ on each type is abstracted by refinement if eq
//   non-triv.   Could consider adding axiom at end of refinement, only if
//   it is not abstracted. 
//
//   Simpler for now to add all definitions. Later a much more effective
//   optimisation is to only add decl + axiom for those instances which occur.

// * TERM_I_LE:  Add decl + axiom.  Map away TERM_I_LE first to id.

// Group 2
//
// *. TO_PROP and TO_BIT elimination. 
//
// Group 3. 
//
// *  Eliminating BIT_TY primitive by using interpreted def.
//
//    Replace all occurrences with TYPE_ID{bit___type}
//    Add type def:
//        If has std 2 element interp, then SUBRANGE{0,1} 
//        If has int interp, then INT_TY
// 
// *. Abstract Bit type.  Switch to uninterpreted type.
//   
//    Bit consts:  Go with type. 
//
//    If intro type def as subrange 0,1 or int
//    then add decl for each, and axiom for = 0 and =1
//   
//    If abstract Bit type.  Then just 2 axioms
//     All x:bit. x = false or x = true
//     and false != true.
//   
//   
//   
// Could tidy up much of this code with some more elegant support.
// e.g. one fun abstract_def def_lhs def_rhs.   Do if/when port to OCAML.

// 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Abstract Bit operations
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// operations are TERM_AND, TERM_OR, TERM_NOT, TERM_IFF

// assumes bit type still declared.

//---------------------------------------------------------------------------
// abstractBitOp
//---------------------------------------------------------------------------

Node*
abstractBitOp(Node* n) {

    if (n->kind == TERM_AND) {
        return n->updateKindAndId(FUN_AP, "bit___and");
    }
    else if (n->kind == TERM_OR) {
        return n->updateKindAndId(FUN_AP, "bit___or");
    }
    else if (n->kind == TERM_IFF) {
        return n->updateKindAndId(FUN_AP, "bit___iff");
    }
    else if (n->kind == TERM_NOT) {
        return n->updateKindAndId(FUN_AP, "bit___not");
    }
    else {
        return n;
    }
    assert(false);
}

//---------------------------------------------------------------------------
// abstractBitOps
//---------------------------------------------------------------------------
void
abstractBitOps(FDLContext* ctxt, Node* unit) {
    Node* rules = unit->child(1);

    Node* Bit = Node::bit_ty;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add declarations for new operations
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    Node* andDecl =
        new Node(DECL_FUN, "bit___and", new Node(SEQ, Bit, Bit), Bit);
    Node* orDecl =
        new Node(DECL_FUN, "bit___or",   new Node(SEQ, Bit, Bit), Bit);
    Node* iffDecl =
        new Node(DECL_FUN, "bit___iff", new Node(SEQ, Bit, Bit), Bit);
    Node* notDecl =
        new Node(DECL_FUN, "bit___not", new Node(SEQ, Bit), Bit);

    ctxt->insert(andDecl);
    ctxt->insert(orDecl);
    ctxt->insert(iffDecl);
    ctxt->insert(notDecl);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add axioms for abstract operations
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Use primitive operations for convenience.  

    Node* andAxiom =
          new Node(FORALL,"",
             new Node(SEQ,
                      new Node(DECL,"x",Bit),
                      new Node(DECL,"y",Bit)),
             new Node(IFF,
                      new Node(TO_PROP,
                               new Node(TERM_AND,
                                        new Node(VAR,"x"),
                                        new Node(VAR,"y")
                                        )
                               ),
                      new Node(AND,
                               new Node(TO_PROP, new Node(VAR,"x")),
                               new Node(TO_PROP, new Node(VAR,"y"))
                               )
                      ),
             new Node(PAT,
                      new Node(TERM_AND,
                               new Node(VAR,"x"),
                               new Node(VAR,"y")
                               )
                      )
                   );
  
    
    Node* orAxiom =
          new Node(FORALL,"",
             new Node(SEQ,
                      new Node(DECL,"x",Bit),
                      new Node(DECL,"y",Bit)),
             new Node(IFF,
                      new Node(TO_PROP,
                               new Node(TERM_OR,
                                        new Node(VAR,"x"),
                                        new Node(VAR,"y")
                                        )
                               ),
                      new Node(OR,
                               new Node(TO_PROP, new Node(VAR,"x")),
                               new Node(TO_PROP, new Node(VAR,"y"))
                               )
                      ),
             new Node(PAT,
                      new Node(TERM_OR,
                               new Node(VAR,"x"),
                               new Node(VAR,"y")
                               )
                      )
                   );
  
    Node* iffAxiom =
          new Node(FORALL,"",
             new Node(SEQ,
                      new Node(DECL,"x",Bit),
                      new Node(DECL,"y",Bit)),
             new Node(IFF,
                      new Node(TO_PROP,
                               new Node(TERM_IFF,
                                        new Node(VAR,"x"),
                                        new Node(VAR,"y")
                                        )
                               ),
                      new Node(IFF,
                               new Node(TO_PROP, new Node(VAR,"x")),
                               new Node(TO_PROP, new Node(VAR,"y"))
                               )
                      ),
             new Node(PAT,
                      new Node(TERM_IFF,
                               new Node(VAR,"x"),
                               new Node(VAR,"y")
                               )
                      )
                   );
  
    Node* notAxiom =
          new Node(FORALL,"",
             new Node(SEQ,
                      new Node(DECL,"x",Bit)),
             new Node(IFF,
                      new Node(TO_PROP,
                               new Node(TERM_NOT,
                                        new Node(VAR,"x")
                                        )
                               ),
                      new Node(NOT,
                               new Node(TO_PROP, new Node(VAR,"x"))
                               )
                      ),
             new Node(PAT,
                      new Node(TERM_NOT,
                               new Node(VAR,"x")
                               )
                      )
                   );
  

    rules->addChild(nRULE("bit and",andAxiom));
    rules->addChild(nRULE("bit or",orAxiom));
    rules->addChild(nRULE("bit iff",iffAxiom));
    rules->addChild(nRULE("bit not",notAxiom));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Abstract away operation occurrences
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    unit->mapOver1(abstractBitOp);
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AbstractBit-valued equality
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//---------------------------------------------------------------------------
// abstractBitValuedEq
//---------------------------------------------------------------------------

// TERM_EQ x y T --> FUN_AP{<Tname>___bit_eq} x y

// where <Tname> = typeToName(T);

Node*
abstractBitValuedEq(Node* n) {

    if (n->kind == TERM_EQ  && n->arity() == 3) {
        string typeName = typeToName(n->child(2));
        n->popChild();
        return n->updateKindAndId(FUN_AP, typeName + "___bit_eq");
    } else {
        return n;
    }
}

//---------------------------------------------------------------------------
// mkBitValuedEqDecl
//---------------------------------------------------------------------------


// DECL_FUN{<Tname>___bit_eq} (SEQ T T) BIT_TY

Node* mkBitValuedEqDecl(const string& typeName) {

    return new Node(DECL_FUN,
                    typeName + "___bit_eq",
                    new Node(SEQ, nameToType(typeName), nameToType(typeName)),
                    new Node(BIT_TY)
                    );
}

//---------------------------------------------------------------------------
// mkBitValuedEqAxiom
//---------------------------------------------------------------------------

Node* mkBitValuedEqAxiom(const string& typeName) {

    string opName = typeName + "___bit_eq";
    return
          new Node(FORALL,"",
             new Node(SEQ,
                      new Node(DECL,"x",nameToType(typeName)),
                      new Node(DECL,"y",nameToType(typeName))),
             new Node(IFF,
                      new Node(TO_PROP,
                               new Node(FUN_AP,
                                        opName,
                                        new Node(VAR,"x"),
                                        new Node(VAR,"y")
                                        )
                               ),
                      new Node(EQ,"",
                               new Node(VAR,"x"),
                               new Node(VAR,"y"),
                               nameToType(typeName)
                               )
                      ),
             new Node(PAT,
                      new Node(FUN_AP,
                               opName,
                               new Node(VAR,"x"),
                               new Node(VAR,"y")
                               )
                      )
                   );
  


}

//---------------------------------------------------------------------------
// abstractBitValuedEqs
//---------------------------------------------------------------------------

void
abstractBitValuedEqs(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add declarations for primitive types
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    ctxt->insert(mkBitValuedEqDecl("integer"));
    ctxt->insert(mkBitValuedEqDecl("real"));
    ctxt->insert(mkBitValuedEqDecl("bit___type"));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add axioms for primitive types
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    rules->addChild(nRULE("bit eq (int)",mkBitValuedEqAxiom("integer")));
    rules->addChild(nRULE("bit eq (real)",mkBitValuedEqAxiom("real")));
    rules->addChild(nRULE("bit eq (bit)",mkBitValuedEqAxiom("bit___type")));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Loop over declared types
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


    for (map<string,Node*>::iterator i = ctxt->typeMap.begin();
         i != ctxt->typeMap.end();
         i++ ) {

        Node* typeDecl = i->second;
        //   DEF_TYPE {id}  type 
        // | DEF_TYPE {id}

        string typeId = typeDecl->id;
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Add declaration for declared type
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        ctxt->insert(mkBitValuedEqDecl(typeId));
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Add axiom for declared type
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        rules->addChild(nRULE("bit eq (" + typeId + ")",
                              mkBitValuedEqAxiom(typeId)));
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Abstract away operation occurrences
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    unit->mapOver1(abstractBitValuedEq);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Abstract Bit-valued integer LE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//---------------------------------------------------------------------------
// abstractBitValuedIntLE
//---------------------------------------------------------------------------

Node*
abstractBitValuedIntLE(Node* n) {

    if (n->kind == TERM_I_LE) {
        return n->updateKindAndId(FUN_AP, "integer___bit_le");
    } else {
        return n;
    }
}

//---------------------------------------------------------------------------
// abstractBitValuedIntLEs
//---------------------------------------------------------------------------

void
abstractBitValuedIntLEs(FDLContext* ctxt, Node* unit) {
    Node* rules = unit->child(1);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    Node* decl =
        new Node(DECL_FUN,
                 "integer___bit_le",
                 new Node(SEQ, Node::int_ty, Node::int_ty),
                 Node::bit_ty
                 );

    ctxt->insert(decl);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add axiom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    Node* axiom = 
        new Node(FORALL,"",
                 new Node(SEQ,
                          new Node(DECL,"x",Node::int_ty),
                          new Node(DECL,"y",Node::int_ty)),
                 new Node(IFF,
                          new Node(TO_PROP,
                                   new Node(TERM_I_LE,
                                            new Node(VAR,"x"),
                                            new Node(VAR,"y")
                                            )
                                   ),
                          new Node(I_LE,
                                   new Node(VAR,"x"),
                                   new Node(VAR,"y")
                                   )
                      ),
                 new Node(PAT,
                          new Node(TERM_I_LE,
                                   new Node(VAR,"x"),
                                   new Node(VAR,"y")
                                   )
                          )
                 );
  
    rules->addChild(nRULE("bit le (int)",axiom));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Abstract away occurrences
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    unit->mapOver1(abstractBitValuedIntLE);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Abstract Bit-valued real LE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//---------------------------------------------------------------------------
// abstractBitValuedRealLE
//---------------------------------------------------------------------------

Node*
abstractBitValuedRealLE(Node* n) {

    if (n->kind == TERM_R_LE) {
        return n->updateKindAndId(FUN_AP, "real___bit_le");
    } else {
        return n;
    }
}

//---------------------------------------------------------------------------
// abstractBitValuedRealLEs
//---------------------------------------------------------------------------

void
abstractBitValuedRealLEs(FDLContext* ctxt, Node* unit) {
    Node* rules = unit->child(1);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add declaration
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    Node* decl =
        new Node(DECL_FUN,
                 "real___bit_le",
                 new Node(SEQ, Node::real_ty, Node::real_ty),
                 Node::bit_ty
                 );

    ctxt->insert(decl);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add axiom
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    Node* axiom = 
        new Node(FORALL,"",
                 new Node(SEQ,
                          new Node(DECL,"x",Node::real_ty),
                          new Node(DECL,"y",Node::real_ty)),
                 new Node(IFF,
                          new Node(TO_PROP,
                                   new Node(TERM_R_LE,
                                            new Node(VAR,"x"),
                                            new Node(VAR,"y")
                                            )
                                   ),
                          new Node(R_LE,
                                   new Node(VAR,"x"),
                                   new Node(VAR,"y")
                                   )
                      ),
                 new Node(PAT,
                          new Node(TERM_R_LE,
                                   new Node(VAR,"x"),
                                   new Node(VAR,"y")
                                   )
                          )
                 );
  
    rules->addChild(nRULE("bit le (real)",axiom));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Abstract away occurrences
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    unit->mapOver1(abstractBitValuedRealLE);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Expand coercions between prop(bool formulas) and bit.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// TO_PROP TERM_TRUE --> TRUE
// TO_PROP TERM_FALSE --> FALSE
// TO_PROP b --> EQ b TERM_TRUE BIT_TY
//
// TO_BIT TRUE  --> TERM_TRUE
// TO_BIT FALSE --> TERM_FALSE
// TO_BIT p --> ITE p TERM_TRUE TERM_FALSE BIT_TY


Node*
elimBitPropCoercion(Node* n) {
    if (n->kind == TO_PROP) {

        if (n->child(0)->kind == TERM_TRUE) {
            n->kind = TRUE;
            n->popChild();
        }
        else if (n->child(0)->kind == TERM_FALSE) {
            n->kind = FALSE;
            n->popChild();
        }
        else {
            n->kind = EQ;
            n->addChild(new Node(TERM_TRUE));
            n->addChild(new Node(BIT_TY));
        }
    } else if (n->kind == TO_BIT) {

        if (n->child(0)->kind == TRUE) {
            n->kind = TERM_TRUE;
            n->popChild();
        }
        else if (n->child(0)->kind == FALSE) {
            n->kind = TERM_FALSE;
            n->popChild();
        }
        else {
            n->kind = ITE;
            n->addChild(new Node(TERM_TRUE));
            n->addChild(new Node(TERM_FALSE));
            n->addChild(new Node(BIT_TY));
        }
    }
    return n;
}

void
elimBitPropCoercions(Node* unit) {
    unit->mapOver1(elimBitPropCoercion);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Elim Bit type and constants
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Make them a defined type and constants
//
// 
Node* 
abstractBitTypeTrueFalse(Node* n) {

    if (n->kind == BIT_TY) {
        return new Node(TYPE_ID, "bit___type");
    }
    else if (n->kind == TERM_TRUE) {
        return new Node(CONST, "bit___true");
    }
    else if (n->kind == TERM_FALSE) {
        return new Node(CONST, "bit___false");
    }
    else
        return n;
}




void
elimBitTypeAndConsts(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add definition for new named bit type
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    Node* defType;

        
    if (ctxt->hasFeature("prim bit type has int interp")) {

        defType = new Node(DEF_TYPE, "bit___type", Node::int_ty);

    } else {

        defType = new Node(DEF_TYPE,
                           "bit___type",
                           new Node(SUBRANGE_TY,
                                    new Node (NATNUM, "0"),
                                    new Node (NATNUM, "1"))
                           );
    }

    ctxt->insert(defType);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add declarations for consts
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    Node* trueDecl = new Node(DEF_CONST, "bit___true", Node::bit_ty);
    Node* falseDecl = new Node(DEF_CONST, "bit___false", Node::bit_ty);

    ctxt->insert(trueDecl);
    ctxt->insert(falseDecl);

                 
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add axioms for consts
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    Node* falseAxiom = new Node(EQ,"",
                                new Node(CONST, "bit___false"),
                                new Node(NATNUM, "0"),
                                new Node(BIT_TY));
    Node* trueAxiom = new Node(EQ,"",
                               new Node(CONST, "bit___true"),
                               new Node(NATNUM, "1"),
                               new Node(BIT_TY));
    

    rules->addChild(nRULE("bit false",falseAxiom));
    rules->addChild(nRULE("bit true",trueAxiom));
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Abstract away occurrences of type and constants
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Map over all declarations to catch BIT_TY uses there.

    ctxt->typeSeq.mapOver1(abstractBitTypeTrueFalse);
    ctxt->termSeq.mapOver1(abstractBitTypeTrueFalse);
    unit->mapOver1(abstractBitTypeTrueFalse);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Abstract Bit type and constants
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// To implement.

void
abstractBitTypeAndConsts(FDLContext* ctxt, Node* unit) {

    assert(false);
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Master function for Bit abstraction and elimination
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



void
abstractBit(FDLContext* ctxt, Node* unit) {

    if (option("abstract-bit-ops"))
        abstractBitOps(ctxt,unit);

    if (option("abstract-bit-valued-eqs"))
        abstractBitValuedEqs(ctxt,unit);
    
    if (option("abstract-bit-valued-int-real-le")) {
        abstractBitValuedIntLEs(ctxt,unit);
        abstractBitValuedRealLEs(ctxt,unit);
    }

    if (! option("no-elim-bit-prop-coercions"))
        elimBitPropCoercions(unit);

    if (option("elim-bit-type-and-consts"))
        elimBitTypeAndConsts(ctxt, unit);
    else if (option("abstract-bit-type-and-consts"))
        abstractBitTypeAndConsts(ctxt, unit);

}



