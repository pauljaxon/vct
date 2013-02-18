//========================================================================
//========================================================================
// PROCESSOR.CC
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
//
// Intermediate solver-independent processing of FDL unit components.
//


#include <map>
using std::map;
using std::make_pair;
using std::pair;

#include <stdexcept>

#include <algorithm>  // For swap.
using std::swap;

#include <set>
using std::set;

#include "processor.hh"

#include "context.hh"
#include "normalisation.hh"
#include "translation.hh"

#include "utility.hh"
#include "node-utils.hh"

#include "bignum.hh"

#include "formatter.hh"

using std::string;



using namespace z;

/*


============================================================================
New options
============================================================================
Options defined here and in translation.cc

Option sections below presented in order of processing

Enumerated type abstraction
---------------------------

-abstract-enums  # Use RLS file axioms
-elim-enums      # map to integer subranges

Early array and record abstraction
----------------------------------
-abstract-arrays-records-early # Enable abstraction at this point

# See "Late array and record abstraction" section below for rest of options


QFOL->FOL conversion
--------------------
-bit-type   # Do conversion

-bit-type-bool-eq-to-iff

-bit-type-with-ite

-bit-type-prefer-bit-vals  
-bit-type-prefer-props    
                                 # If neither then uses bit-fun if instance in
                                 # individ position.  O/w uses bool-fun.
# Tracing options

-trace-prop-to-bit-insertion
-trace-intro-bit-ops-and-rels

type refinement
---------------

-refine-types           # Master control

-refine-bit-eq-equiv    # Add in definition for bit-valued non-triv equiv rels
                        # Use when -bit-type-with-ite not included
-refine-int-subrange-type          # types to refine
-refine-bit-type-as-int-subtype
-refine-bit-type-as-int-quotient
-refine-array-types-with-quotient

-refine-array-types-with-weak-extension-constraint

-refine-uninterpreted-types        # use if want to ensure infinite models

# Options for axiom generation

-no-subtyping-axioms
-strong-subtyping-axioms
-no-functionality-axioms

# Tracing options
-trace-refine-types-quant-relativisation
-trace-refine-types-eq-refinement

Late array and record abstraction
----------------------------------
-abstract-arrays-records-late  # Enable abstraction at this point

# Eliminate redundant operators

-elim-array-constructors 
-elim-record-constructors 
-abstract-record-updates 


# Add axioms defining types axiomatically

-add-array-select-update-axioms
-add-array-extensionality-axioms

-add-record-select-constructor-axioms
-add-record-constructor-extensionality-axioms  
-add-record-select-update-axioms
-add-record-eq-elements-extensionality-axioms

# Introduce aliases for equalities to help with matching ext axioms
-use-array-eq-aliases
-use-record-eq-aliases

# Abstract operators and types

-abstract-array-select-updates
-abstract-array-types

-abstract-record-selects-constructors
-abstract-record-selects-updates
-abstract-record-types



bit abstraction 
----------------
-abstract-bit-ops
-abstract-bit-valued-eqs
-abstract-bit-valued-int-le
-elim-bit-type-and-consts
-abstract-bit-type-and-consts


Arithmetic simplification
-------------------------
-abstract-real-div
-elim-consts
-ground-eval-exp
-ground-eval
-expand-exp-const
-arith-eval
-sym-prefix
-sym-consts



Arithmetic abstraction
----------------------
-abstract-nonlin-times
-abstract-exp
-abstract-divmod
-abstract-real-div
-abstract-reals




Miscellaneous
-------------
-switch-types-to-int
-lift-quants
-strip-quantifier-patterns
-elim-type-aliases

*/

/* 
===================================================================
Some old documentation 
===================================================================
(Some of this could be out of date)

Terminology: 

- A `declaration' of a type|constant|variable|function introduces the
  name, along with a type in the latter 3 cases, but does not give a
  value.

- A `definition' of a type or constant also provides a value.

- An `array|record|enumeration type definition' is a type definition
  with an array|record|enumeration type on the rhs.

- An `array|record|enumeration type name' is the type name introduced by
  an array|record|enumeration type definition.

  FDL syntax seems to ensure that every array|record|enumeration type
  gets a name.

- An enumeration type definition declares one or more `enumeration constants'.


*/



//========================================================================
// Arithmetic translations
//========================================================================

