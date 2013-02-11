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

            Node* rule = rules->child(j); // rule = RULE{<name>} <body>
            rule->child(0) = normaliseRule(rule->child(0));
            result->addChild(rule);
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
// Delete rules excluded by user
//========================================================================
// using -skip-rule=<rule-name> option.

// Handles multiple rules excluded with multiple uses of option.

void
deleteRulesExcludedByUser(FDLContext* ctxt,
                          Node* unit,
                          UnitInfo* unitInfo) {

    Node* rules = unit->child(1);

    vector<string> userExcludedRules = optionVals("skip-rule");
    if (userExcludedRules.size() == 0) return;
    
    for (int ruleNum = 1; ruleNum <= rules->arity(); ruleNum++) {

        Node* rule = rules->child(ruleNum-1);

        string ruleName = rule->id;

        // Exclude rule if name matches any of the provided patterns
        bool excludeRule = false;
        for (int ri = 0; ri != (int) userExcludedRules.size(); ri++) {
            const string& rulePat = userExcludedRules.at(ri);
            if (stringMatch(rulePat, ruleName)) {
                excludeRule = true;
                break;
            }
        }
        if (excludeRule) {
            rule->child(0) = nTRUE;
            unitInfo->addExcludedRule(ruleNum-1);
        }
    }
    return;
}



//========================================================================
// Find badly-formed logical nodes
//========================================================================
// A "logical node" is a cluster of nodes used to represent particular
// operators in FDL. 

// Occasionally badly formed logical nodes come up in user rules
// files, so need to flush them out.  If they are left in place, later
// code (e.g. using getSubNodes()) will crash.

// We only check for issues that are not caught by parsing.

class LogicalNodeCheck {
public:
    string messages;
    void addMessage(const string& m);
    Node* operator() (FDLContext* c, Node* n);
};

void
LogicalNodeCheck::addMessage(const string& m) {
    if (messages.size() > 0) messages += ENDLs;
    messages += m;
    return;
}

Node*
LogicalNodeCheck::operator() (FDLContext* c, Node* n) {

    // For clarity here, we go through all the case considered by the
    // Node::getSubNodes() method.

    switch(n->kind) {

    case FORALL:
    case EXISTS:
        // FORALL (SEQ d1 ... nk) body [pat]
        // EXISTS (SEQ d1 ... nk) body [pat]
        //   where di = DECL{id} type
        break;

    case ARR_ELEMENT: {
        // ARR_ELEMENT arr (SEQ i1 ... in),  n >= 1
        if (n->arity() == 2
            && n->child(1)->kind == SEQ
            && n->child(1)->arity() > 0) break;
        
        addMessage("Badly formed ARR_ELEMENT at "
                   + c->getPathString() + ".");
        break;
    }
    case ARR_UPDATE: {
        // ARR_UPDATE arr (SEQ i1 ... in) val,  n >= 1
        if (n->arity() == 3
            && n->child(1)->kind == SEQ
            && n->child(1)->arity() > 0) break;
        
        addMessage("  Badly formed ARR_UPDATE at "
                   + c->getPathString() + ".");
        break;
    }
    case MK_ARRAY: {
        // MK_ARRAY{arrname} default a1 ... an, n >= 0
        // MK_ARRAY{arrname} a1 ... an,         n >= 1
        //   where 
        //     ai = ASSIGN indextuples val,  m >= 1
        //     indextuples = 
        //           indextuple 
        //           | INDEX_AND indextuple1 ... indextuplek,  k > 1
        //     indextuple = (SEQ i1 ... im)
        //     ij = e  |  SUBRANGE e1 e2

        bool isBad = false;
        
        for (int i = 0; i != n->arity(); i++) {
            Node* c = n->child(i);
            if (i == 0 && c->kind != ASSIGN) {
                continue;
            }
            // Check indeed that have an assignment at each child
            if (c->kind == ASSIGN && c->arity() == 2) {

                // Check each tuple in indexTuples is a SEQ of indexes

                Node* indexTuples = c->child(0);
                if (indexTuples->kind == INDEX_AND) {
                    // Have multiple index tuples.  Check each one.
                    for (int j = 0; j != indexTuples->arity(); j++) {
                        if (indexTuples->child(j)->kind != SEQ) isBad = true;
                    }
                } else {
                    // Have a single tuple of indexes
                    if (indexTuples->kind != SEQ) isBad = true;
                }
            } else {
                // ai is not ASSIGN node with arity 2.
                isBad = true;
            }
        }

        if (isBad) addMessage("Badly formed MK_ARRAY at "
                              + c->getPathString() + ".");
        break;
    }
    case MK_RECORD: {
        // MK_RECORD{rcdname} a1 ... an, n >= 1
        //   where ai = ASSIGN{fldname} val

        bool isBad = false;
        for (int i = 0; i < n->arity(); i++) {
            Node* assign = n->child(i);
            if (! (assign->kind == ASSIGN && assign->arity() == 1))
                isBad = true;
        }
        if (isBad) addMessage("Badly formed MK_RECORD at "
                              + c->getPathString() + ".");
        break;
    }
    case RECORD_TY: {
        // RECORD_TY{rcdname} d1 ... dn, n >= 1
        //   where di = DECL{fldname} Ti
        break;
    }
    default: {
        break;
    }
    }
    return n;
}

