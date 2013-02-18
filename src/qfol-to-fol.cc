//========================================================================
//========================================================================
// QFOL-TO-FOL.CC
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
// QFOL to FOL conversion
//========================================================================
//========================================================================
// Introduce bit type for Boolean values at individual level.

// Check if proposition in non-propositional context.

// If there is a term valued alternative, fix operator kind.
// Report if encounter unfixable boolean value/operator.




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// boolToBit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Node* boolToBit(Node* n) {
    if (n->kind == BOOL_TY) {
        return Node::bit_ty;
    } else {
        return n;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// boolTypeParamsToBit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 1. Convert all uses of Bool as component types of arrays and records to
//    Bit.  
// 2. Ensure that all aliases for Bool become aliases for Bit.
// 3. Convert all uses of Bool in quantifier types to Bit
// 4. Convert all equalities over Bool to equalities over Bit
//    (both in EQ and TERM_EQ)
//
//

void
boolTypeParamsToBit(FDLContext* ctxt, Node* unit) {
    for (map<string,Node*>::iterator i = ctxt->typeMap.begin();
         i != ctxt->typeMap.end();
         i++ ) {

        Node* typeDecl = i->second;
        //   DEF_TYPE  {id}  type 
        // | DEF_TYPE {id}
        if (typeDecl->arity() == 0) continue;

        Node* type = typeDecl->child(0);

        if (type->kind == BOOL_TY) {
            typeDecl->child(0) = Node::bit_ty;
        }

        // ARRAY_TY (SEQ type+) type
        // RECORD_TY (DECL{id} type)+
         
        else if (type->kind == ARRAY_TY || type->kind == RECORD_TY) {

            typeDecl->mapOver1(boolToBit);
        }
    }
    // Convert any uses of BOOL_TY in quantifiers and equalities. 

    unit->mapOver1(boolToBit);

    
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// introBitOpsAndRels
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Node*
introBitOpsAndRel(FDLContext* ctxt, Node* n) {
    if (! ctxt->isFormula) {
        switch(n->kind) {
        case TRUE: {
            n->kind = TERM_TRUE;
            break;
        }
        case FALSE: {
            n->kind = TERM_FALSE;
            break;
        }
        case AND: {
            n->kind = TERM_AND;
            break;
        }
        case OR: {
            n->kind = TERM_OR;
            break;
        }
        case NOT: {
            n->kind = TERM_NOT;
            break;
        }
        case IMPLIES: {
            n->kind = TERM_OR;
            n->child(0) = new Node(TERM_NOT, n->child(0));
            break;
        }
        case IFF: {
            n->kind = TERM_EQ;
            n->addChild(new Node(BOOL_TY));
            break;
        }
        case EQ: {
            n->kind = TERM_EQ;
            break;
        }
        case I_LT: {  // x < y  <=>  not (y <= x)
            Node* x = n->child(0);
            Node* y = n->child(1);
            n->kind = TERM_NOT;
            n->popChild();
            n->child(0) = new Node(TERM_I_LE, y, x);
            break;
        }
        case I_LE: {
            n->kind = TERM_I_LE;
            break;
        }
        case R_LT: {  // x < y  <=>  not (y <= x)
            Node* x = n->child(0);
            Node* y = n->child(1);
            n->kind = TERM_NOT;
            n->popChild();
            n->child(0) = new Node(TERM_R_LE, y, x);
            break;
        }
        case R_LE: {
            n->kind = TERM_R_LE;
            break;
        }
        default: {
            if (isProp(n)) {
                printMessage(ERRORm,
                             "Found " + kindString(n->kind) +
                             " at individual position");
                ctxt->addFeature("Error");
            }
            return n;
        }
        } // END switch

	// Should only reach here if progress made

	if (n->kind == TERM_EQ) {
	    printOnOption("trace-intro-bit-ops-and-rels",
			  "Introducing TERM_EQ over "
			  + typeToName(n->child(2)));
	} else {
		printOnOption("trace-intro-bit-ops-and-rels",
			      "Introducing " + kindString(n->kind));
	}
			  
    }
    return n;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IntroBitTypeFindIndividualIds class
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Class for function-object closure.
//
//
// Find ids for consts, FDL vars and functions used in individual positions

class IntroBitTypeFindIndividualIds {
public:
    set<string> indConstIds;
    set<string> indFunIds;
 
    bool isIndConstId(string id) {
        return indConstIds.find(id) != indConstIds.end();
    }
        
    bool isIndFunId(string id) {
        return indFunIds.find(id) != indFunIds.end();
    }
        
    Node* operator() (FDLContext* c, Node* n);
};

Node*
IntroBitTypeFindIndividualIds::operator() (FDLContext* c, Node* n) {

    if (c->isFormula) return n;

    // In individual context
    if (  n->kind == CONST
          && c->lookupEnumConst(n->id) == 0) {

        // CONST should be for FDL var or const
        if (c->lookupId(n->id) == 0) {
            printMessage(ERRORm, "Undeclared CONST encountered: " + (n->id) );
        }
        else {
            indConstIds.insert(n->id);
        }
    }
    if (n->kind == FUN_AP) {
        indFunIds.insert(n->id);
    }
    return n;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// introBitFixFunConstDecls()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
introBitFixFunConstDecls(FDLContext* ctxt, Node* unit) {

    bool preferBitVals = option("bit-type-prefer-bit-vals");
    bool preferProps   = option("bit-type-prefer-props");

    if ( (preferBitVals && preferProps)
         || (preferProps && !option("bit-type-with-ite"))
         ) {
        printMessage(ERRORm, "Invalid combination of bit-type options");
    }
    
    //----------------------------------------------------------------------
    // Gather ids for CONSTs and FUN_APs in individual positions
    //----------------------------------------------------------------------

    IntroBitTypeFindIndividualIds indIds;

    mapOverWithContext(indIds, ctxt, unit);


    //----------------------------------------------------------------------
    // Change Bool types to bit types in declarations
    //----------------------------------------------------------------------

    // Resolve use of Bool (or aliases of Bool) for constant, FDL
    // variable, and function range type to either Bool or Bit type.

    // If preferBitVals, always use Bit type.
    // If preferProps, always use Bool
    
    // Else use Bit just when const/var/fun is used in an individual position.


    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Work over constant decls and defs
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


    for (map<string,Node*>::iterator i = ctxt->constMap.begin();
         i != ctxt->constMap.end();
         i++) {

        //   DEF_CONST  {id} type exp
        // | DEF_CONST {id} type
        Node* constDecl = i->second;

        Node* baseType = ctxt->normaliseType(constDecl->child(0));
        if (baseType->kind == BOOL_TY) {

            if ( (preferBitVals || indIds.isIndConstId(constDecl->id))
                 && !preferProps) {
                constDecl->child(0) = Node::bit_ty;
            } else {
                constDecl->child(0) = Node::bool_ty;
            }
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Work over FDL variable decls 
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


    for (map<string,Node*>::iterator i = ctxt->varMap.begin();
         i != ctxt->varMap.end();
         i++) {

        // DECL_VAR {id} type
        Node* varDecl = i->second;
        Node* baseType = ctxt->normaliseType(varDecl->child(0));
        if (baseType->kind == BOOL_TY) {

            if ( (preferBitVals || indIds.isIndConstId(varDecl->id))
                 && !preferProps) {
                varDecl->child(0) = Node::bit_ty;
            } else {
                varDecl->child(0) = Node::bool_ty;
            }
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Work over function decls
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


    for (map<string,Node*>::iterator i = ctxt->funMap.begin();
         i != ctxt->funMap.end();
         i++) {

        Node* funDecl = i->second;  // DECL_FUN {id} (SEQ type+) type

        // Convert all uses of Bool as arg type to Bit.
        // (Any aliases for Bool are converted later by fixing alias typedefs)

        funDecl->child(0)->mapOver1(boolToBit);

        Node* baseRangeType = ctxt->normaliseType(funDecl->child(1));
        if (baseRangeType->kind == BOOL_TY) {

            if ( (preferBitVals || indIds.isIndFunId(funDecl->id))
                 && !preferProps) {

                funDecl->child(1) = Node::bit_ty;
            } else {
                funDecl->child(1) = Node::bool_ty;
            }
        }
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// fixAtomicFmlas
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 

// 1. Change FUN_APs of bool-valued functions to PRED_APs
// 2. Change bool-valued CONSTS to nullary PRED_APs.
// 
Node* fixAtomicFmlas(FDLContext* c, Node* n) {

    if (n->kind == FUN_AP || n->kind == CONST) {
        
        Node* nType = c->getType(n);

        if (nType->kind == BOOL_TY) {

            n->kind = PRED_AP;
        }
    }
    return n;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// addBitToProp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Node* addBitToProp(FDLContext* c, Node* n) {

    vector<Node**> subNodes = n->getSubNodes();
    Node* subNodeTypes = c->getExactSubNodeTypes(n);

    for (int i = 0; i != (int) subNodes.size(); i++) {
        Node** subNode = subNodes.at(i);
        Node* actualType = c->normaliseType(c->getType(*subNode));
        Node* expectedType = c->normaliseType(subNodeTypes->child(i));

        if (expectedType->kind == BOOL_TY
            && actualType->kind == BIT_TY) {

            printMessage(FINERm, "Adding Bit to Prop coercion for"
                         + ENDLs + (*subNode)->toString());

            *subNode = new Node(TO_PROP, *subNode);
        }
    }
    return n;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// addPropToBit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Node* addPropToBit(FDLContext* c, Node* n) {

    vector<Node**> subNodes = n->getSubNodes();
    Node* subNodeTypes = c->getExactSubNodeTypes(n);

    for (int i = 0; i != (int) subNodes.size(); i++) {
        Node** subNode = subNodes.at(i);
        Node* actualType = c->normaliseType(c->getType(*subNode));
        Node* expectedType = c->normaliseType(subNodeTypes->child(i));

        if (expectedType->kind == BIT_TY
            && actualType->kind == BOOL_TY) {

            // Only trace non-trivial coercions.
            // Trivial ones are expanded away when TO_PROP expanded.

            if ( ! ( (*subNode)->kind == TRUE
                     || (*subNode)->kind == FALSE
                     )
                 ) {
                printOnOption("trace-prop-to-bit-insertion",
                              "Adding Prop to Bit coercion for"
                              + ENDLs + (*subNode)->toString());

                printMessage(FINERm, "Adding Prop to Bit coercion for"
                             + ENDLs + (*subNode)->toString());
            }

            *subNode = new Node(TO_BIT, *subNode);
        }
    }
    return n;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// introBitType
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Main function

void
introBitType (FDLContext* ctxt, Node* unit) {

    if (!option("bit-type-with-ite")) {
        mapOverWithContext(introBitOpsAndRel, ctxt, unit);
    }

    boolTypeParamsToBit(ctxt, unit);

    introBitFixFunConstDecls(ctxt, unit);
    

    mapOverWithContext(fixAtomicFmlas, ctxt, unit);
    mapOverWithContext(addBitToProp, ctxt, unit);

    if (option("bit-type-with-ite")) {
        mapOverWithContext(addPropToBit, ctxt, unit);
    }
    ctxt->removeFeature("QFOL");
    ctxt->addFeature("FOL");
}


