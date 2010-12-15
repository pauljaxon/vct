//========================================================================
//========================================================================
// REFINE-TYPES.CC
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
// Type Refinement
//========================================================================
//========================================================================
/*

Type refinement
===============

Phases

A Ensure all formulas fully typed. 

  Add notion of `type name'.  Applicable to TYPE_IDs and interpreted
  types such as BOOL_TY (QFOL), BIT_TY (FOL), INT_TY and REAL_TY.



B Build map from named types to type representation structures.

C Make pass over all rules and goal formulas
  Do uniformly - no specific code for particular types or operators.

  o Relativise quantifiers, as needed
  o Fix equalities, as needed


D Make pass over all function, constant and predicate declarations
  Do uniformly - no specific code for particular types or operators.

  o update declarations 
    - for now, the types need no updating since not renamed
    - Also declarations themselves not renamed

  o Add new rules
    - constant, function subtyping, if non-trivial
    - function, arity>0 predicate functionality, if non-trivial

  o update existing rules? 


E Make pass over all named types.  

    This is proxy for map over type decls. Is better since includes
    also the interpreted types for which there are no declarations.

    Changes for optimisation purposes are flagged with `might'.
    
  For each named type T:

  o update associated declaration/definition
    - For now, reuse typename
    - For now, only need to change for Subrange type 

  o add new context declarations
+   - (array) might add any_element decl.
    - (bit)   add type definition

    - (ALL)  add decls for mem_T and equiv_T if non-trivial

  o update existing rules
    - (array) might modify rule for const_A operator 
              [how is this to be located?]
    - (array) might modify subtyping rule for const_A operator 
              [But what if hasn't been even generated yet?]
    - (bit)   might modify axioms for bitops
    - (bit)   might modify axioms for all bit-valued relations
              (both user defined and built-ins)
    - (bit)   might modify subtyping 

  o add new rules
    - (array) might add subtyping and functionality axioms for built-ins 
              select and update
    - (array) might add axiom for subtyping of any_element.
    - (bit)   add defs for TERM_TRUE and TERM_FALSE

    - (ALL)  add definitions for mem_T and equiv_T if non-trivial


F Make pass over all rules and goal formulas
  o (bit) Eliminate b2p operator (only *after* equalities fixed)
          [This really doesn't belong to body of type refinement]


*/


//========================================================================
// TypeRep Class
//========================================================================
// Class for type representations

class TypeRep {
private:

    string typeName; // Name of type being represented
    bool memTrivial;
    bool equivTrivial;

    Node* newDecls;  // New decls to be added at *start* of declaration list
    Node* newRules;

public:

    //--------------------------------------------------------------------
    // Constructors and updaters
    //--------------------------------------------------------------------
    TypeRep(const string& TyName, bool memTriv, bool equivTriv)
        : typeName(TyName),
          memTrivial(memTriv),
          equivTrivial(equivTriv) {

        newDecls = new Node(SEQ);
        newRules = new Node(SEQ);
    }

    void addNewDecl(Node* d) { newDecls->addChild(d); }
    void addNewRule(Node* r) { newRules->addChild(r); }

    //--------------------------------------------------------------------
    // Accessors
    //--------------------------------------------------------------------
    // Node* getBaseType();

    string getTypeName() { return typeName;}
    bool  hasMemTrivial() { return memTrivial;}
    bool  hasEquivTrivial() { return equivTrivial;}

    Node* getNewRules() { return newRules; }
    Node* getNewDecls() {return newDecls; }


    //--------------------------------------------------------------------
    // General instance methods
    //--------------------------------------------------------------------
    Node* mkMemPred(Node* arg);

    Node* mkMemPredDecl();
    Node* mkMemPredAxiom(Node* rhs);
    
    // Make instance of equivalence relation
    Node* mkEquivPred(Node* arg1, Node* arg2);

    Node* mkEquivPredDecl();
    Node* mkEquivPredAxiom(Node* rhs);


    // Make application of bit-valued equivalence function
    Node* mkBitEquivFun(Node* arg1, Node* arg2);

    Node* mkBitEquivFunDecl();
    Node* mkBitEquivFunAxiom();


    //--------------------------------------------------------------------
    // Static members
    //--------------------------------------------------------------------

    static bool FOL;
    static TypeRep* boolTypeRep;


}; // END TypeRep

//--------------------------------------------------------------------
// Static data member definitions
//--------------------------------------------------------------------

TypeRep* TypeRep::boolTypeRep;
bool TypeRep::FOL = false;

//--------------------------------------------------------------------
// Instance method definitions
//--------------------------------------------------------------------
// Let <T> = nameToType(<typename>)

Node* 
TypeRep::mkMemPred(Node* arg) {
    if (memTrivial) {
        return new Node(TRUE);
    } else {
        Kind k = FOL ? PRED_AP : FUN_AP;
        return new Node(k, typeName + "___member", arg);
    }
}

// Make DECL_FUN{<typename>___member} (SEQ <T>) BOOL_TY

Node* 
TypeRep::mkMemPredDecl() {
    return new Node(DECL_FUN,
                    typeName + "___member",
                    new Node (SEQ, nameToType(typeName)),
                    new Node (BOOL_TY)
                    );
}

// Make:   All x:<T>. <typename>___member(x) <=>  <rhs>

