//========================================================================
//========================================================================
// CONTEXT.CC
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



#include <map>
using std::map;
using std::make_pair;
using std::pair;

#include <stdexcept>

#include <set>
using std::set;

#include "context.hh"

#include "utility.hh"

#include "typesort.hh"


//============================================================================
// Context functions
//============================================================================


FDLContext::FDLContext(Node* FDL_AST) : 
    typeSeq(SEQ, UNMANAGED),   // Initialisers to make sure Nodes are outside pool
    termSeq(SEQ, UNMANAGED),
    bindings(SEQ, UNMANAGED)
    {
    for (int i = 0;
         i != FDL_AST->arity();
         i++) {
        insert(FDL_AST->child(i));
    }

    // Do topological sort on decls to ensure def before use.

    try {

        Node* sortResult = TypeSort::sort(&typeSeq);
        
        typeSeq = * (sortResult->child(0));

        // Make sure new declarations find their way into the type map.
        
        for (int i = 0; i != sortResult->child(1)->arity(); i++) {

            Node* newDecl = sortResult->child(1)->child(i);
            typeMap.insert(make_pair(newDecl->id, newDecl));
        }

    } catch (std::runtime_error e) {
        printMessage(ERRORm,
                     string("Topological sort of type decls failed\n")
                     + e.what()
                     );
    }

}

// NB: If ever modify this to merge new and existing bindings, should then
// check the effect of the return val on callers.

bool mapInsertWithCheck(const char* mapName,
                        map<string,Node*>& m,  // Use & to modify arg, not copy
                        const string& key,
                        Node* val) {

    bool success = m.insert(make_pair(key, val)).second;
    if (!success) {
        Node* mapVal = m.find(key)->second;
        if (val->equals(mapVal)) {
            printMessage(INFOm,
                         "Rejected repeated entry for " + key
                         + " in map " + mapName);
        } else {
                printMessage(ERRORm,
                             "New value for " + key
                             + " in map " + mapName + 
                             +  "conflicts with existing value." +  ENDLs
                             + "Existing value: " + ENDLs
                             + mapVal->toString() + ENDLs
                             + "New value: " + ENDLs
                             + val->toString() + ENDLs
                    );
        }
    }
    return success;
}


void
FDLContext::insert(Node* decl, bool atEnd) {


    switch (decl->kind) {
    case DEF_TYPE: {

        if (decl->arity() != 0) {
            Node* ty = decl->child(0);
            if (ty->kind == ENUM_TY
                || ty->kind == ARRAY_TY
                || ty->kind == RECORD_TY) {
                
                ty->id = decl->id;
            }
        }

        bool success = mapInsertWithCheck("typeMap",
                                          typeMap,
                                          decl->id,
                                          decl);
        if (!success) break;
        
        if (atEnd) {
            typeSeq.addChild(decl);
        } else {
            typeSeq.addLeftChild(decl);
        }
        if (decl->arity() == 0) break;

        Node* ty = decl->child(0);

        if (ty->kind == ENUM_TY) {

            Node* enumTy = ty;
            
            
            for ( int i = 0; 
                  i!= enumTy->arity();
                  i++) {
                Node* enumId = enumTy->child(i);
                // Ensure new type id constructed each time round
                Node* namedEnumTy = new Node(TYPE_ID, decl->id);
                Node* enumConstDecl = new Node (DEF_CONST,
                                                enumId->id,
                                                namedEnumTy);
                              
                enumConstMap.insert(make_pair(enumId->id, enumConstDecl));
            }
        }
        else if (ty->kind == RECORD_TY) {
            Node* recordTy = ty;
            
            Node* recordTyId = new Node(TYPE_ID, decl->id);

            for ( int i = 0; 
                  i!= recordTy->arity();
                  i++) {
                Node* fieldDecl = recordTy->child(i); // DECL{fname}(type)
                // If fname already in map, need to update val to UNKNOWN()

                // Try adding fname, recType pair to record field map
                pair < map<string, Node*>::iterator, bool > res = 
                    recordFieldMap.insert(make_pair(fieldDecl->id,recordTyId));
                if (!res.second) {
                    // If already entry in map for fieldDecl->id,
                    // update corresponding value to UNKNOWN() to flag
                    // that type cannot be unambiguously figured out
                    // from fieldname.
                    (res.first)->second = Node::unknown;
                }
            } // End for
        } // End if
        break;
    }
    case DEF_CONST:  {
        bool success =
            mapInsertWithCheck("constMap", constMap, decl->id, decl);
        if (success) {
            termSeq.addChild(decl);
        }
        break;
    }
    case DECL_VAR:   {
        decl->kind = DEF_CONST;
        Node* declI = decl->copy();  // Initial value variant of var
        declI->id = decl->id + "___init";
        Node* declL = decl->copy();  // loop start value variant of var
        declL->id = decl->id + "___loopinit";
        
        bool success1 =
            mapInsertWithCheck("constMap", constMap, decl->id, decl);
        bool success2 =
            mapInsertWithCheck("constMap", constMap, declI->id, declI);
        bool success3 =
            mapInsertWithCheck("constMap", constMap, declL->id, declL);
        
        //        varMap.  insert(make_pair(decl->id, decl));
        
        if (success1) termSeq.addChild(decl);
        if (success2) termSeq.addChild(declI);
        if (success3) termSeq.addChild(declL);
        
        break;
    }
    case DECL_FUN:  {
        bool success = mapInsertWithCheck("funMap", funMap, decl->id, decl);
        if (success) termSeq.addChild(decl);
        break;
    }
    default: assert(false); 
    }            
    return;
}