#include "arith.cc"


//========================================================================
// Well-formedness check
//========================================================================
// Check that only see expected Node kinds and check subnode typing
// is as expected.



Node* checkNodeWF(FDLContext* ctxt, Node* n) {

    switch (n->kind) {

    default: break;
    }
    return n;

}


void checkUnitWellFormedness(FDLContext* ctxt, Node* unit) {
    mapOverWithContext(checkNodeWF, ctxt, unit);
}


//========================================================================
// Printing of quantifier type info
//========================================================================


class Fun {
private:
    FDLContext* ctxt;
public:
    Fun (FDLContext* c) : ctxt(c) {};

    void operator() (Node* n) {

        // [FORALL|EXISTS] (SEQ decl+) exp   where decl ::= DECL{id} type 
        if (n->kind == FORALL || n->kind == EXISTS) {

            bool printFmla = false;

            for ( int i = 0; 
                  i != n->child(0)->arity();
                  i++) {

                Node* decl = n->child(0)->child(i);
                    
                Node* normTy = ctxt->normaliseType(decl->child(0));

                if (normTy->kind != INT_TY && normTy->kind != PENDING)
                    printFmla = true;

                printMessage(INFOm, "quant types ("
                             + intToString(i) + " " 
                             + decl->id + "): "
                             + kindString(normTy->kind));
            }
            if (printFmla)
                printMessage(FINEm, "Interesting quantification" + ENDLs
                             + (n->toString()));
        }
    }



};


//========================================================================
// Lift quantifiers
//========================================================================
/*
Provide rules such as:

1. IMPLIES e1 (FORALL (SEQ decls) e2) 
   -->
   FORALL (SEQ decls) (IMPLIES e1 e2) 

   provided decls don't capture any free variables in e1.

2. FORALL (SEQ decls1) (FORALL (SEQ decls2) e) 
   --> FORALL (SEQ decls1 decls2) e

*/


set<string>* getFreeVarsAux (Node* n) {

    if (n->arity() == 0) {

        set<string>* s = new set<string>();
        if (n->kind == VAR)
            s->insert(n->id);
        return s;

    } else if (n->kind == FORALL || n->kind == EXISTS) {

        Node* decls = n->child(0);
        set<string>* s = getFreeVarsAux(n->child(1));
        for (int i = 0; i != (decls->arity()); i++) {
            s->erase(decls->child(i)->id);
        }
        return s;
        
    } else {

        set<string>* s = getFreeVarsAux(n->child(0));
        for (int i = 1; i != n->arity(); i++) {
            set<string>* ss = getFreeVarsAux(n->child(i));
            s->insert(ss->begin(), ss->end());
            delete ss;
        }
        return s;
    }
}

set<string> getFreeVars (Node* n) {
    set<string>* s = getFreeVarsAux(n);
    set<string> ss = *s;
    delete s;
    return ss;
}


// Apply rule(s).

Node* liftQuants(FDLContext* c, Node* n) {

    if (n->kind == IMPLIES && n->child(1)->kind == FORALL) {

        Node* e1 = n->child(0);
        Node* decls = n->child(1)->child(0);
    
        set<string> e1FreeVars = getFreeVars(e1);

        for (int i = 0; i != decls->arity(); i++) {
            if ( e1FreeVars.count(decls->child(i)->id) != 0 ) {

                printMessage(WARNINGm,
                             "FORALL lift of " + (decls->child(i)->id)
                             + " inhibited to avoid capture");
                return n;
            }
        }

        n->kind = FORALL;
        n->child(0) = decls;

        n->child(1)->kind = IMPLIES;
        n->child(1)->child(0) = e1;

    }
    else if (n->kind == FORALL && n->child(1)->kind == FORALL) {

        Node* decls1 = n->child(0);
        Node* decls2 = n->child(1)->child(0);
        Node* e = n->child(1)->child(1);

        decls1->appendChildren(decls2);
        n->child(1) = e;
    }
    return n;
}




//========================================================================
// Use tuples for array function indexes
//========================================================================
// Not used.  Keep in case used in future.