Node*
TypeRep::mkMemPredAxiom(Node* rhs) {

    Node* lhs = mkMemPred(new Node (VAR, "x"));

    return new Node(FORALL,
                    new Node (SEQ, new Node (DECL,
                                             "x",
                                             nameToType(typeName))),
                    boolTypeRep->mkEquivPred(lhs,rhs)
                    );
}





Node* 
TypeRep::mkEquivPred(Node* arg1, Node* arg2) {

    if (FOL && typeName == string("boolean")) {

        // Useful special case, given how, in FOL case, BOOL_TY is used in 
        // range type position of function declaration to signify a predicate
        // is being declared.

        return new Node(IFF, arg1, arg2);

    } else if (equivTrivial) {
        return new Node(EQ, "", arg1, arg2, nameToType(typeName));

    } else {
        Kind k = (FOL) ? PRED_AP : FUN_AP;
        return new Node(k, typeName + "___equiv", arg1, arg2);
    }
}


// Make DECL_FUN{<typename>___equiv} (SEQ <T> <T>) BOOL_TY

Node* 
TypeRep::mkEquivPredDecl() {
    return new Node(DECL_FUN,
                    typeName + "___equiv",
                    new Node (SEQ, nameToType(typeName), nameToType(typeName)),
                    new Node (BOOL_TY)
                    );
}

// Make:   All x,y:<T>. <typename>___equiv(x,y) <=>  <rhs>

Node*
TypeRep::mkEquivPredAxiom(Node* rhs) {

    Node* lhs = mkEquivPred(new Node (VAR, "x"), new Node (VAR, "y"));

    return new Node(FORALL,
                    new Node (SEQ,
                              new Node (DECL,
                                        "x",
                                        nameToType(typeName)),
                              new Node (DECL,
                                        "y",
                                        nameToType(typeName))
                              ),
                    boolTypeRep->mkEquivPred(lhs,rhs)
                    );
}

Node* 
TypeRep::mkBitEquivFun(Node* arg1, Node* arg2) {
    if (equivTrivial) {
        return new Node(TERM_EQ, "", arg1, arg2, nameToType(typeName));
    } else {
        return new Node(FUN_AP, typeName + "___bit_equiv", arg1, arg2);
    }
}

Node*
TypeRep::mkBitEquivFunDecl() {
    return new Node(DECL_FUN,
                    typeName + "___bit_equiv",
                    new Node (SEQ, nameToType(typeName), nameToType(typeName)),
                    new Node (BIT_TY)
                    );
}

Node*
TypeRep::mkBitEquivFunAxiom() {

    Node* lhs = new Node(TO_PROP,
                         mkBitEquivFun(new Node (VAR, "x"), new Node (VAR, "y"))
                         );
    Node* rhs = mkEquivPred(new Node (VAR, "x"), new Node (VAR, "y"));

    return new Node(FORALL,
                    new Node (SEQ,
                              new Node (DECL,
                                        "x",
                                        nameToType(typeName)),
                              new Node (DECL,
                                        "y",
                                        nameToType(typeName))
                              ),
                    boolTypeRep->mkEquivPred(lhs,rhs)
                    );
}




//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RefineTypes class
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Class to hold state and methods for phases of refinement.

class RefineTypes {

private:
    Node* unit;
    FDLContext* ctxt;

    map<string,TypeRep> typeReps;


public:
    RefineTypes(FDLContext* c, Node* n) {
        ctxt = c;
        unit = n;
        TypeRep::FOL = ctxt->hasFeature("FOL");
    }

    
    TypeRep* insertTypeRep(const string& typeName, bool memTriv, bool EquivTriv) {

        map<string,TypeRep>::iterator i =
            typeReps.insert(make_pair(typeName,
                                      TypeRep(typeName, memTriv, EquivTriv)
                                      )).first;
        return &(i->second);
    }

    TypeRep* insertTrivialTypeRep(const string& typeName) {
        return insertTypeRep(typeName, true, true);
    }
    TypeRep* insertSubtypeTypeRep(const string& typeName) {
        return insertTypeRep(typeName, false, true);
    }
    TypeRep* insertQuotientTypeRep(const string& typeName) {
        return insertTypeRep(typeName, true, false);
    }
    TypeRep* insertGeneralTypeRep(const string& typeName) {
        return insertTypeRep(typeName, false, false);
    }

    
    TypeRep* lookupTypeRep(const string& typeName);

    bool allSubnodesMemTrivial(Node* typeTuple);
 
    bool allSubnodesEquivTrivial(Node* typeTuple);
 
    void initialiseTypeRepMap();

    void refineFormulas();

    void addSubtypingAndFunctionalityRules();

    void addSpecialDeclsAndRules();

}; // END RefineTypes

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RefineTypes methods
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TypeRep*
RefineTypes::lookupTypeRep(const string& typeName) {
    map<string,TypeRep>::iterator i = typeReps.find(typeName);
    if (i == typeReps.end()) {
        printMessage(ERRORm,
                     "RefineTypes::lookupTypeRep: lookup failed for typeName "
                     + typeName
                     );
        return new TypeRep(typeName, true, true);
    } else {
        return & (i->second);
    }
}

//-------------------------------------------------------------------------
// allSubnodesMemTrivial
//-------------------------------------------------------------------------
// Assumes typeTuple is of one of forms
//      TUPLE_TY T1 ... Tn
//      SEQ T1 ... Tn
//      RECORD_TY D1 ... Dn    where Di = DECL{<fieldname>} Ti 
// 
// and Ti are all named types.

// Would be nice to code up in HO Lang...