Node* 
FDLContext::lookupType(const string& s) {
        map<string, Node*>::iterator i = typeMap.find(s);
        if (i == typeMap.end())
            return 0;
        else
            return i->second;
}



Node* 
FDLContext::lookupConst(const string& s) {
        map<string, Node*>::iterator i = constMap.find(s);
        if (i == constMap.end())
            return 0;
        else
            return i->second;
}

Node* 
FDLContext::lookupVar(const string& s) {
        map<string, Node*>::iterator i = varMap.find(s);
        if (i == varMap.end())
            return 0;
        else
            return i->second;
}

Node* 
FDLContext::lookupFun(const string& s) {
        map<string, Node*>::iterator i = funMap.find(s);
        if (i == funMap.end())
            return 0;
        else
            return i->second;
}

Node* 
FDLContext::lookupEnumConst(const string& s) {
        map<string, Node*>::iterator i = enumConstMap.find(s);
        if (i == enumConstMap.end())
            return 0;
        else
            return i->second;
}

Node* 
FDLContext::lookupRecordField(const string& s) {
        map<string, Node*>::iterator i = recordFieldMap.find(s);
        if (i == recordFieldMap.end())
            return Node::unknown;
        else
            return i->second;
}


// Normalise top level of type. 

// Returns PENDING when type declaration missing.
// [Maybe push this error-case behaviour back into lookupType??]

Node* 
FDLContext::normaliseType(Node* n) {

    if (n->kind == TYPE_ID || n->kind == TYPE_PARAM) {

        Node* typeDecl = lookupType(n->id);

        if (typeDecl == 0) {
            printMessage(ERRORm,
                         "normaliseType: encountered undeclared type: "
                         + n->id);
            return new Node(PENDING);
        }
        else if (typeDecl->arity() == 0) {
            if (n->kind == TYPE_ID)
                return n;
            else  // n->kind == TYPE_PARAM
                return new Node(TYPE_ID, n->id);
        }
	else {
	  return normaliseType(typeDecl->child(0));
        }
    } 
    return n;  // If not TYPE_ID
}

// Put type into canonical (unique) form.  Could be TYPE_ID if type is
// named type for ENUM type, ARRAY or RECORD type.

Node* 
FDLContext::canoniseType(Node* n) {

    if (n->kind == TYPE_ID || n->kind == TYPE_PARAM) {

        Node* typeDecl = lookupType(n->id);

        if (typeDecl == 0) {
            printMessage(ERRORm,
                         "canoniseType: encountered undeclared type: "
                         + n->id);
            return new Node(PENDING);
        }
        else if (typeDecl->arity() == 0

                    // typeDecl->arity() == 1 if we get to these tests
                 || typeDecl->child(0)->kind == ARRAY_TY
                 || typeDecl->child(0)->kind == RECORD_TY
                 || typeDecl->child(0)->kind == ENUM_TY) {

            if (n->kind == TYPE_ID)
                return n;
            
            else  // n->kind == TYPE_PARAM

                return new Node(TYPE_ID, n->id);
        } 
	else {
            return canoniseType(typeDecl->child(0));
        }
    } 
    return n;  // If not TYPE_ID
}