void
deleteRulesWithBadLogicalNodes(FDLContext* ctxt,
                               Node* unit,
                               UnitInfo* unitInfo) {

    Node* rules = unit->child(1);
    for (int ruleNum = 1; ruleNum <= rules->arity(); ruleNum++) {

        Node* rule = rules->child(ruleNum-1);

        LogicalNodeCheck lnc;
        mapOverWithContext(lnc, ctxt, rule);

        if (lnc.messages.size() > 0) {

            // rule = RULE{<name>} <rule>
            string ruleName = rule->id;
            string ruleKind =
                unitInfo->isDirUserRule(ruleNum - 1)
                ? "directory user"
                : unitInfo->isUnitUserRule(ruleNum - 1)
                ? "unit user"
                : "Examiner";
            
            printMessage(ERRORm,
                         "FDL issue" + ENDLs
                         + "Found oddly-formed " + ruleKind 
                                                 + " rule " + ruleName + ENDLs
                         + lnc.messages + ENDLs
                         + "Deleting rule.");

            rule->child(0) = nTRUE;
            unitInfo->addExcludedRule(ruleNum-1);
        }
    }
    return;
}

//========================================================================
// Find undeclared constants, functions and types
//========================================================================

class declCheck {
public:
    set<string> constIds;
    set<string> funIds;
    set<string> typeIds;
 
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
    else if (n->kind == TYPE_ID
             && c->lookupType(n->id) == 0) {

        typeIds.insert(n->id);
    }
    return n;
}

// return true if success

void
findUndeclaredIds(FDLContext* ctxt,
                  Node* unit,
                  string& undeclaredConsts,
                  string& undeclaredFuns,
                  string& undeclaredTypes) {

    declCheck c;
    mapOverWithContext(c, ctxt, unit);

    for (set<string>::iterator i = c.constIds.begin()
             ; i != c.constIds.end()
             ; i++ ) {
        undeclaredConsts += ENDLs + (*i);
    }
    for (set<string>::iterator i = c.funIds.begin()
             ; i != c.funIds.end()
             ; i++ ) {
        undeclaredFuns += ENDLs + (*i);
    }
    for (set<string>::iterator i = c.typeIds.begin()
             ; i != c.typeIds.end()
             ; i++ ) {
        undeclaredTypes += ENDLs + (*i);
    }
    return;
}


bool checkForUndeclaredIds(FDLContext* ctxt, Node* unit) {
    string constStr;
    string funStr;
    string typeStr;
    findUndeclaredIds(ctxt, unit, constStr, funStr, typeStr);
    if (constStr.size() > 0) {
        printMessage(ERRORm,
                     "FDL issue" + ENDLs
                     + "Found undeclared constants:" + constStr);
    }
    if (funStr.size() > 0) {
        printMessage(ERRORm,
                     "FDL issue" + ENDLs
                     + "Found undeclared functions:" + funStr);
    }
    if (typeStr.size() > 0) {
        printMessage(ERRORm, 
                     "FDL issue" + ENDLs
                     + "Found undeclared types:" + typeStr);
    }
    return constStr.size() + funStr.size() + typeStr.size() > 0 ;
}

// Rules with undeclared ids replaced with constant true.
// Positions of deleted rules recorded in unitInfo->excludedRules.

// Undeclared ids in Examiner generated rules are always flagged as
// errors