bool
RefineTypes::allSubnodesMemTrivial(Node* typeTuple) {
    for (int i = 0; i != typeTuple->arity(); i++) {
        Node* childType =
            (typeTuple->kind == RECORD_TY)
            ? typeTuple->child(i)->child(0)
            : typeTuple->child(i);
        if (! lookupTypeRep(typeToName(childType))->hasMemTrivial())
            return false;
    }
    return true;
}
//-------------------------------------------------------------------------
// allSubnodesEquivTrivial
//-------------------------------------------------------------------------
 
bool
RefineTypes::allSubnodesEquivTrivial(Node* typeTuple) {
    for (int i = 0; i != typeTuple->arity(); i++) {
        Node* childType =
            (typeTuple->kind == RECORD_TY)
            ? typeTuple->child(i)->child(0)
            : typeTuple->child(i);
        if (! lookupTypeRep(typeToName(childType))->hasEquivTrivial())
            return false;
    }
    return true;
}


//-------------------------------------------------------------------------
// initialiseTypeRepMap
//-------------------------------------------------------------------------

void
RefineTypes::initialiseTypeRepMap() {

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Update static state of TypeRep class
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Rather ugly. But this saves passing in this info as an argument
    // to all the TypeRep constructors.

    TypeRep::FOL = ctxt->hasFeature("FOL");
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Add special cases: int, real, bool
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    TypeRep::boolTypeRep = insertTrivialTypeRep("boolean");

    insertTrivialTypeRep("integer");

    insertTrivialTypeRep("real");


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Add special case: bit
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    if (option("refine-bit-type-as-int-subtype")) {

        ctxt->addFeature("prim bit type has int interp");
        
        TypeRep* t = insertSubtypeTypeRep("bit___type");


        // Add DECL_FUN{bit___type___member} (SEQ BIT_TY) BOOL_TY

        t->addNewDecl(t->mkMemPredDecl());


        // Add rule:   
        //   All x:bit. bit___type___member(x) <=>  0 <= x and  x <= 1

        Node* rhs = new Node(AND,
                             new Node(I_LE,
                                      new Node(NATNUM, "0"),
                                      new Node(VAR, "x")),
                             new Node(I_LE,
                                      new Node(VAR, "x"),
                                      new Node(NATNUM, "1")));

        t->addNewRule(t->mkMemPredAxiom(rhs));
        
        
    } else if (option("refine-bit-type-as-int-quotient")) {

        ctxt->addFeature("prim bit type has int interp");

        TypeRep* t = insertQuotientTypeRep("bit___type");

        // Add DECL_FUN{bit___type___equiv} (SEQ BIT_TY BIT_TY) BOOL_TY

        t->addNewDecl(t->mkEquivPredDecl());


        // Add rule:
	//   All x,y:bit. bit___type___equiv(x,y) <=>  (b2p(x) <=> b2p(y))

        Node* rhs = new Node(IFF,
                             new Node(TO_PROP, new Node(VAR, "x")),
                             new Node(TO_PROP, new Node(VAR, "y"))
                             );

        t->addNewRule(t->mkEquivPredAxiom(rhs));
        
    } else {
        insertTrivialTypeRep("bit___type");
    }


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Add entry for each type decl
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    for (int i = 0; i != ctxt->typeSeq.arity(); i++) {
        Node* typeDecl = ctxt->typeSeq.child(i);


        if (typeDecl->arity() == 0) {

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // DEF_TYPE{id} - uninterpreted type
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

            if (option("refine-uninterpreted-types")) {
                
		TypeRep* t = insertSubtypeTypeRep(typeDecl->id);

		// Add DECL_FUN{id} (SEQ TYPE_ID{id}) BOOL_TY
		t->addNewDecl(t->mkMemPredDecl());

            } else {
                insertTrivialTypeRep(typeDecl->id);
            }

        } else  // Know typeDecl->arity() > 0
	    if (typeDecl->child(0)->kind == TYPE_ID
                   || typeDecl->child(0)->kind == INT_TY
                   || typeDecl->child(0)->kind == REAL_TY
                   || typeDecl->child(0)->kind == BIT_TY
                   || typeDecl->child(0)->kind == BOOL_TY
                   ) {
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Def type is defining alias.
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // DEF_TYPE{new-id} TYPE_ID{old-id} 
            // DEF_TYPE{new-id} INT_TY
            // DEF_TYPE{new-id} REAL_TY
            // DEF_TYPE{new-id} BIT_TY
            // DEF_TYPE{new-id} BOOL_TY
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            string newTypeId = typeDecl->id;
            string oldTypeName = typeToName(typeDecl->child(0));
            
            TypeRep* oldTypeIdRep = lookupTypeRep(oldTypeName);

            TypeRep* newTypeIdRep = 
                insertTypeRep(newTypeId,
                              oldTypeIdRep->hasMemTrivial(),
                              oldTypeIdRep->hasEquivTrivial());

            if (!oldTypeIdRep->hasMemTrivial()) {

                // Add DECL_FUN{<new-id>___member} (SEQ TYPE_ID{new-id}) BOOL_TY

                newTypeIdRep->addNewDecl(newTypeIdRep->mkMemPredDecl());
                              
                // Add rule:
                // All x:<new-id>. <new-id>___member(x) <=>  <old-id>___member(x)

                Node* rhs = oldTypeIdRep->mkMemPred(new Node(VAR, "x"));

                newTypeIdRep->addNewRule(newTypeIdRep->mkMemPredAxiom(rhs));

            }
            if (!oldTypeIdRep->hasEquivTrivial()) {

                // Add DECL_FUN{<new-id>___equiv} (SEQ TYPE_ID{new-id}
                //                                     TYPE_ID{new-id}
                //                                 ) BOOL_TY

                newTypeIdRep->addNewDecl(newTypeIdRep->mkEquivPredDecl());

                // Add rule:
                // All x,y:bit. <new-id>___equiv(x,y) <=>  <old-id>___equiv(x)

                Node* rhs = oldTypeIdRep->mkEquivPred(new Node(VAR, "x"),
                                                      new Node(VAR, "y"));
                
                newTypeIdRep->addNewRule(newTypeIdRep->mkEquivPredAxiom(rhs));

            }

        } else if (typeDecl->child(0)->kind == SUBRANGE_TY) {
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // DEF_TYPE{id} (SUBRANGE_TY{t-id} <i>  <j>)
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

            if (option("refine-int-subrange-type")) {

                TypeRep* t = insertSubtypeTypeRep(typeDecl->id);

                // Add DECL_FUN{<t-id>___member} (SEQ TYPE_ID{t-id}) BOOL_TY

                t->addNewDecl(t->mkMemPredDecl());
                              
                // Add rule:
                // All x:<t-id>. <t-id>___member(x) <=>  <i> <= x /\ x <= <j>

                Node* lower = typeDecl->child(0)->child(0);
                Node* upper = typeDecl->child(0)->child(1);
                Node* rhs = new Node(AND,
                                     new Node(I_LE,
                                              lower,
                                              new Node(VAR, "x")),
                                     new Node(I_LE,
                                              new Node(VAR, "x"),
                                              upper));

                t->addNewRule(t->mkMemPredAxiom(rhs));
                
            } else {
                insertTrivialTypeRep(typeDecl->id);
            }
            
        } else if (typeDecl->child(0)->kind == ENUM_TY) {
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // DEF_TYPE{id} (ENUM_TY{type-id} ... )
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Expecting that usually enumerated types are abstracted or
            // eliminated by this stage.
            
            insertTrivialTypeRep(typeDecl->id);


        } else if (typeDecl->child(0)->kind == ARRAY_TY) {
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // DEF_TYPE{<t-id>} (ARRAY_TY{<t-id>} (TUPLE_TY S1 ... Sn) T)
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            Node* indexTypes = typeDecl->child(0)->child(0);
            Node* elementType = typeDecl->child(0)->child(1);
            TypeRep* elementTypeRep = lookupTypeRep(typeToName(elementType));
            
            bool allIndexTypesMemTrivial =
                allSubnodesMemTrivial(indexTypes);
            bool allIndexTypesEquivTrivial =
                allSubnodesEquivTrivial(indexTypes);

            if (! allIndexTypesEquivTrivial) {
                printMessage(ERRORm, 
                             "Type refinement cannot handle array type "
                             + (typeDecl->id)
                             + "with non-trivial equivalence" + ENDLs
                             + "on one or more index types"
                             );

                insertTrivialTypeRep(typeDecl->id);
                continue; // Go on to next type decl
            }
            // Index types all have trivial equiv rel from now on

            bool arrayEquivTrivial;
            bool arrayMemTrivial;

            if (option("refine-array-types-with-quotient")) {
                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // Out of bounds elements unconstrained
                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                arrayMemTrivial = elementTypeRep->hasMemTrivial();
                arrayEquivTrivial =
                    allIndexTypesMemTrivial
                    && elementTypeRep->hasEquivTrivial();

                TypeRep* arrayTypeRep = 
                    insertTypeRep(typeDecl->id,
                                  arrayMemTrivial,
                                  arrayEquivTrivial);


                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // Add declaration and defining rule for mem predicate
                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                if (!arrayMemTrivial) {

                    // Add DECL_FUN{<t-id>___member} (SEQ <t-id>) BOOL_TY

                    arrayTypeRep->addNewDecl(arrayTypeRep->mkMemPredDecl());

                    // RHS of def is
                    //
                    // FORALL (SEQ (DECL{s1} S1) ... (DECL{sn} Sn))
                    //   mem_S1(s1) /\ ... /\ mem_Sn(sn)
                    //   => 
                    //   mem_T( select_A(x, (SEQ s1 ... sn))
                    //

                    Node* forallDecls = new Node(SEQ);

                    Node* hyps = new Node(AND);
                    Node* indexes = new Node(SEQ);
            
                    for (int i = 0; i != indexTypes->arity(); i++) {
                        Node* indexType = indexTypes->child(i);
                        TypeRep* t = lookupTypeRep(typeToName(indexType));
                        
                        string varName = "s" + intToString(i);
                        forallDecls->addChild(new Node (DECL,
                                                        varName,
                                                        indexTypes->child(i)->copy()));

                        indexes->addChild(new Node(VAR, varName));

                        if (!t->hasMemTrivial()) {
                            Node* memPred = t->mkMemPred(new Node(VAR, varName));
                            hyps->addChild(memPred);
                        }
                    }
                    Node* concl =
                        elementTypeRep->mkMemPred(new Node
                                                  (ARR_ELEMENT,
                                                   typeDecl->id,
                                                   new Node(VAR, "x"),
                                                   indexes)
                                                  );
                                                            
                    Node* rhs = new Node(FORALL,
                                         forallDecls,
                                         mkGeneralImplies(hyps, concl));

                    arrayTypeRep->addNewRule(arrayTypeRep->mkMemPredAxiom(rhs));
                                         
                } // END IF !arrayMemTrival

                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // Add declaration and defining rule for equiv predicate
                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                if (!arrayEquivTrivial) {

                    // Add DECL_FUN{<t-id>___equiv} (SEQ <t-id> <t-id>) BOOL_TY

                    arrayTypeRep->addNewDecl(arrayTypeRep->mkEquivPredDecl());

                    // RHS of def is
                    //
                    // FORALL (SEQ (DECL{s1} S1) ... (DECL{sn} Sn))
                    //   mem_S1(s1) /\ ... /\ mem_Sn(sn)
                    //   => 
                    //   equiv_T(select_A(x, (SEQ s1 ... sn)),
                    //           select_A(y, (SEQ s1 ... sn)))
                    //

                    Node* forallDecls = new Node(SEQ);

                    Node* hyps = new Node(AND);
                    Node* indexes = new Node(SEQ);
            
                    for (int i = 0; i != indexTypes->arity(); i++) {
                        Node* indexType = indexTypes->child(i);
                        TypeRep* t = lookupTypeRep(typeToName(indexType));
                        
                        string varName = "s" + intToString(i);
                        forallDecls->addChild(new Node (DECL,
                                                        varName,
                                                        indexTypes->child(i)->copy()));

                        indexes->addChild(new Node(VAR, varName));

                        if (!t->hasMemTrivial()) {
                            Node* memPred = t->mkMemPred(new Node(VAR, varName));
                            hyps->addChild(memPred);
                        }
                    }
                    Node* concl =
                        elementTypeRep->mkEquivPred(new Node (ARR_ELEMENT,
                                                              typeDecl->id,
                                                              new Node(VAR, "x"),
                                                              indexes),
                                                    new Node (ARR_ELEMENT,
                                                              typeDecl->id,
                                                              new Node(VAR, "y"),
                                                              indexes->copy())
                                                    );
                                                            
                    Node* rhs = new Node(FORALL,
                                         forallDecls,
                                         mkGeneralImplies(hyps, concl));

                    arrayTypeRep->addNewRule(arrayTypeRep->mkEquivPredAxiom(rhs));

                } // END IF !arrayEquivTrivial


            } else {
                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // Out of bounds elements constrained
                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                arrayMemTrivial =
                    allIndexTypesMemTrivial
                    && elementTypeRep->hasMemTrivial();
                
                arrayEquivTrivial = elementTypeRep->hasEquivTrivial();

                TypeRep* arrayTypeRep = 
                    insertTypeRep(typeDecl->id,
                                  arrayMemTrivial,
                                  arrayEquivTrivial);

                string defaultElementName =
                    typeDecl->id + "___default_arr_element";

                Node* defaultElementDecl =
                    new Node (DEF_CONST,
                              defaultElementName,
                              elementType->copy());
                              

                arrayTypeRep->addNewDecl(defaultElementDecl);

                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // Add declaration and defining rule for mem predicate
                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                if (!arrayMemTrivial) {

                    // Add DECL_FUN{<t-id>___member} (SEQ <t-id>) BOOL_TY

                    arrayTypeRep->addNewDecl(arrayTypeRep->mkMemPredDecl());

                    // RHS of def is
                    //
                    // FORALL (SEQ (DECL{s1} S1) ... (DECL{sn} Sn))
                    //   mem_S1(s1) /\ ... /\ mem_Sn(sn)
                    //   => 
                    //   mem_T( select_A(x, (SEQ s1 ... sn))
                    // AND
                    //   NOT (mem_S1(s1) /\ ... /\ mem_Sn(sn))
                    //   => 
                    //  equiv_T( select_A(x, (SEQ s1 ... sn), default_el_A)
                    //
                    //  
                    //We could use EQ over T rather than equiv_T.
                    //May be easier to reason with???
                    // Allow for either

                    Node* forallDecls = new Node(SEQ);

                    Node* hyps = new Node(AND);
                    Node* indexes = new Node(SEQ);
            
                    for (int i = 0; i != indexTypes->arity(); i++) {
                        Node* indexType = indexTypes->child(i);
                        TypeRep* t = lookupTypeRep(typeToName(indexType));
                        
                        string varName = "s" + intToString(i);
                        forallDecls->addChild(new Node (DECL,
                                                        varName,
                                                        indexTypes->child(i)->copy()));

                        indexes->addChild(new Node(VAR, varName));

                        if (!t->hasMemTrivial()) {
                            Node* memPred = t->mkMemPred(new Node(VAR, varName));
                            hyps->addChild(memPred);
                        }
                    }
                    Node* selectExp = new Node(ARR_ELEMENT,
                                               typeDecl->id,
                                               new Node(VAR, "x"),
                                               indexes);

                    Node* defaultElement = new Node(CONST, defaultElementName);

                    Node* extConstraint =
                        option("refine-array-types-with-weak-extension-constraint")
                        ? elementTypeRep->mkEquivPred(selectExp->copy(),
                                                      defaultElement)
                        : new Node(EQ, "",
                                   selectExp->copy(),
                                   defaultElement,
                                   elementType->copy()
                                   );

                    Node* concl =
                        new Node(AND,
                                 mkGeneralImplies(hyps,
                                                  elementTypeRep
                                                  ->mkMemPred(selectExp)
                                                  ),
                                 mkGeneralImplies(new Node (NOT,
                                                            hyps->copy()),
                                                  extConstraint
                                                  ));
                                                            
                    Node* rhs = new Node(FORALL, forallDecls, concl);

                    arrayTypeRep->addNewRule(arrayTypeRep->mkMemPredAxiom(rhs));
                                         
                } // END IF !arrayMemTrival

                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // Add declaration and defining rule for equiv predicate
                // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                if (!arrayEquivTrivial) {

                    // Add DECL_FUN{<t-id>___equiv} (SEQ <t-id> <t-id>) BOOL_TY

                    arrayTypeRep->addNewDecl(arrayTypeRep->mkEquivPredDecl());

                    // RHS of def is
                    //
                    // FORALL (SEQ (DECL{s1} S1) ... (DECL{sn} Sn))
                    //   equiv_T(select_A(x, (SEQ s1 ... sn)),
                    //           select_A(y, (SEQ s1 ... sn)))
                    //

                    Node* forallDecls = new Node(SEQ);

                    Node* indexes = new Node(SEQ);
            
                    for (int i = 0; i != indexTypes->arity(); i++) {
                        //  Node* indexType = indexTypes->child(i);
                        
                        string varName = "s" + intToString(i);
                        forallDecls->addChild(new Node (DECL,
                                                        varName,
                                                        indexTypes->child(i)->copy()));

                        indexes->addChild(new Node(VAR, varName));

                    }


                    Node* concl =
                        elementTypeRep->mkEquivPred(new Node (ARR_ELEMENT,
                                                              typeDecl->id,
                                                              new Node(VAR, "x"),
                                                              indexes),
                                                    new Node (ARR_ELEMENT,
                                                              typeDecl->id,
                                                              new Node(VAR, "y"),
                                                              indexes->copy())
                                                    );
                                                            
                    Node* rhs = new Node(FORALL, forallDecls, concl);

                    arrayTypeRep->addNewRule(arrayTypeRep->mkEquivPredAxiom(rhs));

                } // END IF !arrayEquivTrivial

            }
            

        } else if (typeDecl->child(0)->kind == RECORD_TY) {
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // DEF_TYPE{R} (RECORD_TY{R} D1 ... Dn)
            // where Di = DECL{fi} Ti
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

            bool recordMemTrivial = allSubnodesMemTrivial(typeDecl->child(0));
            bool recordEquivTrivial = allSubnodesEquivTrivial(typeDecl->child(0));

            TypeRep* recordTypeRep = 
                insertTypeRep(typeDecl->id,
                              recordMemTrivial,
                              recordEquivTrivial
                              );

            Node* recordTy = typeDecl->child(0);
            
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Add declaration and defining rule for mem predicate
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            if (!recordMemTrivial) {

                // Add DECL_FUN{<R>___member} (SEQ <R>) BOOL_TY

                recordTypeRep->addNewDecl(recordTypeRep->mkMemPredDecl());

                // RHS of def is
                //
                // mem_T1(RCD_ELEMENT{f1} x R) 
                // /\ ... /\ mem_Tn(RCD_ELEMENT{fn} x R) 

                Node* rhs = new Node(AND);
                
                for (int i = 0; i != recordTy->arity(); i++) {
                    Node* decl = recordTy->child(i);
                    Node* fieldTy = decl->child(0);

                    TypeRep* t = lookupTypeRep(typeToName(fieldTy));

                    Node* conjunct =
                        t->mkMemPred(new Node(RCD_ELEMENT,
                                              decl->id,
                                              new Node(VAR, "x"),
                                              new Node(TYPE_PARAM,
	 					       recordTy->id) ));


                    rhs->addChild(conjunct);
                }
                if (rhs->arity() == 1) rhs = rhs->child(0);
                
                recordTypeRep->addNewRule(recordTypeRep->mkMemPredAxiom(rhs));

            } // END IF !recordMemTrivial

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Add declaration and defining rule for equiv predicate
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            if (!recordEquivTrivial) {

                // Add DECL_FUN{<R>___equiv} (SEQ <R> <R>) BOOL_TY

                recordTypeRep->addNewDecl(recordTypeRep->mkEquivPredDecl());

                // RHS of def is
                //
                // equiv_T1 (RCD_ELEMENT{f1} x R) (RCD_ELEMENT{f1} y R) 
                // /\ ... /\ ...
                // equiv_Tn (RCD_ELEMENT{fn} x R) (RCD_ELEMENT{fn} y R) 


                Node* rhs = new Node(AND);
                
                for (int i = 0; i != recordTy->arity(); i++) {
                    Node* decl = recordTy->child(i);
                    Node* fieldTy = decl->child(0);

                    TypeRep* t = lookupTypeRep(typeToName(fieldTy));

                    Node* conjunct =
                        t->mkEquivPred(new Node(RCD_ELEMENT,
                                                decl->id,
                                                new Node(VAR, "x"),
						new Node(TYPE_PARAM,
							 recordTy->id) ),
                                       new Node(RCD_ELEMENT,
                                                decl->id,
                                                new Node(VAR, "y"),
						new Node(TYPE_PARAM,
							 recordTy->id) )
			               );

                    rhs->addChild(conjunct);
                }
                if (rhs->arity() == 1) rhs = rhs->child(0);
                
                recordTypeRep->addNewRule(recordTypeRep->mkEquivPredAxiom(rhs));
            }
                          
        } else {
            printMessage(ERRORm,
                         "RefineTypes::initialiseTypeRepMap: unrecognised type "
                         + kindString(typeDecl->child(0)->kind)
                         );
        } // END switch ***

    } // END for loop over type defs
    return;
}