Node* useTupleForArrayIndex(Node* n) {
    // ARR_ELEMENT a i1 ... ik --> ARR_ELEMENT a (SEQ i1 ... ik)
    if (n->kind == ARR_ELEMENT) {

        Node* array = n->child(0);
        n->popLeftChild();
        n->kind = SEQ;
        return new Node(ARR_ELEMENT, array, n);

    }
    // ARR_UPDATE a i1 ... ik v --> ARR_UPDATE a (SEQ i1 ... ik) v
    else if (n->kind == ARR_UPDATE) {

        Node* array = n->child(0);
        Node* newValue = n->lastChild();
        n->popChild();
        n->popLeftChild();
        n->kind = SEQ;
        return new Node(ARR_UPDATE, "", array, n, newValue);
    }
    else
        return n;
}

void useTuplesForArrayIndexes(Node* unit) {
    unit->mapOver1(useTupleForArrayIndex);
}

//========================================================================
// Remove EQ, NE and ITE type arguments
//========================================================================

Node* removeTypeArg(Node* n) {

    if ( ( (n->kind == EQ || n->kind == NE || n->kind == TERM_EQ)
           && n->arity() == 3)
         || (n->kind == ITE && n->arity() == 4)
        )  {
        n->popChild();
    }
    return n;
}

void removeTypeArgs(Node* unit) {
    unit->mapOver1(removeTypeArg);
}

//========================================================================
// Remove Patterns from Quantifiers
//========================================================================
// Patterns are used in the axioms characterising operations on the bit type.
// May be used elsewhere too.

Node* removeQuantifierPattern(Node* n) {

    if ( (n->kind == FORALL || n->kind == EXISTS)
         && n->arity() == 3) {
        n->popChild();
    }
    return n;
}

void removeQuantifierPatterns(Node* unit) {
    unit->mapOver1(removeQuantifierPattern);
}


//========================================================================
// Use integer type for all types
//========================================================================
// Replace all uses of type ids by integer type. 

// Check that all defined types are abstract or aliases for defined types
// or int type.

Node* typeIdToIntTy(Node* n) {
    switch(n->kind) {
    case TYPE_ID: return Node::int_ty;
    default: return n;
    }
}

void checkTypesAbstract(Node* n) {
    if (n->kind == DEF_TYPE) {
        if (n->arity() == 0
            || n->child(0)->kind == TYPE_ID
            || n->child(0)->kind == INT_TY) return;

        printMessage(ERRORm, "Found non-abstract type def for " + (n->id));
    }
    return;
}

void switchAllTypesToInt(FDLContext* ctxt, Node* unit) {
    unit->mapOver1(typeIdToIntTy);
    ctxt->termSeq.mapOver1(typeIdToIntTy);
    ctxt->typeSeq.mapOver(checkTypesAbstract);
    return;
}




//========================================================================
// Strip universal hyps and rules
//========================================================================

void stripUnivHyps(Node* unit) {

    Node* oldRules = unit->child(1);
    Node* newRules = new Node(SEQ);
        
    for (int i = 0; i != oldRules->arity(); i++) {
        if (oldRules->child(i)->kind != FORALL)
            newRules->addChild(oldRules->child(i));
    }
    unit->child(1)->children = newRules->children;

    Node* goals = unit->child(2);
    for (int i = 0; i != goals->arity(); i++) {
        Node* goal = goals->child(i);

        if (goal->arity() < 2) continue;
        Node* oldHyps = goal->child(0);
        Node* newHyps = new Node(SEQ);

        for (int j = 0; j != oldHyps->arity(); j++) {
            if (oldHyps->child(j)->kind != FORALL)
                newHyps->addChild(oldHyps->child(j));

        }
        goal->child(0)->children = newHyps->children;
    }
}


//========================================================================
// Gather concls
//========================================================================

Node* gatherConcls(Node* goals) {
    Node* result = new Node(AND);

    for (int i = 0; i != goals->arity(); i++) {

        if (goals->child(i)->arity() == 2) { // If goal is non-trivial
            
            result->addChild(goals->child(i)->child(1));
        }
    }
    return result;
}

//========================================================================
// Tracing properties of units
//========================================================================