// Undeclared ids in directory-level user-supplied rules are flagged
// with warnings unless option
// -expect-dir-user-rules-with-undeclared-ids is provided.

void
deleteRulesWithUndeclaredIds(FDLContext* ctxt,
                             Node* unit,
                             UnitInfo* unitInfo) {

    Node* rules = unit->child(1);
    for (int ruleNum = 1; ruleNum <= rules->arity(); ruleNum++) {

        Node* rule = rules->child(ruleNum-1);
        string undeclaredConsts;
        string undeclaredFuns;
        string undeclaredTypes;
        findUndeclaredIds(ctxt,
                          rule,
                          undeclaredConsts,
                          undeclaredFuns,
                          undeclaredTypes);
        if (undeclaredConsts.size()
            + undeclaredFuns.size()
            + undeclaredTypes.size() > 0) {

            // rule = RULE{<name>} <rule>
            string ruleName = rule->id;
            string ruleKind =
                unitInfo->isDirUserRule(ruleNum - 1)
                ? "directory user"
                : unitInfo->isUnitUserRule(ruleNum - 1)
                ? "unit user"
                : "Examiner";

            int messageLevel = ERRORm;
            string FDLissue("FDL issue" + ENDLs);
            if (option("expect-dir-user-rules-with-undeclared-ids")
                && unitInfo->isDirUserRule(ruleNum - 1)) {
                
                messageLevel = INFOm;
                FDLissue = "";
            }

            rule->child(0) = nTRUE;

            printMessage(messageLevel,
                         FDLissue
                         + "Found " + ruleKind + " rule " + ruleName
                         + " with " + ENDLs
                         + "  undeclared constants:" + undeclaredConsts + ENDLs
                         + "  undeclared functions:" + undeclaredFuns + ENDLs
                         + "  undeclared types:" + undeclaredTypes + ENDLs
                         + "Rule deleted.");

            unitInfo->addExcludedRule(ruleNum-1);
        }
    } // End for 
}


//========================================================================
// normaliseIneqs
//========================================================================

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
// Close rules.
//========================================================================
// Infer types for free variables in rules and universally quantify them.

/*
Strategy:


- Consider formula f.  
- need to infer type for each free Prolog var.
- Then replace f with universal closure of f, closing over these free vars.

Input f.
Output closure of f.

Need to map over f, maintaining map of freevars to types.

To allow for binders already in f, map over f function should
maintain context.


mapped function operation:

- consider each subnode n of f in turn, maintaining context of bindings

  - consider each ith child n' of n in turn
    if n' is an unbound Upper-case var
    then 

     Let Tyn' = best guess at type that node n expects n' to have

     if (n' is unbound in typing map)
    then 
       create entry in typing map n' -> Tyn'
    else 
       merge new guess with existing guess in map for n'.
- At end, 

  -  Form closure using resolved bindings.

  -  Flag if there any unresolved bindings

  - If, in phase 3 of type resolution, an unresolved binding is for ir
    or ire type, choose to use i.  Hopefully this guess is right some
    of the time, maybe more often than other guesses.
  
*/


class VarTypingFun {
public:
    map<string,Node*> vMap;
    
    VarTypingFun() {};
    
    Node* mergeVarTypings(Node* ty1, Node* ty2, FDLContext* c);

    Node* operator() (FDLContext* c, Node* n);
};

Node*
VarTypingFun::mergeVarTypings(Node* ty1, Node* ty2, FDLContext* c) {
    // Hopefully common case first.
    if (ty1->kind == ty2->kind) {
        return ty1;
    }
    Kind k1 = c->canoniseType(ty1)->kind;
    Kind k2 = c->canoniseType(ty2)->kind;

    if (k1 == UNKNOWN) {
        return ty2;    
    }
    if (k2 == UNKNOWN) {
        return ty1;    
    }
    if (k1 == INT_REAL_OR_ENUM_TY) {
        // Expect ty2 is INT_TY, REAL_TY, ENUM_TY, INT_OR_REAL_TY or
        // INT_REAL_OR_ENUM_TY. 
        return ty2;   
    }
    if (k2 == INT_REAL_OR_ENUM_TY) {
        // Expect ty1 is INT_TY, REAL_TY, ENUM_TY or INT_OR_REAL_TY
        return ty1;    
    }
    if (k1 == INT_OR_REAL_TY) {
        // Expect ty2 is INT_TY, REAL_TY or INT_OR_REAL_TY
        return ty2;   
    }
    if (k2 == INT_OR_REAL_TY) {
        // Expect ty1 is INT_TY or REAL_TY 
        return ty1;   
    }
    if (k1 == INT_TY && k2 == REAL_TY) {
        return ty1;   
    }
    if (k1 == REAL_TY && k2 == INT_TY) {
        return ty2;   
    }
    if (k1 == k2) {
        return ty1;
    }
    return Node::no_ty; // Types are incompatible.  Hopefully never see this!
}

