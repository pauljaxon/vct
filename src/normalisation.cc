//========================================================================
//========================================================================
// NORMALISATION.CC
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

// Put FDL into Standard Form


#include <map>
using std::map;
using std::make_pair;
using std::pair;

#include <stdexcept>

#include <algorithm>  // For swap.
using std::swap;

#include <set>
using std::set;

#include "normalisation.hh"

#include "utility.hh"
#include "node-utils.hh"
#include "formatter.hh"

using std::string;

using namespace z;



//========================================================================
// Regularise Unit Structure
//========================================================================
// Split off declarations into maps for each decl kind.
// Turn rules into formulas.
// Update top-level unit structure.

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// normaliseRule
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* Generate equivalent formula for rule.  

MAY_BE_REPLACED_BY e1 e2   --> EQ e1 e2
MAY_BE_REPLACED_BY e1 e2 (SEQ es)  --> IMPLIES (AND es) (EQ e1 e2)

ARE_INTERCHANGEABLE e1 e2  --> EQ e1 e2
ARE_INTERCHANGEABLE e1 e2 (SEQ es)  --> IMPLIES (AND es) (EQ e1 e2)

MAY_BE_DEDUCED e   --> e
MAY_BE_DEDUCED e (SEQ es)  --> IMPLIES (AND es) e

If any of preconditions are FUN_AP{goal} (FUN_AP{checktype} 
                                            ID{a}
                                            ID{t} )
then strip out as DECl{a} nameToType(t).
  (nameToType converts most ids to TYPE_ID, except for for built-in
   types integer, real, boolean, bit___type, where it returns the special
   operator)

If any are FUN_AP{goal} anything else,
then strip out completely and flag error.
    
If stripped out decls are non-empty, add them on to final formula
in a universal quantifier.

*/
    