void
FDLContext::pushBinding(Node* decl) {bindings.addChild(decl);}

void
FDLContext::popBinding() {bindings.popChild();}

// Lookup starting from end.
Node* 
FDLContext::lookupBinding(const string& s) {
    for (int i = bindings.arity(); i != 0; ) {
        i--;
        Node* decl = bindings.child(i);
        if (s == decl->id) return decl;
    }
    return 0;
}

void 
FDLContext::pushPathStep(z::Kind k, const string& s, int i) {
    pathKinds.push_back(k);
    pathIds.push_back(s);
    pathAddr.push_back(i);
    return;
}

void 
FDLContext::popPathStep() {
    pathKinds.pop_back();
    pathIds.pop_back();
    pathAddr.pop_back();
    return;
}

string 
FDLContext::getPathString() {

    string s;
    for (int i = 0; i != (int) pathAddr.size(); i++) {
        s += kindString(pathKinds.at(i));
        if ( (int) pathIds.at(i).size() > 0) {
            s += "{" + pathIds.at(i) + "}";
        }
        s += "." + intToString(pathAddr.at(i)) + ".";
    }
    return s;
}





// Id = const, enum const, var or bound var.

Node* 
FDLContext::lookupId(const string& s) {   
    Node* n;
    n = lookupBinding(s);
    if (n != 0) return n;
    n = lookupConst(s);
    if (n != 0) return n;
    n = lookupEnumConst(s);
    if (n != 0) return n;
    return lookupVar(s);
}