// Collect list of variables whose typing is not fully resolved, and provide
// a best inferred type for each.

// Variable list includes both free variables and variables bound with type
// INT_OR_REAL_TY.

Node* 
VarTypingFun::operator() (FDLContext* c, Node* n) {

    Formatter::setFormatter(VanillaFormatter::getFormatter());

    Node* subNodeTypes =
        c->typeResolutionPhase == 2 && option("assume-var-in-real-pos-is-real")
        ? c->getExactSubNodeTypes(n)
        : c->getSubNodeTypeOptions(n);

    vector<Node**> subNodes = n->getSubNodes();

    for ( int i = 0; i != (int) subNodes.size(); i++) {

        Node* subNode = *(subNodes.at(i));
        string subNodeId = subNode->id;

        if ( subNode->kind == ID
             && hasUpperCaseStart(subNodeId)
            ) {

            Node* decl = c->lookupId(subNodeId);

            // Check if subNodeId is free or only partially typed
            // if decl != 0, expect it alway to be of form DECL{id}[type]

            if (decl == 0 || decl->child(0)->kind == INT_OR_REAL_TY) {

                Node* subNodeTy;
                if (subNodeTypes != 0) {
                    subNodeTy = subNodeTypes->child(i);
                } else {
                    subNodeTy = Node::unknown;
                }
                
                if (decl != 0) {
                    subNodeTy = mergeVarTypings(decl->child(0), subNodeTy, c);
                }

                pair< map<string,Node*>::iterator,bool > p = 
                    vMap.insert(make_pair(subNodeId, subNodeTy));

                bool insertSuccess = p.second;
                if (!insertSuccess) {
                    Node** typeInMap = & ((p.first)->second);
                    *typeInMap = mergeVarTypings(*typeInMap, subNodeTy, c);
                }
            }
        }
    }
    return n;
}