//-------------------------------------------------------------------------
// refineFormulas
//-------------------------------------------------------------------------
// Relativise quantifiers
// Fix equalities, both bit-valued and propositional

// Local class for closure.  Need to map function over formula tree
// that carries with it a pointer to the RefineTypes object containing
// the typeReps map.


class RefineFormulaClosure {

private:
    RefineTypes* refineTypesState;
public:
    RefineFormulaClosure(RefineTypes* r) : refineTypesState(r) {}

    Node* operator () (Node* n);
};

// FORALL (SEQ d1 .. dn) B  -->
//    FORALL (SEQ d1 .. dn) (Mem_T1(x1) /\ ... /\ Mem_T2(xn)) => B 
// EXISTS (SEQ d1 .. dn) B  -->
//    EXISTS (SEQ d1 .. dn) Mem_T1(x1) /\ ... /\ Mem_T2(xn) /\ B 
//
// where di = DECL{xi} Ti 
//
// Mem_Ti omitted if it is trivial.
// 
// 
Node* 
RefineFormulaClosure::operator() (Node* n) {
    if (n->kind == FORALL || n->kind == EXISTS) {


        
        Node* decls = n->child(0);

        Node* conditions = new Node(AND);
        for (int i = 0; i != decls->arity(); i++) {

            Node* decl = decls->child(i);
            Node* ty = decl->child(0);

            TypeRep* t = refineTypesState->lookupTypeRep(typeToName(ty));

            if (!t->hasMemTrivial()) {
                Node* memPred = t->mkMemPred(new Node(VAR, decl->id));
                conditions->addChild(memPred);
            }
        }
        if (conditions->arity() > 0) {

	    printOnOption("trace-refine-types-quant-relativisation",
			  "Relativising " + kindString(n->kind) + " over "
			  + n->child(0)->toString() );

            if (n->kind == FORALL) {

                Node* hyps =
                    (conditions->arity() >= 2)
                    ? conditions
                    : conditions->child(0);

                n->child(1) = new Node(IMPLIES, hyps, n->child(1));
                
            } else { // n->kind == EXISTS 

                conditions->addChild(n->child(1));
                n->child(1) = conditions;
            }
        }
    }
    // EQ t1 t2 T
    else if (n->kind == EQ) {

        TypeRep* t = refineTypesState->lookupTypeRep(typeToName(n->child(2)));

        if (!t->hasEquivTrivial()) {

	    Node* newEquivPredAp = t->mkEquivPred(n->child(0), n->child(1));

	    printOnOption("trace-refine-types-eq-refinement",
			  "Refining equality over " + t->getTypeName());

	    return newEquivPredAp;
        }
    }
    else if (n->kind == TERM_EQ) {

        TypeRep* t = refineTypesState->lookupTypeRep(typeToName(n->child(2)));

        if (!t->hasEquivTrivial()) {

	    printOnOption("trace-refine-types-bit-eq-refinement",
			  "Refining bit-valued equality over "
			  + t->getTypeName());

            return t->mkBitEquivFun(n->child(0), n->child(1));
        }

    }
    return n;
}