Node* 
FDLContext::lookupConstId(const string& s) {   
    Node* n;
    n = lookupConst(s);
    if (n != 0) return n;
    n = lookupEnumConst(s);
    if (n != 0) return n;
    return lookupVar(s);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// getType
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Returns Node(UNKNOWN) if fails.  Never returns 0.


Node* FDLContext::getType (Node* n) {

    switch(n->kind) {

    case TYPE_PARAM:
    case TYPE_ID:
    case INT_TY:
    case REAL_TY:
    case BOOL_TY:
    case BIT_TY:
    case BITVEC_TY:
    case SUBRANGE_TY:
    case ENUM_TY:
    case ARRAY_TY:
    case RECORD_TY:
    case FUN_TY:     // Yices only
    case TUPLE_TY:
        return Node::type_univ;
        
    case UNIT:
    case DECLS:
    case RULES:
    case RULE:
    case GOALS:
    case GOAL:
    case HYPS:
    case HYP:
    case CONCLS:

    case FORALL:
    case EXISTS:
    case IFF: 
    case IMPLIES:
    case AND:
    case OR:
    case NOT:
    case TRUE:
    case FALSE:
    case EQ:
    case NE:
    case LT :
    case GT:
    case LE:
    case I_LT:
    case I_LE:
    case R_LT:
    case R_LE:
    case IR_EQ:
    case IR_LT:
    case IR_LE:
    case GE:
    case PRED_AP:
    case TO_PROP:
        return Node::bool_ty;

    case TO_BIT:
    case TERM_TRUE:
    case TERM_FALSE:
    case TERM_AND:
    case TERM_OR:
    case TERM_NOT:
    case TERM_EQ:
    case TERM_NE:
    case TERM_I_LT:
    case TERM_I_LE:
    case TERM_R_LT:
    case TERM_R_LE:
        return Node::bit_ty;

    case UMINUS:
    case PLUS:
    case MINUS:
    case TIMES:
    case EXP: 
        return Node::int_or_real_ty;

    case I_UMINUS:
    case I_SUCC:
    case I_PRED:
    case I_PLUS:
    case I_MINUS:
    case I_TIMES:
    case IDIV:
    case MOD: 
    case IDIV_E:
    case MOD_E: 
    case I_EXP:
    case I_SQR:
    case I_ABS:
        return Node::int_ty;
    case NATNUM:
        return new Node(SUBRANGE_TY, n->copy(), n->copy());
    case TO_REAL:
    case R_UMINUS:
    case R_PLUS:
    case R_MINUS:
    case R_TIMES:
    case R_EXP: 
    case R_SQR:
    case R_ABS:
    case RDIV:
    case REALNUM:
        return Node::real_ty;
        
    case ID: {
        string idName = n->id;
        Node* decl = lookupId(idName);
        if (decl != 0) {
            // DECL{id} type
            // DEF_CONST {id} type [exp]
            // DECL_VAR {id} type
            // DEF_CONST {id} enumType
            return decl->child(0);
        }
        // Unbound upper-case ids are expected in some rules
        // Unbound lower-case ids are not expected.
        if (! ('A' <= idName[0] && idName[0] <= 'Z')) {
            printMessage(INFOm,
                         "getType: encountered unexpected lower-case id: "
                         + idName);
        }
        return Node::unknown;
    }

    case CONST: {
        string idName = n->id;
        Node* decl = lookupConstId(idName);
        if (decl != 0) {
            // DEF_CONST {id} type [exp]
            // DECL_VAR {id} type
            // DEF_CONST {id} enumType
            return decl->child(0);
        } else {
            printMessage(INFOm,
                         "getType: encountered undeclared constant: "
                         + idName);
            return Node::unknown;
        }
    }

    case VAR: {
        string idName = n->id;
        Node* decl = lookupBinding(idName);
        if (decl != 0) {
            // DECL{id} type
            return decl->child(0);
        } else {
            printMessage(INFOm,
                         "getType: encountered unbound variable: "
                         + idName);
            return Node::unknown;
        }
    }


    case TUPLE: {
        return Node::unknown;
        /*
        Node* type = new Node(TUPLE_TY);
        for (int i = 0; i != n->arity(); i++) {
            Node* childType = getType(n->child(i));
        }
        */  
    }
    case FUN_AP: {
        string funName = n->id;
        Node* funDecl = lookupFun(funName);
        if (funDecl != 0) {  // DECL_FUN {id} (SEQ type+) type 
            return funDecl->child(1);
        }
        else {
            printMessage(INFOm,
                         "getType: Encountered unexpected function: "
                         + funName);
            return Node::unknown;
        }
    }

    case ARR_ELEMENT: {

        Node* arrTy;
        if (n->id.size() > 0) {
            arrTy = lookupType(n->id)->child(0);
        }
        else if (strictTyping) {
            arrTy = 0;
        }
        else {
            arrTy = normaliseType(getType(n->child(0)));
        }

        if (arrTy != 0) {
            if (arrTy->kind == ARRAY_TY)
                return arrTy->child(1);
            else {
                printMessage(INFOm,
                             "getType: array has unexpected type " +
                             kindString(arrTy->kind));
                return Node::unknown;
            }
            
        } else {
            return Node::unknown;
        }
    }
        
    case ARR_UPDATE: {
        if (n->id.size() > 0) {
            return new Node(TYPE_ID, n->id);
        }
        else if (strictTyping) {
            return Node::unknown;
        }
        else {
            return getType (n->child(0));
        }
    }

        
    case RCD_ELEMENT: { 
        // RCD_ELEMENT{rcd-id} exp
        // RCD_ELEMENT{rcd-id} exp (TYPE_PARAM{type-id})

        Node* rcdTy;

        if (n->arity() == 2) {
            rcdTy = lookupType(n->child(1)->id)->child(0);
        }
        else if (strictTyping) {
            rcdTy = 0;
        }
        else {
            rcdTy = normaliseType(getType(n->child(0)));
        }
        if (rcdTy == 0)
            return Node::unknown;

        if (rcdTy->kind != RECORD_TY) {
            printMessage(INFOm,
                         "getType: record has unexpected type " +
                          kindString(rcdTy->kind));
            return Node::unknown;
        }

        string fieldName = n->id;
        
        for (int i = 0; i != rcdTy->arity(); i++ ) {
             Node* decl = rcdTy->child(i);
             if ( decl->id == fieldName) {
                 return decl->child(0);
             }
        }
        printMessage(INFOm,
                     "getType: attempt to access field " + fieldName
                     + " of record " + (rcdTy->id));
        return Node::unknown;
    }
        
    case RCD_UPDATE: {
        // RCD_UPDATE{rcd-id} exp val
        // RCD_UPDATE{rcd-id} exp val (TYPE_PARAM{type-id})

        if (n->arity() == 3) {
            return new Node(TYPE_ID, n->child(2)->id);
        }
        else if (strictTyping) {
            return Node::unknown;
        }
        else {
            return getType (n->child(0));
        }
    }

    case MK_ARRAY:
    case MK_RECORD:
        return new Node(TYPE_ID, n->id);

    case ITE:
        return n->child(3);
    default:
        return Node::unknown;
    }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// getSubNodeTypes
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Infer types of subexpressions.

// Always returns a SEQ node with arity same as argument node.  If
// inference of child type fails placeholder UNKNOWN Nodes is returned
// as child's type.

// if getOptions = true, when relevant, return special symbols for
// known sets of alternatives e.g. "int or real" & "int or real or
// unknown".  This is useful at the stage of inferring types for free
// variables in user rules.

// if getOptions = false, always return the exact expected type, if possible.
// This case is used for e.g. full type checking.

Node*
FDLContext::getSubNodeTypes (Node* n, bool getOptions) {

    Node* result = new Node(SEQ);

    // Cases when getOptions makes no difference can return result.
    // If case has found types, case should modify result and break.
    // Otherwise case should leave result alone and break.  In this situation,
    // the children types are filled in as UNKNOWN_TYPEs.

    switch(n->kind) {
    case FORALL:
    case EXISTS:
        {
        // FORALL (SEQ d1 ... nk) body
        // EXISTS (SEQ d1 ... nk) body
        //   where di = DECL{id} type

            for (int i = 0; i != n->child(0)->arity(); i++) {

                result->addChild(Node::type_univ);
            }
            result->addChild(Node::bool_ty);
            return result;
        }
    case NOT:
    case TO_BIT:
        {
            result->addChild(Node::bool_ty);
            return result;
        }
    case IFF: 
    case IMPLIES:
        {
            result->addChild(Node::bool_ty);
            result->addChild(Node::bool_ty);
            return result;
        }

    case UNIT:
    case RULES:
    case RULE:
    case GOALS:
    case GOAL:
    case HYPS:
    case HYP:
    case CONCLS:
        
    case AND:
    case OR: 
        {
            for (int i = 0; i != n->arity(); i++) {
                result->addChild(Node::bool_ty);
            }
            return result;
        }
    
    case TO_PROP:
    case TERM_AND:
    case TERM_OR:
    case TERM_NOT:
        {
            for (int i = 0; i != n->arity(); i++) {
                result->addChild(Node::bit_ty);
            }
            return result;
        }

    case TERM_EQ:
    case TERM_NE:
    case EQ:
    case NE:
        {
            if (n->arity() == 3) {
                result->addChild(n->child(2));
                result->addChild(n->child(2));
                result->addChild(Node::type_univ);
                return result;
            } else {
                result->addChild(Node::unknown);
                result->addChild(Node::unknown);
                return result;
            }
        }
    case TRUE:
    case FALSE:
    case NATNUM:
    case REALNUM:
    case ID:
        return result;
    case LE:
    case LT:
        {
            result->addChild(Node::int_real_or_enum_ty);
            result->addChild(Node::int_real_or_enum_ty);
            return result;
        }
    case SUCC:
    case PRED:
        {
            result->addChild(Node::int_real_or_enum_ty);
            return result;
        }
    case UMINUS:
        {
            result->addChild(Node::int_or_real_ty);
            return result;
        }
    case PLUS:
    case MINUS:
    case TIMES:
    case IR_EQ:
    case IR_LT:
    case IR_LE:
        {
            result->addChild(Node::int_or_real_ty);
            result->addChild(Node::int_or_real_ty);
            return result;
        }

    case TERM_I_LT:
    case TERM_I_LE:
    case I_LT:
    case I_LE:
    case I_PLUS:
    case I_MINUS:
    case I_TIMES:
    case IDIV:
    case MOD: 
    case IDIV_E:
    case MOD_E: 
    case I_EXP:
        {
            result->addChild(Node::int_ty);
            result->addChild(Node::int_ty);
            return result;
        }
    case TO_REAL:
    case I_UMINUS:
    case I_SUCC:
    case I_PRED:
    case ODD:
    case I_ABS:
    case I_SQR:
        {
            result->addChild(Node::int_ty);
            return result;
        }
    case TERM_R_LT:
    case TERM_R_LE:
    case R_LT:
    case R_LE:
    case R_PLUS:
    case R_MINUS:
    case R_TIMES:
    case RDIV:
        {
            result->addChild(Node::real_ty);
            result->addChild(Node::real_ty);
            break;
        }
    case R_EXP:
        {
            result->addChild(Node::real_ty);
            result->addChild(Node::int_ty);
            break;
        }
    case R_ABS:
    case R_SQR:
    case R_UMINUS:
        {
            result->addChild(Node::real_ty);
            break;
        }
    case EXP:
        {
            result->addChild(Node::int_or_real_ty);
            result->addChild(Node::int_ty);
            return result;
        }
    case PRED_AP:
    case FUN_AP:
        {
            if (n->arity() == 0) {  // propositional variable
                return result;
            }
            string funName = n->id;
            Node* funDecl = lookupFun(funName);
            if (funDecl != 0) {  // DECL_FUN {id} (SEQ type+) type 
                result->appendChildren(funDecl->child(0));
                break;
            }
            printMessage(INFOm,
                         "getSubNodeTypes: Encountered unexpected function: "
                         + funName
                         );
            break;
        }
        
    case ARR_ELEMENT: // ARR_ELEMENT{<type>?} array (SEQ i1 ... ik)
        {
            Node* arrayTy = 0;
            if (n->id != "") {
                arrayTy = nameToType(n->id);
            }
            else if (strictTyping)
                break;
            else {
                arrayTy = getType(n->child(0));
            }

            Node* normalisedArrayTy = normaliseType(arrayTy);

            if (normalisedArrayTy->kind != ARRAY_TY) break;

            // normalisedArrayTy = ARRAY_TY (SEQ T1 ...Tn) T

            result->addChild(arrayTy);
            result->appendChildren(normalisedArrayTy->child(0));
            break;
        }
    case ARR_UPDATE:  // ARR_UPDATE{<type>?} array (SEQ i1 ...ik) value
        {
            Node* arrayTy = 0;
            if (n->id != "") {
                arrayTy = nameToType(n->id);
            }
            else if (strictTyping)
                break;
            else {
                arrayTy = getType(n->child(0));
            }
            Node* normalisedArrayTy = normaliseType(arrayTy);

            if (normalisedArrayTy->kind != ARRAY_TY) break;

            // normalisedArrayTy = ARRAY_TY (SEQ T1 ...Tn) T

            result->addChild(arrayTy);
            result->appendChildren(normalisedArrayTy->child(0));
            result->addChild(normalisedArrayTy->child(1));
            break;
        }

    case ARR_BOX_UPDATE:  // ARR_BOX_UPDATE{<type>?} array (SEQ r1 ...rk) value
                          // rj = SUBRANGE i1 i2
        {
            Node* arrayTy = 0;
            if (n->id != "") {
                arrayTy = nameToType(n->id);
            }
            else if (strictTyping)
                break;
            else {
                arrayTy = getType(n->child(0));
            }
            Node* normalisedArrayTy = normaliseType(arrayTy);

            if (normalisedArrayTy->kind != ARRAY_TY) break;

            // normalisedArrayTy = ARRAY_TY (SEQ T1 ...Tn) T

            Node* indexTys = normalisedArrayTy->child(0);
            Node* valTy = normalisedArrayTy->child(1);

            result->addChild(arrayTy);
            for (int j = 0; j != indexTys->arity(); j++) {
                result->addChild(indexTys->child(j));
                result->addChild(indexTys->child(j));
            }
            result->addChild(valTy);
            break;
        }

    case MK_ARRAY:
        {
            // MK_ARRAY{arrname} default a1 ... an, n >= 0
            // MK_ARRAY{arrname} a1 ... an,         n >= 1
            //   where ai = ASSIGN (SEQ i1 ... im) val,  m >= 1
            //          i = e | SUBRANGE e1 e2

            Node* typeDecl = lookupType(n->id);
            Node* arrTy = typeDecl->child(0);
            Node* indexTys = arrTy->child(0);
            Node* valTy = arrTy->child(1);
            
            for (int i = 0; i != n->arity(); i++) {
                Node* c = n->child(i);
                if (c->kind != ASSIGN) {
                    result->addChild(valTy);
                    continue;
                }
                Node* indexList = c->child(0);
                for (int j = 0; j != indexList->arity(); j++) {
                    Node* indexTy = indexTys->child(j);
                    
                    if (indexList->child(j)->kind == SUBRANGE) {
                        
                        result->addChild(indexTy);
                        result->addChild(indexTy);
                    }
                    else {
                        result->addChild(indexTy);
                    }
                }
                result->addChild(valTy);
            }

            break;
        }
    case RCD_ELEMENT: // RCD_ELEMENT{field} record type?
        {
            Node* recordTy;
            if (n->arity() == 2)
                recordTy = n->child(1);
            else if (strictTyping)
                break;
            else
                recordTy = getType(n->child(0));

            if (normaliseType(recordTy)->kind != RECORD_TY) break;

            result->addChild(recordTy);
            if (n->arity() == 2)
                result->addChild(Node::type_univ);
            break;
        }
    case RCD_UPDATE:  // RCD_UPDATE{field} record value type?
        {
            Node* recordTy;
            if (n->arity() == 3)
                recordTy = n->child(2);
            else if (strictTyping)
                break;
            else
                recordTy = getType(n->child(0));

            Node* normRecordTy = normaliseType(recordTy);

            if (normRecordTy->kind != RECORD_TY) {
                printMessage(INFOm,
                             "getSubNodeTypes: record has unexpected type " +
                             kindString(normRecordTy->kind));
                break;
            }

            string fieldName = n->id;
            Node* fieldType = 0;

            for (int i = 0; i != normRecordTy->arity(); i++ ) {
                Node* decl = normRecordTy->child(i);
                if ( decl->id == fieldName) {
                    fieldType = decl->child(0);
                    break;
                }
            }
            if (fieldType == 0) {
                printMessage(INFOm,
                             "getSubNodeTypes: attempt to access field "
                             + fieldName
                             + " of record " + (normRecordTy->id));
                fieldType = Node::unknown;
            }

            result->addChild(recordTy);
            result->addChild(fieldType);

            if (n->arity() == 3)
                result->addChild(Node::type_univ);
            break;
        }
    case MK_RECORD:
        {
            // MK_RECORD{rcdname} a1 ... an, n >= 1
            //   where ai = ASSIGN{fldname} val

            string rcdName = n->id;
            Node* typeDecl = lookupType(rcdName);
            Node* rcdTy = typeDecl->child(0);
            for (int i = 0; i != n->arity(); i++) {
                Node* assign = n->child(i);
                string fldName = assign->id;
                // Search for corresponding DECL in type
                Node* fldTy = 0;
                for (int j = 0; j != rcdTy->arity(); j++) {
                    if (rcdTy->child(j)->id == fldName) {
                        fldTy = rcdTy->child(j)->child(0);
                        break;
                    }
                }
                if (fldTy == 0) {
                    printMessage(INFOm,
                                 "getSubNodeTypes: can't find type of "
                                 + fldName + " field of "
                                 + rcdName + " record type");
                    fldTy = Node::unknown;
                }
                result->addChild(fldTy);
            }
            break;
        }
    case ITE: {
        result->addChild(Node::bool_ty);
        result->addChild(n->child(3));
        result->addChild(n->child(3));
        result->addChild(Node::type_univ);
        break;
    }
    default:
        break;
    }

    if (result->arity() == 0) {
        // Case for errors and unknowns

        int numSubNodes = n->getSubNodes().size();
    
        for (int i = 0; i != numSubNodes; i++) {
            result->addChild(Node::unknown);
        }
    }
    else if (getOptions) {
        // Modify all real types to real_or_int

        for (int i = 0; i != result->arity(); i++) {
            if (result->child(i)->kind == REAL_TY)
                result->child(i) = Node::int_or_real_ty;
        }
    }
    return result;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// extractDecls
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Form list of all declarations from contents of tables

// Ensure that types are added in the stored order which respects 
// dependencies.  

// consts, vars and funs are ordered alphabetically.

Node* 
FDLContext::extractDecls() {

    Node* decls = new Node(DECLS);

    for (int j = 0; j != typeSeq.arity(); j++) {
        decls->addChild(typeSeq.child(j));
    }
    map<string,Node*>::iterator i;
    for (i = constMap.begin(); i != constMap.end(); i++) 
        decls->addChild(i->second);
    for (i = varMap.begin(); i != varMap.end(); i++) 
        decls->addChild(i->second);
    for (i = funMap.begin(); i != funMap.end(); i++) 
        decls->addChild(i->second);

    return decls;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Feature methods
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void 
FDLContext::addFeature(const string& s) {
    features.insert(s);
    return;
}

void 
FDLContext::removeFeature(const string& s) {
    features.erase(s);
    return;

}

bool
FDLContext::hasFeature(const string& s) {
    return features.find(s) != features.end();
}