Node*
closeExpr(FDLContext* c, const string& ruleName, Node* expr) {

    VarTypingFun varTypingFun;
    mapOverWithContext(varTypingFun, c, expr);

    if (varTypingFun.vMap.size() == 0) {
        return expr;
    }
    Formatter::setFormatter(VanillaFormatter::getFormatter());
    printMessage(FINEm, "closeExpr: vars found in rule " + ruleName
                 + ENDLs + expr->toString());

    Node* decls;
    if (expr->kind == FORALL) {
        decls = expr->child(0);
    } else {
        decls = new Node(SEQ);
    }

    // The oldType for a var is that (if any) given in the current
    // outermost FORALL bindings of expr.

    // The inferredType for a var is that found by varTypingFun.

    // The newType for a var is the new type to use for the var.  It
    // might differ from the inferred type, as it also takes account
    // of assumptions given in command line options.
    
    for (map<string,Node*>::iterator i = varTypingFun.vMap.begin();
         i != varTypingFun.vMap.end();
         i++) {
        string id = i->first;

        // Collect inferredType.
        Node* inferredType = i->second;

        // Find oldType and ptr (if any) to its declaration.
        Node* oldType = Node::unknown;
        Node* oldDecl = 0;
        for (int i = decls->arity(); i != 0; ) {
            i--;
            Node* decl = decls->child(i);
            if (id == decl->id) {
                oldDecl = decl;
                oldType = oldDecl->child(0);
            }
        }
        
        // Set newType, taking account of command line options.

        Node* newType = inferredType;

        if ( c->typeResolutionPhase == 2
             && inferredType->kind == INT_OR_REAL_TY
             && option("assume-int-or-real-var-is-int")

            ) {
            newType = Node::int_ty;

            if (!option("suppress-warnings-of-var-type-assumptions")) {
                printMessage(WARNINGm,
                      "FDL issue" + ENDLs
                       + "Cannot find unique type for free variable " + id 
                       + " in rule " + ruleName + ENDLs
                       + "Type is either integer or real." + ENDLs
                       + "Assuming it to have integer type. ");

            }

        }
        else if ( c->typeResolutionPhase == 2
             && inferredType->kind == INT_OR_REAL_TY
             && option("assume-int-or-real-var-is-real")

            ) {

            newType = Node::real_ty;

            if (!option("suppress-warnings-of-var-type-assumptions")) {
                printMessage(WARNINGm,
                      "FDL issue" + ENDLs
                       + "Cannot find unique type for free variable " + id 
                       + " in rule " + ruleName + ENDLs
                       + "Type is either integer or real." + ENDLs
                       + "Assuming it to have real type." + ENDLs
                       + "This is potentially unsound.");
            }
        }

        // If progress, then record newType.

        if (newType->kind != oldType->kind
            && newType->kind != UNKNOWN
            && newType->kind != INT_REAL_OR_ENUM_TY
            && newType->kind != NO_TY
            ) {

            // Add/update decls of outermost FORALL
            if (oldDecl != 0) {
                oldDecl->child(0) = newType->copy();
            }
            else {
                decls->addChild(new Node(DECL, id, newType->copy()));
            }

            c->typeResolutionMadeProgress = true;
            if (newType->kind == INT_OR_REAL_TY) {
                c->typeResolutionIncomplete = true;
            }

            // Report if getSubNodeTypes is forcing var to be real

            // Are assuming here that the only way a free var can get typed
            // real is if option assume-var-in-real-pos-is-real is selected.
            
            if (inferredType->kind == REAL_TY
                && !option("suppress-warnings-of-var-type-assumptions")) {
                printMessage(WARNINGm,
                      "FDL issue" + ENDLs
                       + "Cannot find unique type for free variable " + id 
                       + " in rule " + ruleName + ENDLs
                       + "Type is either integer or real." + ENDLs
                       + "Assuming it to have real type,  " + ENDLs
                       + "  since it is an argument to an operator expecting a real. " + ENDLs
                       + "This is potentially unsound.");
            }
        }
        else {
            c->typeResolutionIncomplete = true;
        }
        
    } // End of for i in varTypingFun.vMap


    Node* result;
    if (expr->kind == FORALL) {
        result = expr;
    }
    else if (decls->arity() > 0) {
        result = new Node(FORALL, decls, expr);
    }
    else {
        result = expr;
    }
    
    return result;
}

void closeRules(FDLContext* ctxt, Node* unit) {

    Node* rules = unit->child(1);

    for (int i = 0; i != rules->arity(); i++) {
        Node* rule = rules->child(i);  // rule = RULE{<name>} <formula>
        rule->child(0) = closeExpr(ctxt, rule->id, rule->child(0));
    }
    return;
}

//========================================================================
// Resolve overloading. 
//========================================================================


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// resolveIneqs
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Resolve overloading of LT and LE by looking at types of children.

// Child types are I=int, R=real,E=enum type, U=unknown.
// 
// 
// I and I  -> I
// R or  R  -> I
// IR or IR -> IR  (if not already IR)
// E or  E  -> E
// o/w no progress.