Node* identifyEqAtCompoundTypes(FDLContext* ctxt, Node* n) {
    if (n->kind == EQ) {
        Node* baseType = ctxt->normaliseType(n->child(2));

        if (baseType->kind == ARRAY_TY) {

            printMessageWithHeader("TRACE",
                                   "Found equality at array type "
                                   + typeToName(n->child(2)) + ENDLs
                                   + "Path: " + ctxt->getPathString() + ENDLs
                                   + n->toString());

        }
        else if (baseType->kind == RECORD_TY) {

            printMessageWithHeader("TRACE",
                                   "Found equality at record type "
                                   + typeToName(n->child(2)) + ENDLs
                                   + "Path: " + ctxt->getPathString() + ENDLs
                                   + n->toString());
                          
        }
    }
    return n;
}


void identifyEqsAtCompoundTypes(FDLContext* ctxt, Node* unit) {

    if (option("analyse-only-concls")) {

        Node* concls = gatherConcls(unit->child(2));
        mapOverWithContext(identifyEqAtCompoundTypes, ctxt, concls);

    } else {
        mapOverWithContext(identifyEqAtCompoundTypes, ctxt, unit);
    }
    return;
}

//========================================================================
// User Rule Auditing
//========================================================================

/* Delete the provided set of goals from the .vcg file and generate an
   alternative set of goals for checking properties of user rules.

   5 kinds of goals generated in audit.

  A (1) No user rules.   Always expect not true, but good just to double check.

  B (1) All user rules in hyps.  If provable when A is not, this
        suggests the user rules taken together are inconsistent.

  C (n) 1 user rule at time in hyps.  If single rule is inconsistent  

  D (n) concl = 1 user rule at time, no other user rules in hyps.
    Says which user rules are only there for proof reasons.
  E (n) concl = 1 user rule at time, rest of user rules in hyps.
    Identifies interdependencies between user rules

Construction of new goals;

URs = user rules
uri = user rule i, i ranging over all non-excluded user rules

      URs-uri in Hs.  uri in Hs.   uri as C

   A 
   B        Yes          Yes
   C                     Yes
   D                                 Yes
   E        Yes                      Yes


For each, we add Goal id naming kind of rule, and for C,D,E, the uri.
Reporting code uses this goal id rather than extractGoalInfo to fill
in origins info in .vct report.

Generate origins info as follows 

Name       Kind             index
  A       no urules
  B       all urules
  C       urule as H         <name of user rule added>
  D       urule as C         <name of user rule added>
  E       urule from rest    <name of user rule added>

Simpler to allow hyps to have
labels, just as rules can. 
*/

// Fake an enumeration type, as C++ doesn't support enumeration types
// that can actually be easily enumerated over in a loop.

typedef int RLUAuditKind;  // Kinds of rule audit goals

#define NO_URULES  0
#define ALL_URULES 1
#define URULE_AS_H 2
#define URULE_AS_C 3
#define URULE_FROM_REST 4


// if k == NO_URULES or ALL_URULES, the ruleIndex argument is 
// irrelevant and is ignored.

Node* buildRLUAuditGoal(UnitInfo* unitInfo,
                        Node* unit,
                        int ruleIndex,
                        RLUAuditKind k ) {

    Node* rules = unit->child(1);

    string goalKind(
        k == NO_URULES
        ? "no urules"
        : k == ALL_URULES
        ? "all urules"
        : k == URULE_AS_H
        ? "urule as H" 
        : k == URULE_AS_C
        ? "urule as C"
        : k == URULE_FROM_REST
        ? "urule from rest"
        : "BAD audit goal");

    string indexedRuleName(
      (k == URULE_AS_H || k == URULE_AS_C || k == URULE_FROM_REST)
      ? rules->child(ruleIndex)->id
      : "");
    
    Node* goal = new Node(GOAL,
                          goalKind + "," + indexedRuleName,
                          new Node(HYPS),
                          new Node(CONCLS));

    Node* concls = goal->child(1);
    if (k == NO_URULES) {
        concls->addChild(new Node(FALSE));
        return goal;
    }

    Node* indexedRule = rules->child(ruleIndex);

    if (k == URULE_AS_C || k == URULE_FROM_REST) {
        concls->addChild(indexedRule->child(0)->copy());
    } else {
        concls->addChild(new Node(FALSE));
    }

    if (k == URULE_AS_C) return goal;

    Node* hyps = goal->child(0);
    if (k == URULE_AS_H) {
        hyps->addChild(indexedRule->copy()->updateKind(HYP));
        return goal;
    }

    // k == ALL_URULES or URULE_FROM_REST
    for (int i = 0; i != unitInfo->unitRLURulesEnd; i++) {
        if (unitInfo->isExcludedRule(i)
            || (k == URULE_FROM_REST && i == ruleIndex)) continue;
        
        Node* hypI = rules->child(i)->copy()->updateKind(HYP);
        hyps->addChild(hypI);
    }

    return goal;
}