Node* normaliseRule(Node* rule) {

    // Target structure is 

    //   FORALL decls (IMPLIES (AND antecedents) consequent)

    // where FORALL, IMPLIES and AND are omitted when not needed.


    // Extract out rule preconditions and consequent

    Node* rulePreconditions;
    Node* consequent;
    
    switch(rule->kind) {

    case MAY_BE_REPLACED_BY: 
    case ARE_INTERCHANGEABLE: {
        if (rule->arity() == 2) {
            rulePreconditions = new Node(SEQ);
        }
        else { // arity must be 3.
            rulePreconditions = rule->child(2);
            rule->popChild();
        }
        consequent = rule->updateKind(EQ);
        break;
    }
    case MAY_BE_DEDUCED: {
        if (rule->arity() == 1) {
            rulePreconditions = new Node(SEQ);
        }
        else { // arity must be 2
            rulePreconditions = rule->child(1);
        }
        consequent = rule->child(0);
        break;
    }
    default:
        assert(false);
        return 0;
    } // END switch

    // Process preconditions

    Node* decls = new Node(SEQ);
    Node* antecedents = new Node(AND);

    for (int i = 0; i != rulePreconditions->arity(); i++) {
        Node* p = rulePreconditions->child(i);

        if ( p->kind == FUN_AP && p->id == "goal") {
            Node* prologCmd = p->child(0);

            if (  prologCmd->kind == FUN_AP
                  && prologCmd->id == "checktype"
                  && prologCmd->child(0)->kind == ID
                  && prologCmd->child(1)->kind == ID) {

                string id = prologCmd->child(0)->id;
                Node* type = nameToType(prologCmd->child(1)->id);
                              
                decls->addChild(new Node (DECL, id, type));

            } else {
                printMessage(ERRORm, 
                         "normaliseRule: unexpected Prolog precondition"
                             + ENDLs
                             + prologCmd->toString()
                             );
            }
        } else {
            antecedents->addChild(p);
        }
    }

    // Build rule formula
    Node* body =
        antecedents->arity() == 0
        ? consequent
        : antecedents->arity() == 1
          ? new Node (IMPLIES, antecedents->child(0), consequent)
          : // antecedents->arity() > 1
            new Node (IMPLIES, antecedents, consequent)
        ;

    return
        decls->arity() == 0
        ? body
        : new Node (FORALL, decls, body)
        ;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// flattenRules
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Flatten grouping of rules into rule families.
//  o Ignore type assumption information in each family.
//  o Replace each special rule form by appropriate logical expression.



void
flattenRules(Node* RLS_AST) {

    // -------------------------------------------------------------------
    // Process RLS file
    // -------------------------------------------------------------------

    Node* result = new Node(RULES);
    
    // Loop over rule families
    for (int i = 0;
         i != RLS_AST->arity();
         i++) {

        Node* rFam = RLS_AST->child(i);
        Node* rules = rFam->child(1);

        // Loop over rules in family
        for (int j = 0;
             j != rules->arity();
             j++) {

            Node* rule = rules->child(j);

            result->addChild(normaliseRule(rule));
        }
    }
    // Destructively modify initial top Node object.
    * RLS_AST = * result;
    return;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RegulariseUnitStructure
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 

void fixFDLVar(Node* n) {
    if (n->kind == ID) {
        if (hasSuffix(n->id,"~")) {
            n->id = fixSuffix(n->id,
                              "~",  // old suffix
                              "",   // new prefix
                              "___init");
        } else 
        if (hasSuffix(n->id,"%")) {
            n->id = fixSuffix(n->id,
                              "%",  // old suffix
                              "",   // new prefix
                              "___loopinit");
        }
    }
    return;
}

FDLContext* RegulariseUnitStructure(Node* unit) {

    FDLContext* ctxt = new FDLContext(unit->child(0));

    unit->child(0)->clearChildren();         // Delete context part of unit.

    flattenRules(unit->child(1));

    unit->child(0)->kind = DECLS;
    unit->child(2)->kind = GOALS;

    unit->mapOver(fixFDLVar); 
    return ctxt;
}


//========================================================================
// Augment const and var declarations
//========================================================================


void augmentConstDecls(FDLContext* ctxt, Node* unit) {

    // -------------------------------------------------------------------
    // Add X__base__first and X__base__last constants if not declared.
    // -------------------------------------------------------------------

    map<string, Node*> oldConstMap(ctxt->constMap);// Make reference copy of 
                                                   // constMap

    for (map<string,Node*>::iterator i = oldConstMap.begin();
         i != oldConstMap.end();
         i++) {

        const char* sufs[] = {"__first", "__last"};

        for (int j = 0 ; j != 2; j++) {
            string suf(sufs[j]);
            
            if (hasSuffix(i->first, suf)
                && ! hasSuffix(i->first, "__base" + suf)) {
                
                string newId = i->first;
                newId.erase(newId.size() - suf.size());
                newId.append("__base");
                newId.append(suf);

                if (oldConstMap.count(newId) == 0) {
                    printMessage(FINEm,
                                 "Adding missing declaration for " + newId);

                    Node* newDef = new Node(DEF_CONST,
                                            newId,
                                            i->second->children);
                    ctxt->insert(newDef);
                }
            }
        }
    }

    //--------------------------------------------------------------------
    // Add fun type decls for enumeration types
    //--------------------------------------------------------------------

    //--------------------------------------------------------------------
    // Add declarations for record size constants
    //--------------------------------------------------------------------

    for (int i = 0; i != ctxt->typeSeq.arity(); i++) {
        Node* typeDecl = ctxt->typeSeq.child(i);

        if ( typeDecl->kind == DEF_TYPE && typeDecl->arity() == 1) {
            Node* type = typeDecl->child(0);

            if (type->kind == ENUM_TY) {

                string enumName = typeDecl->id;
                Node* enumTy = new Node(TYPE_ID, enumName);

                // E__pos : E -> int 
                ctxt->insert(new Node(DECL_FUN,
                                      enumName + "__pos",
                                      new Node(SEQ, enumTy),
                                      Node::int_ty));

                // E__val : int -> E  
                ctxt->insert(new Node(DECL_FUN,
                                      enumName + "__val",
                                      new Node(SEQ, Node::int_ty),
                                      enumTy->copy()));

                //   E__succ : E -> E
                ctxt->insert(new Node(DECL_FUN,
                                      enumName + "__succ",
                                      new Node(SEQ, enumTy->copy()),
                                      enumTy->copy()));

                //   E__pred : E -> E
                ctxt->insert(new Node(DECL_FUN,
                                      enumName + "__pred",
                                      new Node(SEQ, enumTy->copy()),
                                      enumTy->copy()));

                //   E__LE   : E * E -> bool
                ctxt->insert(new Node(DECL_FUN,
                                      enumName + "__LE",
                                      new Node(SEQ, enumTy->copy(), enumTy->copy()),
                                      Node::bool_ty));


                //   E__LT   : E * E -> bool
                ctxt->insert(new Node(DECL_FUN,
                                      enumName + "__LT",
                                      new Node(SEQ, enumTy->copy(), enumTy->copy()),
                                      Node::bool_ty));


            } // END if type decl of enum type

            // V8.1.4 seems to require these declarations

            else if (type->kind == RECORD_TY) {
                string recordName = typeDecl->id;

                ctxt->insert(new Node(DEF_CONST,
                                      recordName + "__size",
                                      Node::int_ty));
                                      
            }
        } // END if type is definition
    } // END for loop over typeSeq
    return;
}


//========================================================================
// declCheck class
//========================================================================
// Class for finding undeclared constants and functions
//
//

class declCheck {
public:
    set<string> constIds;
    set<string> funIds;
 
    Node* operator() (FDLContext* c, Node* n);
};

Node*
declCheck::operator() (FDLContext* c, Node* n) {

    if (n->kind == ID
        && !hasUpperCaseStart(n->id)
        && c->lookupId(n->id) == 0) {

        constIds.insert(n->id);

    } 
    else if (n->kind == FUN_AP
             && c->lookupFun(n->id) == 0) {

        funIds.insert(n->id);
    }
    return n;
}

// return true if success

bool
findUndeclaredIds(FDLContext* ctxt, Node* unit) {

    declCheck c;
    mapOverWithContext(c, ctxt, unit);

    if (c.constIds.size() > 0) {

        string ids;
        for (set<string>::iterator i = c.constIds.begin(); i != c.constIds.end()
                 ; i++ ) {
            ids += ENDLs + (*i);
        }
        printMessage(ERRORm, "Found undeclared identifiers:"
                               + ids);
    }

    if (c.funIds.size() > 0) {

        string funIds;
        for (set<string>::iterator i = c.funIds.begin(); i != c.funIds.end()
                 ; i++ ) {
            funIds += ENDLs + (*i);
        }
        printMessage(ERRORm, "Found undeclared functions:"
                               + funIds);
    }

    return c.constIds.size() + c.funIds.size() > 0;
}




//========================================================================
// Close rules.
//========================================================================
// Infer types for free variables in rules and universally quantify them.


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// inferVarTypes
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
Strategy:


- Consider formula f.  
- need to infer type for each free Prolog var.
- Then replace f with universal closure of f, closing over these free vars.

Input f.
Output closure of f.

Need to map over f, maintaining map of freevars to types.

To allow for binders already in f (likely?) map over f function should
maintain context.


mapped function operation:

- Lookup up type Tyn of n. (possibly null if type inf fails).

- consider each subnode n of f in turn, maintaining context of bindings



  - consider each ith child n' of n in turn
    if n' is an unbound Upper-case var
    then 

     Let Tyn' = type of n' (ith arg type if Tyn non null).  Tyn' might be null.

    if (Tyn' is non null and n' is unbound in typing map)
    then 
       create entry in typing map n' -> Tyn'

    else if (Tyn' non null and n' bound in typing map to `pending')

    then   update entry in typing map to n' -> Tyn'

    else if (Tyn' null and n' is unbound in typing map)
    then
       create pending entry in typing map

- At end, 
    Check no pending bindings in typing map.  If are any, report as
    warning or error. 
    Form closure using non-pending bindings.
*/


class VarTypingFun {
public:
    map<string,Node*> vMap;
    
    VarTypingFun() {};
    
    Node* operator() (FDLContext* c, Node* n);
};

Node* 
VarTypingFun::operator() (FDLContext* c, Node* n) {

    Formatter::setFormatter(VanillaFormatter::getFormatter());

    Node* subNodeTypes = c->getSubNodeTypes(n);

    vector<Node**> subNodes = n->getSubNodes();

    for ( int i = 0; i != (int) subNodes.size(); i++) {

        Node* subNode = *(subNodes.at(i));
        string subNodeId = subNode->id;

        if ( subNode->kind == ID
             && hasUpperCaseStart(subNodeId)
             && c->lookupId(subNodeId) == 0 ) {

            Node* subNodeTy;
            if (subNodeTypes != 0) {
                subNodeTy = subNodeTypes->child(i);
            } else {
                subNodeTy = Node::unknown;
            }

            if ( subNodeTy->kind != UNKNOWN && vMap.count(subNodeId) == 0) {

                vMap.insert(make_pair(subNodeId, subNodeTy));

            } else if ( subNodeTy->kind != UNKNOWN
                        && vMap.count(subNodeId) == 1 
                        && vMap.find(subNodeId)->second->kind == PENDING) {

                vMap.find(subNodeId)->second = subNodeTy; 
            }
            else if (subNodeTy->kind == UNKNOWN
                     && vMap.count(subNodeId) == 0) {

                vMap.insert(make_pair(subNodeId, new Node(PENDING)));
            }
            
        }
    }
    return n;
}


Node* closeExpr(FDLContext* c, int rulNum, Node* expr) {

    string rulNumStr (intToString(rulNum));

    VarTypingFun varTypingFun;
    mapOverWithContext(varTypingFun, c, expr);

    if (varTypingFun.vMap.size() == 0) {
        return expr;
    }
    Formatter::setFormatter(VanillaFormatter::getFormatter());
    printMessage(FINEm, "closeExpr: vars found in rule " + rulNumStr
                 + ENDLs + expr->toString());

    
    Node* decls = new Node(SEQ);
    for (map<string,Node*>::iterator i = varTypingFun.vMap.begin();
         i != varTypingFun.vMap.end();
         i++) {

        string id = i->first;
        Node* type = i->second;

        if (type->kind == PENDING) {
            printMessage(INFOm,
                         "closeExpr (rule " + rulNumStr
                         + "): no binding for: " + id);
        } else {
            Formatter::setFormatter(VanillaFormatter::getFormatter());
            printMessage(FINEm, 
                         "closeExpr (rule " + rulNumStr
                         + "): binding " + id + " to " + type->toString());

            decls->addChild(new Node(DECL, id, type->copy()));
        }
    }
    return new Node(FORALL, decls, expr);
}




void closeRules(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);

    for (int i = 0; i != rules->arity(); i++) {
        rules->child(i) = closeExpr(ctxt, i+1, rules->child(i));
    }
    return;
}

//========================================================================
// Resolve overloading. 
//========================================================================


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// normaliseIneqs
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void normaliseIneqs(Node* n) {

    if (n->kind == GE) {
        n->kind = LE;
        swap(n->child(0), n->child(1));
    }
    if (n->kind == GT) {
        n->kind = LT;
        swap(n->child(0), n->child(1));
    }
    return;
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// resolveIneqs
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Resolve overloading of LT and LE by looking at types of children.

Node*
resolveIneqs (FDLContext* ctxt, Node* n) {

    string enumSuffix;
    Kind intRelKind;
    Kind realRelKind;

    if (n->kind == LE) {
        enumSuffix = "__LE";
        intRelKind = I_LE;
        realRelKind = R_LE;

    } else if (n->kind == LT) {
        enumSuffix = "__LT";
        intRelKind = I_LT;
        realRelKind = R_LT;

    } else {
        return n;
    }
         
    Node* child0BaseTy =
        ctxt->normaliseType(ctxt->getType(n->child(0)))->expandSubranges();
    Node* child1BaseTy =
        ctxt->normaliseType(ctxt->getType(n->child(1)))->expandSubranges();

    if (child0BaseTy->kind == INT_TY && child1BaseTy->kind == INT_TY) {
        n->kind = intRelKind;
        return n;
    }
    else if (child0BaseTy->kind == REAL_TY || child1BaseTy->kind == REAL_TY) {
        n->kind = realRelKind;
        return n;
    }
    else if (child0BaseTy->kind == ENUM_TY && child1BaseTy->kind == ENUM_TY) {

        n->kind = FUN_AP;
        n->id = (child0BaseTy->id) + enumSuffix;
        return n;
    }
    else {

        Formatter::setFormatter(VanillaFormatter::getFormatter());
        printMessage(INFOm,
                     "resolveIneqs:  Inequality "
                     + ENDLs + n->toString() 
                     + ENDLs + "has unexpected arg types: " 
                     + ENDLs + child0BaseTy->toString()
                     + ENDLs + child1BaseTy->toString());
        return n;
    }
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// resolveSuccPred
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


Node*
resolveSuccPred(FDLContext* ctxt, Node* n) {
    if (! (n->kind == SUCC || n->kind == PRED) ) return n;

    Node* childTy = ctxt->getType(n->child(0));
    Node* baseChildTy = ctxt->normaliseType(childTy)->expandSubranges();
 
    if (baseChildTy->kind == INT_TY) {
        if (n->kind == SUCC)
            n->kind = I_SUCC;
        else
            n->kind = I_PRED;
        return n;
    }
    if (  baseChildTy->kind == ENUM_TY) {

        if (n->kind == SUCC) {
            n->id = (baseChildTy->id) + "__succ";
        }
        else {
            n->id = (baseChildTy->id) + "__pred";
        }
        n->kind = FUN_AP;
        return n;
    }
    printMessage(INFOm,
                 "resolveSuccPred: unexpected arg type: "
                 + kindString(baseChildTy->kind)
                 + "{" + (baseChildTy->id) + "}"
                 );
    return n;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// resolveEq
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Add type of equality or disequality as 3rd child.

// Is there a coercion from ty1 to ty2?

bool existsCoercion(Node* ty1, Node* ty2) {

    return ty1->kind == INT_TY && ty2->kind == REAL_TY;
}

Node*
resolveEq(FDLContext* ctxt, Node* n) {
    if (! ((n->kind == EQ || n->kind == NE)
           && n->arity() == 2)
        ) return n;

    Node* child0Ty = ctxt->getType(n->child(0))->expandSubranges();
    Node* child1Ty = ctxt->getType(n->child(1))->expandSubranges();

    if (child0Ty->equals(child1Ty) && child0Ty->kind != UNKNOWN) {
        n->addChild(child0Ty->copy());
        return n;
    }
        
    Node* child0BaseTy = ctxt->canoniseType(child0Ty)->expandSubranges();
    Node* child1BaseTy = ctxt->canoniseType(child1Ty)->expandSubranges();

    if (child0BaseTy->equals(child1BaseTy) && child0Ty->kind != UNKNOWN) {
        n->addChild(child0BaseTy->copy());
        return n;
    }

    if (existsCoercion(child0BaseTy, child1BaseTy)) {
        n->addChild(child1BaseTy->copy());
        return n;
    }
        
    if (existsCoercion(child1BaseTy, child0BaseTy)) {
        n->addChild(child0BaseTy->copy());
        return n;
    }
    printMessage(INFOm,
                 "resolveEq: incompatible types "
                 + kindString(child0BaseTy->kind)
                 + " and " + kindString(child1BaseTy->kind)
                 + " for children "
                 + kindString(n->child(0)->kind)
                 + " and " + kindString(n->child(1)->kind)
                 );
    return n;
}






//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Resolve arithmetic Real/Int overloading
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Node* resolveUnaryArithNode(FDLContext* c, Node* n, Kind iKind, Kind rKind) {

    Node* ty0 = c->normaliseType(c->getType(n->child(0)))->expandSubranges();

    if (ty0->kind == INT_TY) {
        return n->updateKind(iKind);
    }
    else if (ty0->kind == REAL_TY) {
        return n->updateKind(rKind);
    }
    else {
        printMessage(INFOm,
                     "resolveUnaryArithNode: " + kindString(n->kind)
                     + "has unexpected arg type: "
                     + kindString(ty0->kind) );
        return n;
    }
}
    
Node* resolveBinaryArithNode(FDLContext* c, Node* n, Kind iKind, Kind rKind) {

    Node* ty0 = c->normaliseType(c->getType(n->child(0)))->expandSubranges();
    Node* ty1 = c->normaliseType(c->getType(n->child(1)))->expandSubranges();

    if (ty0->kind == INT_TY && ty1->kind == INT_TY)
        return n->updateKind(iKind);
    else if (ty0->kind == REAL_TY || ty1->kind == REAL_TY) {
        return n->updateKind(rKind);
    }
    else {
        printMessage(INFOm,
                     "resolveBinaryArithNode: " + kindString(n->kind)
                     + " has unexpected arg types: "
                     + kindString(ty0->kind) + " " 
                     + kindString(ty1->kind) );
        return n;
    }
}
    
Node* resolveArithOps(FDLContext* c, Node* n) {

    switch (n->kind) {
    case(PLUS):   return resolveBinaryArithNode(c, n, I_PLUS, R_PLUS);
    case(TIMES):  return resolveBinaryArithNode(c, n, I_TIMES, R_TIMES);
    case(MINUS):  return resolveBinaryArithNode(c, n, I_MINUS, R_MINUS);
    case(EXP):    return resolveBinaryArithNode(c, n, I_EXP, R_EXP);
    case(UMINUS): return resolveUnaryArithNode(c, n, I_UMINUS, R_UMINUS);
    case(ABS):    return resolveUnaryArithNode(c, n, I_ABS, R_ABS);
    case(SQR):    return resolveUnaryArithNode(c, n, I_SQR, R_SQR);
    default:
        return n;
    }
}







//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Add int to real coercions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Try to insert coercion.  If see problem, just continue.  Typechecking later
// will catch it.

Node* insertRealCoercion(FDLContext* c, Node* n) {

    // Skip n if is internal node of compound operator
    if (n->kind == SEQ
        || n->kind == ASSIGN
        || n->kind == SUBRANGE
        || n->kind == DECL)
        return n;

    vector<Node**> subNodes = n->getSubNodes();
    Node* subNodeTypes = c->getSubNodeTypes(n); // Expected types of subnodes

    for (int i = 0; i != (int) subNodes.size(); i++) {
        Node** subNode = subNodes.at(i);

        Node* subNodeTy =
            c->normaliseType(c->getType(*subNode))->expandSubranges();

        if (subNodeTy->kind != INT_TY) continue;

        // Put test here rather than at top so we only report error
        // when coercion might be needed.
        if (subNodeTypes == 0) {
            printMessage(FINEm,
                         "insertRealCoercion: can't find child types of "
                         + kindString(n->kind));
            return n;
        }
        Node* expectedSubNodeTy =
            c->normaliseType(subNodeTypes->child(i))->expandSubranges();

        if (expectedSubNodeTy->kind == INT_TY) {
            continue;
        }
        else if (expectedSubNodeTy->kind == REAL_TY) {
            *subNode = new Node(TO_REAL, *subNode);
        }
        else {
            printMessage(INFOm,
                         "insertRealCoercion: trying to coerce INT_TY to:"
                         + kindString(expectedSubNodeTy->kind)
                         + ", the reported type of subNode " + intToString(i+1)
                         + " of " + kindString(n->kind) + " node");
        }
    }
    return n;
        

}


Node* resolveOverloadedOp(FDLContext* c, Node* n) {

    n = resolveSuccPred(c,n);
    n = resolveArithOps(c,n);
    n = resolveEq(c,n);
    n = resolveIneqs(c,n);

    return n;
}

void resolveOverloading(FDLContext* ctxt, Node* unit) {

    //--------------------------------------------------------------------
    // Invert > and >= relations.
    //--------------------------------------------------------------------

    unit->mapOver(normaliseIneqs);

    //--------------------------------------------------------------------
    // Resolve overloaded operators
    //--------------------------------------------------------------------
    // - succ and pred operators over int & enum types.
    // - int/real overloading of operators and relations
    // - equality and disequalities
    // - < and <= inequalities.over int, real and enum types.


    // Important here that mapOver is bottom up, so can do resolution
    // in single pass.

    mapOverWithContext(resolveOverloadedOp, ctxt, unit);
    
    //--------------------------------------------------------------------
    // Insert coercions
    //--------------------------------------------------------------------

    mapOverWithContext(insertRealCoercion, ctxt, unit);

}



//========================================================================
// Eliminate standard FDL functions.
//========================================================================
// Eliminate occurrences of ABS, SQR, EXP functions, ODD predicate.

// SUCC and PRED taken care of separately.


Node* elimStdFDLFun(Node* n) {
    switch (n->kind) {
    case I_ABS: return n->updateKindAndId(FUN_AP,"int___abs");
    case R_ABS: return n->updateKindAndId(FUN_AP,"real___abs");
    case I_SQR: {
        n->kind = I_EXP;
        n->addChild(new Node(NATNUM, "2"));
        return n;
    }
    case R_SQR: {
        n->kind = R_EXP;
        n->addChild(new Node(NATNUM, "2"));
        return n;
    }
    case ODD:   return n->updateKindAndId(FUN_AP,"int___odd");
    default:    return n;
    }
}


void elimStdFDLFuns(Node* unit) {
    unit->mapOver1(elimStdFDLFun);
}



//========================================================================
// Simplify array constructors
//========================================================================
// exp ::= 
//    MK_ARRAY{arr-name} a1 ... an
//  | MK_ARRAY{arr-name} exp a1 ... an
//
//  a ::= ASSIGN indexset val
// 
// indexset ::= 
//    i
//  | INDEX_AND i1 ... ik,  k > 1
//
// i ::= SEQ ie1 ... iem
//
// ie ::=  exp | SUBRANGE exp exp


// 1. Eliminate use of ASSIGN for multiple index tuples
//
//    (INDEX_AND i1 ... ik) ASSIGN e  ---> (i1 ASSIGN e) ... (ik ASSIGN e)

// 2. If one index in an index set uses a SUBRANGE expression, make sure all
//    of them do. 

Node* simpMkArray(Node* n) {
    if (n->kind != MK_ARRAY) return n;

    // Eliminate multiple index ASSIGNs

    Node* newN = new Node(MK_ARRAY, n->id);

    for (int i = 0; i != n->arity(); i++) {
        Node* c = n->child(i);
        
        if (c->kind != ASSIGN  || c->child(0)->kind != INDEX_AND) {
            
            newN->addChild(c);
        }
        else { // c = ASSIGN (INDEX_AND i1 ... ik) e

            Node* indexSet = c->child(0);
            Node* e = c->child(1);

            // Reuse existing nodes for 1st of assignments
            c->child(0) = indexSet->child(0);
            newN->addChild(c);

            // Create new nodes for rest of assignments
            for (int j = 1; j < indexSet->arity(); j++) {
                newN->addChild(new Node (ASSIGN,
                                         indexSet->child(j),
                                         e->copy()) );
            }
        }
    }
    // Make use of SUBRANGE uniform

    for (int i = 0; i != newN->arity(); i++) {

        if (newN->child(i)->kind != ASSIGN) continue;
        Node* indexSet = newN->child(i)->child(0);

        bool existsSubrangeExp = false;
        for (int j = 0; j != indexSet->arity(); j++) {
            if (indexSet->child(j)->kind == SUBRANGE) {
                existsSubrangeExp = true;
                break;
            }
        }
        if (existsSubrangeExp) {
            for (int j = 0; j != indexSet->arity(); j++) {
                if (indexSet->child(j)->kind != SUBRANGE) {
                    indexSet->child(j) =
                        new Node(SUBRANGE,
                                 indexSet->child(j),
                                 indexSet->child(j)->copy()
                                 );
                }
            }
        }
    }
    return newN;
}


//========================================================================
// Fix IDs
//========================================================================
// Replace IDs by CONST or VAR, as appropriate.
// VAR only used for bound variables, *not* FDL vars.  FDL vars 
// are mapped to CONSTs.
// Distinction needed for SMTLIB format.


Node* resolveIDs(FDLContext* c, Node* n) {
    if (n->kind == ID) {
        // Check bound vars before consts, in case bindings shadow a const.
        if (c->lookupBinding(n->id) != 0)
            return n->updateKind(VAR); 
        if (c->lookupVar(n->id) != 0
            || c->lookupConst(n->id) != 0
            || c->lookupEnumConst(n->id) != 0
            ) {
            return n->updateKind(CONST);
        }
        else {
            printMessage(INFOm,
                         "resolveIDs: encountered unexpected id: "
                         + n->id);
            return n;
        }
    }
    else
        return n;
}


//========================================================================
// Augmentation of array operators with array name information.
//========================================================================
/*
     ARR_ELEMENT arr index-tup    --> ARR_ELEMENT{type-id} arr index-tup
     ARR_UPDATE arr index-tup val --> ARR_UPDATE{type-id} arr index-tup val

    where t-id is the type-id for the type of argument rcd.

*/

Node* addTypesToArrayOps (FDLContext* c, Node* n) {

    if (n->kind == ARR_ELEMENT || n->kind == ARR_UPDATE) {

        Node* arrTy = c->getType(n->child(0));
        Node* normArrTy = c->normaliseType(arrTy);

        if (normArrTy->kind == ARRAY_TY && normArrTy->id.size() > 0) {
            n->id = normArrTy->id;
        }
        else {
            printMessage(INFOm,
                         "Failed to infer type of array operator "
                         + kindString(n->kind) +  "{"
                         + n->id + "}"
                         + ENDLs + "Found type "
                         + kindString(normArrTy->kind));
            printMessage(FINESTm,
                         "Failed to infer type of array operator " + ENDLs
                         + (n->toString()) + ENDLs + "Found type " + ENDLs
                         + (normArrTy->toString())
                         );
        }
    }
    return n;
}




//========================================================================
// Augmentation of record operators with record name information.
//========================================================================
/*
     RCD_ELEMENT{rcd-id} rcd     --> RCD_ELEMENT{rcd-id} rcd  t-id 
     RCD_UPDATE{rcd-id} rcd val   --> RCD_UPDATE{rcd-id} rcd val  t-id

where t-id is the type-id for the type of argument rcd.

*/

Node* addTypesToRecordOps (FDLContext* c, Node* n) {

    if (n->kind == RCD_ELEMENT || n->kind == RCD_UPDATE) {

        Node* rcdTy = c->getType(n->child(0));
        Node* normRcdTy = c->normaliseType(rcdTy);

        if (normRcdTy->kind == RECORD_TY) {
            n->addChild(new Node(TYPE_PARAM, normRcdTy->id));
        }
        else {
            printMessage(INFOm,
                         "Failed to infer type of record operator "
                         + kindString(n->kind) +  "{"
                         + n->id + "}"
                         + ENDLs + "Found type "
                         + kindString(normRcdTy->kind));
            printMessage(FINESTm,
                         "Failed to infer type of record operator " + ENDLs
                         + (n->toString()));
        }
    }
    return n;
}

//========================================================================
//========================================================================
// Main processing functions
//========================================================================
//========================================================================


FDLContext* 
putUnitInStandardForm(Node* unit) {

    //--------------------------------------------------------------------
    // Split off decls and turn rules into formula sequence
    //--------------------------------------------------------------------

    FDLContext* ctxt = RegulariseUnitStructure(unit);

    // Now: unit == UNIT(DECLS(), RULES(...), GOALS(...))
    // ctxt has maps for const, vars, funs, types and boundvars

    // -------------------------------------------------------------------
    // Augment constant and var decls
    // -------------------------------------------------------------------
    // Add decls for init state vars
    // Add missing X__base__first and X__base__last decls
    // Add decls for enum type functions

    augmentConstDecls(ctxt, unit);

    //--------------------------------------------------------------------
    // Check all function and constant identifiers have bindings
    //--------------------------------------------------------------------

    if (findUndeclaredIds(ctxt, unit))
        return 0;

    //--------------------------------------------------------------------
    // Simplify array constructors
    //--------------------------------------------------------------------

    unit->mapOver1(simpMkArray); 

    //--------------------------------------------------------------------
    // Ensure type inference set up to be eager
    //--------------------------------------------------------------------

    ctxt->strictTyping = false;

    //--------------------------------------------------------------------
    // Close any rules with free variables.
    //--------------------------------------------------------------------
    // Infer var types from typings of surrounding operators.

    closeRules(ctxt, unit);

    //--------------------------------------------------------------------
    // Resolve overloading
    //--------------------------------------------------------------------
    // Also eliminate > and >= operators and add in int-to-real coercions.

    resolveOverloading(ctxt, unit);

    //--------------------------------------------------------------------
    // Add type info to array operators
    //--------------------------------------------------------------------

    mapOverWithContext(addTypesToArrayOps, ctxt, unit);

    //--------------------------------------------------------------------
    // Add type info to record operators
    //--------------------------------------------------------------------

    mapOverWithContext(addTypesToRecordOps, ctxt, unit);


    //--------------------------------------------------------------------
    // Eliminate special operators for FDL standard functions
    //--------------------------------------------------------------------

    elimStdFDLFuns(unit);

    //--------------------------------------------------------------------
    // Resolve uses of IDs
    //--------------------------------------------------------------------
    // Replace with VAR or CONST

    mapOverWithContext(resolveIDs, ctxt, unit);

    return ctxt;
}