Node*
resolveIneqs (FDLContext* ctxt, Node* n) {

    string enumSuffix;
    Kind intRelKind;
    Kind realRelKind;
    Kind intRealRelKind;

    if (n->kind == LE || n->kind == IR_LE) {
        enumSuffix = "__LE";
        intRelKind = I_LE;
        realRelKind = R_LE;
        intRealRelKind = IR_LE;

    } else if (n->kind == LT || n->kind == IR_LT) {
        enumSuffix = "__LT";
        intRelKind = I_LT;
        realRelKind = R_LT;
        intRealRelKind = IR_LT;

    } else {
        return n;
    }
         
    Node* child0BaseTy =
        ctxt->normaliseType(ctxt->getType(n->child(0)))->expandSubranges();
    Node* child1BaseTy =
        ctxt->normaliseType(ctxt->getType(n->child(1)))->expandSubranges();

    if (child0BaseTy->kind == INT_TY && child1BaseTy->kind == INT_TY) {

        n->kind = intRelKind;
        ctxt->typeResolutionMadeProgress = true;
    }
    else if (child0BaseTy->kind == REAL_TY || child1BaseTy->kind == REAL_TY) {

        n->kind = realRelKind;
        ctxt->typeResolutionMadeProgress = true;
    }
    else if (child0BaseTy->kind == INT_OR_REAL_TY
             || child1BaseTy->kind == INT_OR_REAL_TY) {

        if (n->kind != intRealRelKind) {
            n->kind = intRealRelKind;
            ctxt->typeResolutionMadeProgress = true;
        }
        else {
            ctxt->typeResolutionIncomplete = true;
        }
    }
    else if (child0BaseTy->kind == ENUM_TY) {

        n->kind = FUN_AP;
        n->id = (child0BaseTy->id) + enumSuffix;
        ctxt->typeResolutionMadeProgress = true;
    }
    else if (child1BaseTy->kind == ENUM_TY) {

        n->kind = FUN_AP;
        n->id = (child1BaseTy->id) + enumSuffix;
        ctxt->typeResolutionMadeProgress = true;
    }
    else {
        ctxt->typeResolutionIncomplete = true;
    }
    return n;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// resolveSuccPred
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Assumes pred and succ never used on reals.  This seems consistent
// with FDL documentation.


Node*
resolveSuccPred(FDLContext* ctxt, Node* n) {
    if (! (n->kind == SUCC || n->kind == PRED) ) return n;

    Node* childTy = ctxt->getType(n->child(0));
    Node* baseChildTy = ctxt->normaliseType(childTy)->expandSubranges();
 
    if (baseChildTy->kind == INT_TY
        || baseChildTy->kind == INT_OR_REAL_TY) {
        if (n->kind == SUCC) {
            n->kind = I_SUCC;
        } else {
            n->kind = I_PRED;
        }
        ctxt->typeResolutionMadeProgress = true;
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
        ctxt->typeResolutionMadeProgress = true;
        return n;
    }
    ctxt->typeResolutionIncomplete = true;
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

bool hasSupertype(Node* ty) {

    return ty->kind == INT_TY;
}

Node*
resolveEq(FDLContext* ctxt, Node* n) {
    if (! ((n->kind == EQ && n->arity() == 2) || n->kind == IR_EQ) 
        ) return n;

    Node* child0Ty = ctxt->getType(n->child(0))->expandSubranges();
    Node* child1Ty = ctxt->getType(n->child(1))->expandSubranges();

    // Pick up an easy common case first
    if (child0Ty->equals(child1Ty) && !isUnresolvedType(child0Ty) ) {
        n->addChild(child0Ty->copy());
        n->kind = EQ;
        ctxt->typeResolutionMadeProgress = true;
        return n;
    }
    Node* child0BaseTy = ctxt->canoniseType(child0Ty)->expandSubranges();
    Node* child1BaseTy = ctxt->canoniseType(child1Ty)->expandSubranges();

    /* Cases

       * = any type
       T = resolved type with no supertype.  Includes real.
       I = int type
       IR = int_or_real type

       *,T or T,*   Use T

       I and I      Use I

       IR or IR     Use IR providing not already IR.

       otherwise.   Fail.

     */
  

    if (!isUnresolvedType(child0Ty) && ! hasSupertype(child0Ty)) {
        // T,*
        n->addChild(child0BaseTy->copy());
        n->kind = EQ;
        ctxt->typeResolutionMadeProgress = true;
        return n;
    }

    if (!isUnresolvedType(child1Ty) && ! hasSupertype(child1Ty)) {
        // *,T
        n->addChild(child1BaseTy->copy());
        n->kind = EQ;
        ctxt->typeResolutionMadeProgress = true;
        return n;
    }

    if (child0BaseTy->equals(child1BaseTy) && ! isUnresolvedType(child0Ty) ) {
        // I,I
        n->addChild(child0BaseTy->copy());
        ctxt->typeResolutionMadeProgress = true;
        return n;
    }

    if ( (child0BaseTy->kind == INT_OR_REAL_TY
          || child1BaseTy->kind == INT_OR_REAL_TY) && n->kind == EQ
        ) {
        // IR or IR
        n->kind = IR_EQ;
        ctxt->typeResolutionMadeProgress = true;
        return n;
    }

    ctxt->typeResolutionIncomplete = true;
    return n;
}






//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Resolve arithmetic Real/Int overloading
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Node* resolveUnaryArithNode(FDLContext* c, Node* n, Kind iKind, Kind rKind) {

    Node* ty0 = c->normaliseType(c->getType(n->child(0)))->expandSubranges();

    if (ty0->kind == INT_TY) {
        c->typeResolutionMadeProgress = true;
        return n->updateKind(iKind);
    }
    else if (ty0->kind == REAL_TY) {
        c->typeResolutionMadeProgress = true;
        return n->updateKind(rKind);
    }
    else {
        c->typeResolutionIncomplete = true;
        return n;
    }
}
    
Node* resolveBinaryArithNode(FDLContext* c, Node* n, Kind iKind, Kind rKind) {

    Node* ty0 = c->normaliseType(c->getType(n->child(0)))->expandSubranges();
    Node* ty1 = c->normaliseType(c->getType(n->child(1)))->expandSubranges();

    if (ty0->kind == REAL_TY || ty1->kind == REAL_TY) {
        // R,R   I,R   R,I   U,R   R,U
        c->typeResolutionMadeProgress = true;
        return n->updateKind(rKind);
    }
    else if (ty0->kind == INT_TY && ty1->kind == INT_TY) {
        // I,I
        c->typeResolutionMadeProgress = true;
        return n->updateKind(iKind);
    }
    else {
        c->typeResolutionIncomplete = true;
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
// Augmentation of array operators with array name information.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
     ARR_ELEMENT arr index-tup    --> ARR_ELEMENT{type-id} arr index-tup
     ARR_UPDATE arr index-tup val --> ARR_UPDATE{type-id} arr index-tup val

    where t-id is the type-id for the type of argument rcd.

*/

Node* addTypeToArrayOp (FDLContext* c, Node* n) {

    if ((n->kind == ARR_ELEMENT && n->id.size() == 0)
        ||
        (n->kind == ARR_UPDATE && n->id.size() == 0)) {

        Node* arrTy = c->getType(n->child(0));
        Node* normArrTy = c->normaliseType(arrTy);

        if (normArrTy->kind == ARRAY_TY && normArrTy->id.size() > 0) {
            n->id = normArrTy->id;
            c->typeResolutionMadeProgress = true;
        }
        else {
            c->typeResolutionIncomplete = true;
        }
    }
    return n;
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Augmentation of record operators with record name information.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
     RCD_ELEMENT{rcd-id} rcd     --> RCD_ELEMENT{rcd-id} rcd  t-id 
     RCD_UPDATE{rcd-id} rcd val   --> RCD_UPDATE{rcd-id} rcd val  t-id

where t-id is the type-id for the type of argument rcd.

Two strategies are used:

1. Look at record type declarations in FDL file and see if only one
   record type uses the rcd-id fieldname.  If so, use that type.

2. Try to infer type of rcd argument.  

*/

Node* addTypeToRecordOp (FDLContext* c, Node* n) {

    if ((n->kind == RCD_ELEMENT && n->arity() == 1)
        ||
        (n->kind == RCD_UPDATE && n->arity() == 2)) {

        Node* rcdTyFromDecls = c->lookupRecordField(n->id);
        if (rcdTyFromDecls->kind != UNKNOWN) {

            n->addChild(rcdTyFromDecls->copy());
            c->typeResolutionMadeProgress = true;
            return n;
        }
        Node* rcdTy = c->getType(n->child(0));
        Node* normRcdTy = c->normaliseType(rcdTy);

        if (normRcdTy->kind == RECORD_TY) {
            n->addChild(new Node(TYPE_PARAM, normRcdTy->id));
            c->typeResolutionMadeProgress = true;
            return n;
        }
        c->typeResolutionIncomplete = true;
    }
    return n;
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
    Node* subNodeTypes = c->getExactSubNodeTypes(n); // Expected types of subnodes

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Assembly of main procedure for resolving overloading
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Node* resolveOverloadedOp(FDLContext* c, Node* n) {

    n = resolveSuccPred(c,n);
    n = resolveArithOps(c,n);
    n = resolveEq(c,n);
    n = resolveIneqs(c,n);
    n = addTypeToArrayOp(c,n);
    n = addTypeToRecordOp(c,n);
    return n;
}

void resolveOverloading(FDLContext* ctxt, Node* unit) {

    //--------------------------------------------------------------------
    // Resolve overloaded / polymorphic operators
    //--------------------------------------------------------------------
    // - succ and pred operators over int & enum types.
    // - int/real overloading of operators and relations
    // - equality and disequalities
    // - < and <= inequalities.over int, real and enum types.
    // - polymorphic array operators
    // - polymorphic record operators

    // Useful here that mapOver is bottom up, as resolution often depends
    // on examining typing of arguments

    mapOverWithContext(resolveOverloadedOp, ctxt, unit);
    
}





//========================================================================
// Type resolution 
//========================================================================
// Resolve all polymorphism / overloading and figure out types
// of free variables.


void resolveTyping(FDLContext* ctxt, Node* unit) {

    //--------------------------------------------------------------------
    // Ensure type inference set up to be eager
    //--------------------------------------------------------------------

    ctxt->strictTyping = false;


    //--------------------------------------------------------------------
    // Iteratively close rules and resolve overloading
    //--------------------------------------------------------------------
    // Victor 0.9.1 and before used a single call of closeRules() followed
    // resolveOverloading().
    // Examples have been seen in the 2nd Tokeneer release of user-defined
    // rules for which this is not sufficient.

    // The typing resolutions in Phase 1 are always safe.  Those in
    // Phase 2 are potentially unsound.

    int maxPhase =
        option("assume-var-in-real-pos-is-real")
        || option("assume-int-or-real-var-is-real")
        || option("assume-int-or-real-var-is-int")
        ? 2
        : 1;
      
    for (int phase = 1; phase <= maxPhase; phase++) {

        ctxt->typeResolutionPhase = phase;

        const int maxLoopIt = 10;
        
        for (int i = 1 ; i <= maxLoopIt; i++) {

            ctxt->typeResolutionMadeProgress = false;
            ctxt->typeResolutionIncomplete = false;

            closeRules(ctxt, unit);

            resolveOverloading(ctxt, unit);

            if (! ctxt->typeResolutionMadeProgress
                || ! ctxt->typeResolutionIncomplete) break;

            if (i == maxLoopIt) {

                printMessage(WARNINGm,
                             "resolveTyping: in phase " + intToString(phase)
                             + ", reached loop iteration " + intToString(i)
                             + ENDLs + "Halting type resolution in this phase");
            }
        } // End for i

        if (! ctxt->typeResolutionIncomplete) break;

    } // End for phase 

    //--------------------------------------------------------------------
    // Insert coercions
    //--------------------------------------------------------------------

    mapOverWithContext(insertRealCoercion, ctxt, unit);


    return;
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
            // Leave ID as is.  Type checking will catch it later.
            /*
            printMessage(ERRORm,
                         "FDL issue" + ENDLs
                         + "resolveIDs: encountered unexpected id: "
                         + n->id + ENDLs
                         + " at " + c->getPathString() + ENDLs
                         + "If in user rule, fix by adding checktype "
                           "precondition for id.");
            */
            return n;
        }
    }
    else
        return n;
}


//========================================================================
//========================================================================
// Main processing function
//========================================================================
//========================================================================


FDLContext* 
putUnitInStandardForm(Node* unit, UnitInfo* unitInfo) {

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
    // Delete rules named in -skip-rule options
    //--------------------------------------------------------------------
    // Updates unitInfo with indexes of rules to exclude.

    deleteRulesExcludedByUser(ctxt, unit, unitInfo);

    //--------------------------------------------------------------------
    // Delete rules with badly-formed logical nodes
    //--------------------------------------------------------------------
    // Updates unitInfo with indexes of rules to exclude.

    deleteRulesWithBadLogicalNodes(ctxt, unit, unitInfo);

    //--------------------------------------------------------------------
    // Delete rules with undeclared constants, functions and types
    //--------------------------------------------------------------------
    // Updates unitInfo with indexes of rules to exclude.

    deleteRulesWithUndeclaredIds(ctxt, unit, unitInfo);

    
    //--------------------------------------------------------------------
    // Check all type, function and constant identifiers have bindings
    //--------------------------------------------------------------------
    // Check throughout whole unit.

    if (checkForUndeclaredIds(ctxt, unit))
        return 0;

    //--------------------------------------------------------------------
    // Simplify array constructors
    //--------------------------------------------------------------------

    unit->mapOver1(simpMkArray); 

    //--------------------------------------------------------------------
    // Invert > and >= relations.
    //--------------------------------------------------------------------

    unit->mapOver(normaliseIneqs);

    //--------------------------------------------------------------------
    // Resolve typing 
    //--------------------------------------------------------------------
    // Resolve typing of overloaded & polymorphic operators and of
    // free vars in rules.

    resolveTyping(ctxt, unit);

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