void initRLUAuditGoals(UnitInfo* unitInfo, Node* unit) {

    Node* goals = unit->child(2);
    goals->clearChildren();

    // Add new goals for each of the audit parts in turn.

    // Part A
    if (option("rule-audit-a")) {
        goals->addChild(buildRLUAuditGoal(unitInfo, unit, 0, NO_URULES));
    }

    // Stop here if no user rules.  Rest of Parts assume there is
    // at least one user rule
    if (unitInfo->unitRLURulesEnd == 0) return;

    // Part B
    if (option("rule-audit-b")) {
        goals->addChild(buildRLUAuditGoal(unitInfo, unit, 0, ALL_URULES));
    }

    // Parts C,D,E
    for (RLUAuditKind k = URULE_AS_H; k <= URULE_FROM_REST; k++)  {

        if ( ! (    (option("rule-audit-c") && k == URULE_AS_H)
                 || (option("rule-audit-d") && k == URULE_AS_C)
                 || (option("rule-audit-e") && k == URULE_FROM_REST)
                 ) ) continue;
        
        for (int i = 0; i != unitInfo->unitRLURulesEnd; i++) {
            if (unitInfo->isExcludedRule(i)) continue;

            Node* ruleI = unit->child(1)->child(i);
            if (option("rule-audit-rule")
                && optionVal("rule-audit-rule") != ruleI->id ) continue;

            Node* newGoal = buildRLUAuditGoal(unitInfo, unit, i, k);
            goals->addChild(newGoal);
        }
    }
    
    // Ensure all of user rules are excluded from assertion with rest
    // of rules of unit.

    // And keep track of number of user rules considered by rule
    // audit.
    
    for (int i = 0; i != unitInfo->unitRLURulesEnd; i++) {
        if (unitInfo->isExcludedRule(i)) continue;
        unitInfo->addExcludedRule(i);
        unitInfo->auditedUserRules++;
    }
    return;
}

//========================================================================
// Theory translation
//========================================================================