void 
RefineTypes::refineFormulas() {
    RefineFormulaClosure c(this);

    unit->mapOver1(c);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// addSubtypingAndFunctionalityRules()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
RefineTypes::addSubtypingAndFunctionalityRules() {
    Node* rules = unit->child(1);

    //------------------------------------------------------------------------
    // Add rules for constant subtyping
    //------------------------------------------------------------------------
    map<string,Node*>::iterator i;
    for (i = ctxt->constMap.begin(); i != ctxt->constMap.end(); i++) {
        Node* decl = i->second;
        // DEF_CONST{id} type [exp]

        TypeRep* t = lookupTypeRep(typeToName(decl->child(0)));
            
        if (!t->hasMemTrivial()) {
            rules->addChild(t->mkMemPred(new Node(CONST, decl->id)));
        }
            
    }

    //------------------------------------------------------------------------
    // Add rules for FDL global variable subtyping
    //------------------------------------------------------------------------
    for (i = ctxt->varMap.begin(); i != ctxt->varMap.end(); i++) {
        Node* decl = i->second;
        // DECL_VAR{id} 
        TypeRep* t = lookupTypeRep(typeToName(decl->child(0)));
            
        if (!t->hasMemTrivial()) {
            rules->addChild(t->mkMemPred(new Node(CONST, decl->id)));
        }
    }

    //------------------------------------------------------------------------
    // Add rules for function subtyping and functionality, and relation
    // functionality
    //------------------------------------------------------------------------

    for (i = ctxt->funMap.begin(); i != ctxt->funMap.end(); i++) {
        Node* decl = i->second;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Process function declaration  DECL_FUN{f} (SEQ S1 ... Sn) T
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        string declId = decl->id;
        Node* domTypes = decl->child(0);
        Node* rangeType = decl->child(1);

        TypeRep* rangeTypeRep = lookupTypeRep(typeToName(rangeType));

        if (!option("no-subtyping-axioms") && !rangeTypeRep->hasMemTrivial()) {

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Add subtyping axiom
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // FORALL (SEQ (DECL{x1} S1) ... (DECL{xn} Sn))
            //     Mem_S1(x1) /\ ... /\ Mem_Sn(xn) => MemT(f(x1,...,xn))
            //
            // or
            //
            // FORALL (SEQ (DECL{x1} S1) ... (DECL{xn} Sn)) MemT(f(x1,...,xn))

            Node* forallDecls = new Node(SEQ);
            Node* hyps = new Node(AND);
            Node* funAp = new Node(FUN_AP, declId);
            
            for (int i = 0; i != domTypes->arity(); i++) {
                string varName = "x" + intToString(i);
                forallDecls->addChild(new Node (DECL,
                                                varName,
                                                domTypes->child(i)->copy()));
                funAp->addChild(new Node(VAR, varName));

                if (!option("strong-subtyping-axioms")) {
                    Node* ty = domTypes->child(i);
                    TypeRep* t = lookupTypeRep(typeToName(ty));

                    if (!t->hasMemTrivial()) {
                        Node* memPred = t->mkMemPred(new Node(VAR, varName));
                        hyps->addChild(memPred);
                    }
                }
            }
            Node* memPredFunAp = rangeTypeRep->mkMemPred(funAp);
            
            Node* body = mkGeneralImplies(hyps, memPredFunAp);

            Node* subtypingAxiom = new Node(FORALL, forallDecls, body);
            rules->addChild(subtypingAxiom);
        }

        
        // Functionality lemma.

        if (!option("no-functionality-axioms")
            && !allSubnodesEquivTrivial(domTypes)) {

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // Add functionality axiom
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
            // FORALL (SEQ (DECL{x1} S1) (DECL{y1} S1) ... 
            //             (DECL{xn} Sn) (DECL{yn} Sn)
            //        )
            //     Mem_S1(x1) /\ Mem_S1(y1) /\ Equiv_S1(x1, y1)     
            //     /\ ...
            //     /\ Mem_Sn(x1) /\ Mem_Sn(y1) /\ xn =_Sn yn 

            //     => Equiv_T(f(x1,...,xn), f(y1,...,yn))
            //

            Node* forallDecls = new Node(SEQ);
            Node* hyps = new Node(AND);
            
            Kind apKind;
            if (rangeType->kind == BOOL_TY && ctxt->hasFeature("FOL"))
                apKind = PRED_AP;
            else 
                apKind = FUN_AP;
                    
            Node* funApX = new Node(apKind, decl->id);
            Node* funApY = new Node(apKind, decl->id);
            
            for (int i = 0; i != domTypes->arity(); i++) {
                string varNameXi = "x" + intToString(i);
                string varNameYi = "y" + intToString(i);
                forallDecls->addChild(new Node (DECL,
                                                varNameXi,
                                                domTypes->child(i)->copy()));
                forallDecls->addChild(new Node (DECL,
                                                varNameYi,
                                                domTypes->child(i)->copy()));
                funApX->addChild(new Node(VAR, varNameXi));
                funApY->addChild(new Node(VAR, varNameYi));

                Node* ty = domTypes->child(i);
                TypeRep* t = lookupTypeRep(typeToName(ty));

                if (!t->hasMemTrivial()) {
                    Node* memPredXi = t->mkMemPred(new Node(VAR, varNameXi));
                    Node* memPredYi = t->mkMemPred(new Node(VAR, varNameYi));
                    hyps->addChild(memPredXi);
                    hyps->addChild(memPredYi);
                }
                Node* equivPredXYi = t->mkEquivPred(new Node(VAR, varNameXi),
                                                 new Node(VAR, varNameYi));
                hyps->addChild(equivPredXYi);
                

                
            }

            Node* equivPredFunAps = rangeTypeRep->mkEquivPred(funApX, funApY);

            Node* functionalityAxiom =
                new Node(FORALL,
                         forallDecls,
                         mkGeneralImplies(hyps, equivPredFunAps)
                         );

            rules->addChild(functionalityAxiom);

        } // END add functionality axiom

    } // END iteration over FUN_DECLs
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// addSpecialDeclsAndRules();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void 
RefineTypes::addSpecialDeclsAndRules() {

    Node* rules = unit->child(1);
    
    for (map<string,TypeRep>::iterator i = typeReps.begin();
         i != typeReps.end();
         i++ ) {
        TypeRep* t = &(i->second);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Add new entries to context
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        Node* decls = t->getNewDecls();

        for (int i = 0; i != decls->arity(); i++) {

            ctxt->insert(decls->child(i));
        }
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        // Add new rules
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (!option("disable-refinement-rules")) {
            rules->appendChildren(t->getNewRules());
        }

        if (option("refine-bit-eq-equiv") && !t->hasEquivTrivial()) {
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Add decl for bit-valued equiv rel
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        
            ctxt->insert(t->mkBitEquivFunDecl());
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Add defining axiom for bit-valued equiv rel
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            rules->addChild(t->mkBitEquivFunAxiom());
            
        }

    }
    return;
}

void abstractSubrangeTypeDecl (Node* n) {
    if (n->kind == DEF_TYPE
        && n->arity() == 1
        && n->child(0)->kind == SUBRANGE_TY) {
        
        n->child(0) = new Node(INT_TY);
    }
}

//========================================================================
// Master function for invoking type refinement
//========================================================================

void refineTypes (FDLContext* ctxt, Node* unit) {

    RefineTypes state(ctxt, unit);

    state.initialiseTypeRepMap();

    state.refineFormulas();

    if (!option("disable-subtyping-functionality-rules"))
        state.addSubtypingAndFunctionalityRules();

    state.addSpecialDeclsAndRules();

    if (option("refine-int-subrange-type")) {
        ctxt->typeSeq.mapOver(abstractSubrangeTypeDecl);
    }
    return;
}