string 
translateUnit(UnitInfo* unitInfo, FDLContext* ctxt, Node* unit) {

    printMessage(FINESTm,
                 "Standard Form context:" + ENDLs
                 + ctxt->extractDecls()->toString()
                 );
    printMessage(FINESTm,
                 "Standard Form rules and goals:" + ENDLs + unit->toString());


    //--------------------------------------------------------------------
    // Check unit well typed
    //--------------------------------------------------------------------

    printMessage(FINEm,
                 "Standard Form type check starting");

    if (!typeCheckUnit("pre-translation", unitInfo, ctxt, unit))
        return "Pre-translation typecheck failed";

    printMessage(FINEm,
                 "Standard Form type check ended");


    //--------------------------------------------------------------------
    // Analysis of unit
    //--------------------------------------------------------------------
    // So far just this one query.  Could imagine many more.

    if (option("identify-eqs-at-arr-rec-types")) {

        identifyEqsAtCompoundTypes(ctxt, unit);
    }

    //--------------------------------------------------------------------
    // Eliminating equalities over boolean type
    //--------------------------------------------------------------------
    if (option("bool-eq-to-iff")) {
        mapOverWithContext(boolEqToIff, ctxt, unit);
    }


    //--------------------------------------------------------------------
    // Eliminating boolean order operations and relations
    //--------------------------------------------------------------------

    unit->mapOver(elimBoolOrderOpsRels);

    //--------------------------------------------------------------------
    // Eliminating enumeration type primitives
    //--------------------------------------------------------------------

    if (option("abstract-enums"))
        abstractEnums(ctxt, unit);
    else if (option("elim-enums"))
        enumsToIntSubranges(ctxt, unit);
    else if (option("axiomatise-enums"))
        axiomatiseEnums(ctxt, unit);

    //--------------------------------------------------------------------
    // Early array and record abstraction
    //--------------------------------------------------------------------

    if (option("abstract-arrays-records-early"))

        abstractArraysRecords(ctxt, unit);


    //--------------------------------------------------------------------
    // QFOL->FOL conversion
    //--------------------------------------------------------------------

    if (option("bit-type")) introBitType(ctxt, unit);

    //--------------------------------------------------------------------
    // Refining types
    //--------------------------------------------------------------------

    if (option("refine-types")) refineTypes(ctxt, unit);
 
    //--------------------------------------------------------------------
    // Abstracting Bit type and associated relations and operations
    //--------------------------------------------------------------------

    abstractBit(ctxt, unit);


    //--------------------------------------------------------------------
    // Late array and record abstraction
    //--------------------------------------------------------------------

    if (option("abstract-arrays-records-late"))

        abstractArraysRecords(ctxt, unit);

    //--------------------------------------------------------------------
    // Do arithmetic simplifications
    //--------------------------------------------------------------------
    // Mostly these strive to make formulas more linear and ease
    // div/mod reasoning.

    arithSimp(ctxt, unit);

    //--------------------------------------------------------------------
    // Abstract non-linear and real arithmetic primitives
    //--------------------------------------------------------------------

    arithAbstract(ctxt, unit);

    //--------------------------------------------------------------------
    // Miscellaneous normalisation steps
    //--------------------------------------------------------------------

    // If quantifiers under implications, matching routines of solvers don't
    // always pick them up. E.g. with Simplify format.

    if (option("lift-quants")) mapOverWithContext(liftQuants, ctxt, unit);

    // Only SMTLib driver is expecting patterns

    if (option("strip-quantifier-patterns")) stripQuantPats(unit);

    // Needed for new SMTLib format translation.

    if (option("elim-type-aliases"))  elimTypeAliases(ctxt, unit);

    // Needed for Simplify translation

    if (option("switch-types-to-int")) switchAllTypesToInt(ctxt, unit);


    // - Convert I_SUCC(x) to x + 1, I_PRED(x) to x - 1.
    elimIntSuccPreds(unit);
    
    //--------------------------------------------------------------------
    // Replace goals with user rule auditing goals, if requested
    //--------------------------------------------------------------------

    if (option("do-rule-audit")) initRLUAuditGoals(unitInfo, unit);

    //--------------------------------------------------------------------
    // Check unit well typed
    //--------------------------------------------------------------------
    
    printMessage(FINESTm,
                 "Primary Translation End - context:" + ENDLs
                 + ctxt->extractDecls()->toString()
                 );
    printMessage(FINESTm,
                 "Primary Translation End - rules and goals:"
                 + ENDLs + unit->toString());


    Formatter::setFormatter(TestFormatter::getFormatter());

    printOnOption("print-translation-end",
                  "Primary Translation End - context:" + ENDLs
                  + ctxt->extractDecls()->toString());

    printOnOption("print-translation-end",
                  "Primary Translation End - rules and goals:"
                  + ENDLs + unit->toString());

    Formatter::setFormatter(VanillaFormatter::getFormatter());


            
    printMessage(FINEm,
                 "Primary Translation End - type check starting");

    if (!typeCheckUnit("post-translation", unitInfo, ctxt, unit))
        return "Post-translation typecheck failed";

    printMessage(FINEm,
                 "Primary Translation End - type check ended");


    //--------------------------------------------------------------------
    // Minor final formatting in preparation for passing to drivers
    //--------------------------------------------------------------------
    // - Remove type argument from EQ, NE, TERM_EQ and ITE nodes.

    removeTypeArgs(unit);

    if (!option("include-patterns-in-quantifiers")) {
        removeQuantifierPatterns(unit);
    }

    //--------------------------------------------------------------------
    // Insert modified declarations back into unit AST.
    //--------------------------------------------------------------------
    unit->child(0) = ctxt->extractDecls();
    delete ctxt;

    return "good";
}


//========================================================================
// Exported top level of processing
//========================================================================

string
elaborateUnit(Node* unit, UnitInfo* unitInfo) {
    printMessage(FINESTm, "Unit parse tree:" + ENDLs + unit->toString());

    // Modifies both unit and unitInfo
    FDLContext* ctxt = putUnitInStandardForm(unit, unitInfo);

    if (ctxt == 0) return "FDL normalisation failed";

    // Modifies possibly all 3 args
    return translateUnit(unitInfo, ctxt, unit);
}







