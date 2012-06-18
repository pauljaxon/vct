//========================================================================
//========================================================================
// ARITH.CC
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


/*
Some older documentation on these arithmetic translations


- If "elim-consts" option:
  
  o Harvest constant and variable definitions of form 
    EQ ID NATNUM{n} or EQ ID (UMINUS NATNUM{n})
    from rules and hyps.  

  o Apply substitutions implied by definitions to all rules and goals,
    except when constant is shadowed by bound variable.

  This can make syntactically non-linear expressions linear in some cases.


- If "ground-eval" option:

  Evaluate out integer arithmetic expressions involving integer literals
  and operators PLUS, MINUS, TIMES, IDIV, EXP and UMINUS.

  Exception thrown if number magnitude exceeds 2^32 - 1.

  If option not given, just evaluate out EXP occurrences.


- If "arith-eval" option:

  Try hard to bring together integer literals in expressions involving
  TIMES and IDIV so as to make expression linear.


- If "sym-const" option with natural number value k, 

  o Add a new abstract constant declaration for each value occurring in 
    rules or goals above k.
  
  o Replace each of these values with the corresponding abstract constant.

  o Add new rules asserting ordering of abstract constants.    If new 
    constants are c1 ... cn, then rules assert that
    k < c1 < c2 < ... < cn.

  o New constant for value <v> is named  <prefix><v> where prefix 
    is either supplied by "sym-prefix" option or defaults to "k@".

*/

//========================================================================
// Auxiliary functions
//========================================================================
// NB: this is very similar to but not same as isIntNode() from bignum.cc.

bool isIntConst(Node* n) {
    if (n->kind == I_UMINUS)
        return isIntConst(n->child(0));
    else
        return n->kind == NATNUM;
}

bool isRealConst(Node* n) {
    if (n->kind == R_UMINUS)
        return isRealConst(n->child(0));
    else if (n->kind == RDIV && ! option("abstract-real-div"))
        return isRealConst(n->child(0)) && isRealConst(n->child(1));
    else if (n->kind == TO_REAL)
        return isIntNode(n->child(0));
    else
        return false;
    
}

//========================================================================
// Elimination of arithmetic constant definitions
//========================================================================

bool isConstDef(Node* n) {
    if (n->kind == RULE) n = n->child(0);
    
    return
        n->kind == EQ
        && n->child(0)->kind == CONST
        && (isIntConst(n->child(1)) || isRealConst(n->child(1)));
}

class ApplyConstSubst {
private:
    map<string,Node*> constMap;
public:
    ApplyConstSubst(map<string,Node*> m) : constMap(m) {}

    Node* operator() (FDLContext* c, Node* n);
};

Node*
ApplyConstSubst::operator() (FDLContext* c, Node* n) {
    if (n->kind != CONST) return n;

    string id = n->id;
    
    map<string,Node*>::iterator i = constMap.find(id);
    if (i != constMap.end()) {
        return i->second->copy();
    } else {
        return n;
    }
}


void elimConsts(FDLContext* ctxt, Node* unit) {

    map<string,Node*> constMap;

    Node* rules = unit->child(1);
    for (int i = 0; i != rules->arity(); i++) {
        Node* rule = rules->child(i);
        if (isConstDef(rule)) {

            Node* eqNode =
                rule->kind == RULE
                ? rule->child(0)
                : rule;
            
            constMap.insert(make_pair(eqNode->child(0)->id, eqNode->child(1)));
        }
    }

    Node* goals = unit->child(2);
    for (int i = 0; i != goals->arity(); i++) {

        Node* goal = goals->child(i);

        if (goal->arity() != 2) continue;
        
        Node* hyps = goal->child(0);

        for (int j = 0; j != hyps->arity(); j++) {
            Node* hyp = hyps->child(j);
            if (isConstDef(hyp)) {
                constMap.insert(make_pair(hyp->child(0)->id,
                                          hyp->child(1)));
            }
        }
    }

    ApplyConstSubst f(constMap);
    mapOverWithContext(f, ctxt, unit);
    return;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// evalExp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Evaluate special case of exponent with natnum arguments.
// Superceded by groundEval, but kept for back comparison purposes.

void evalExp(Node* n) {
    if (n->kind == I_EXP
        && n->child(0)->kind == NATNUM && n->child(1)->kind == NATNUM) {


        MyInt a1(n->child(0));
        MyInt a2(n->child(1));

        printMessage(FINEm, "evalExp: evaluating "
                     + a1.toString() + " to power " + a2.toString() );

        n->kind = NATNUM;
        n->id = MyInt::exp(a1,a2).toString();
        n->clearChildren();
    }
    return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// expandExpConst
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Expand e ** k where k >= 0 is a constant integer.

Node* expandExpConst(Node* n) {
    if ((n->kind == I_EXP || n->kind == R_EXP)
        && n->child(1)->kind == NATNUM) {


        Kind timesKind = (n->kind == I_EXP) ? I_TIMES : R_TIMES;
        Node* result;

        int k = stringToInt(n->child(1)->id);
        Node* e = n->child(0);

        printMessage(FINEm, "expandExpConst: expanding "
                     + kindString(e->kind) + " to power " + intToString(k) );

        if (k == 0) {
            result = n;
            result->kind = NATNUM;
            result->id = "1";
            result->clearChildren();
            if (n->kind == R_EXP) {
                result = new Node(TO_REAL, result);
            }
        }
        else if (k == 1) {
            result = e;
        }
        else { // k >= 2
            result = n;
            result->kind = timesKind;
            // child(0) already set to e.
            result->child(1) = e->copy();

            for (int i = 3; i <= k; i++) {
                // add on ith copy of e
                result = new Node(timesKind, e->copy(), result);
            }
        }
        return result;
    }
    return n;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// abstractNonLinMult
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void abstractNonLinMult(Node* n) {
    if (n->kind == I_TIMES) {
        if (! (isIntNode(n->child(0)) || isIntNode(n->child(1)))) {
            n->kind = FUN_AP;
            n->id = "int___times";
        }
    }
    else if (n->kind == R_TIMES) {
        if (! (isRealConst(n->child(0)) || isRealConst(n->child(1)))) {
            n->kind = FUN_AP;
            n->id = "real___times";
        }
    }
    return;
}


//========================================================================
// Creation of symbolic constants
//========================================================================

class BigConsts {
private:
    set<MyInt> bigConsts;
    MyInt threshold;
    string prefix;

public:
    void setPrefix(string s) {prefix = s;}
    vector<string> getNewIds();
    BigConsts(string thresholdStr);

    void operator() (Node* n);  // To map over expression

    Node* getRules();

};

BigConsts::BigConsts(string thresholdStr) {
    threshold = MyInt(thresholdStr);
}

void
BigConsts::operator() (Node* n) {
    if (n->kind != NATNUM) return;

    MyInt val(n);
    if (threshold < val) { 
        bigConsts.insert(val);
        n->kind = CONST;
        n->id = prefix + (n->id);
    }
    return;
}

vector<string> 
BigConsts::getNewIds() {
    vector<string> result;
    for (set<MyInt>::iterator i = bigConsts.begin();
         i != bigConsts.end();
         i++) {
        result.push_back(prefix + (*i).toString());
    }
    return result;
}

Node*
BigConsts::getRules() {
    Node* rules = new Node(SEQ);
    Node* prev = threshold.toNode();
    
    for (set<MyInt>::iterator i = bigConsts.begin();
         i != bigConsts.end();
         i++) {
        Node* current = new Node(CONST, prefix + (*i).toString());
        
        rules->addChild(nRULE("bigconst lt (" + prev->id + ","
                              + current->id + ")",
                              new Node(I_LT, prev, current)));
        prev = current;

    }
    return rules;
}

//========================================================================
// Introduce symbolic constants
//========================================================================

void introSymbolicConsts(FDLContext* ctxt, Node* unit) {

    string thresholdStr = optionVal("sym-consts");
    BigConsts* constSet = new BigConsts(thresholdStr);
    string prefix;
    if (option("sym-prefix"))
        prefix = optionVal("sym-prefix");
    else
        prefix = "k___";
    constSet->setPrefix(prefix);

    unit->mapOver(* constSet);

    // Add new decls to existing decls
    vector<string> newConsts(constSet->getNewIds());
    for (int i = 0; i != (int) newConsts.size(); i++) {
        ctxt->insert(new Node( DEF_CONST,
                              newConsts.at(i),
                              Node::int_ty
                              )
                    );
    }


    // Add new rules to existing rules
    Node* newRules = constSet->getRules();
    Node* currentRules = unit->child(1);
    currentRules->appendChildren(newRules);
}

//========================================================================
// Eliminate I_SUCC and I_PRED
//========================================================================

Node* elimIntSuccPred(Node* n) {
    if (n->kind == I_SUCC) {
        n->kind = I_PLUS;
        n->addChild(new Node(NATNUM,"1"));
    }
    else if (n->kind == I_PRED) {
        n->kind = I_MINUS;
        n->addChild(new Node(NATNUM,"1"));
    }
    return n;
}

void elimIntSuccPreds(Node* unit) {
    unit->mapOver(elimIntSuccPred);
}

//========================================================================
// Abstract I_EXP and R_EXP
//========================================================================

Node* abstractExp(Node* n) {
    switch(n->kind) {
    case I_EXP: return n->updateKindAndId(FUN_AP,"int___exp");
    case R_EXP: return n->updateKindAndId(FUN_AP,"real___exp");
    default:    return n;
    }
}

void abstractExps(Node* unit) {
    unit->mapOver1(abstractExp);
}

//========================================================================
// Abstract IDIV and MOD
//========================================================================

Node* abstractDivMod(Node* n) {
    switch(n->kind) {
    case IDIV: return n->updateKindAndId(FUN_AP,"int___div");
    case MOD: return n->updateKindAndId(FUN_AP,"int___mod");
    default:    return n;
    }
}

void abstractDivMods(Node* unit) {
    unit->mapOver1(abstractDivMod);
}


//========================================================================
// Introduce axioms relating IDIV and MOD to IDIV_E and MOD_E
//========================================================================
// IDIV_E and MOD_E are the primitives supported by SMTLIB2.
/*
In email to SMTLIB mail list (11/8/08), John Harrison argued for SMTLIB2 to
use the Euclidean definitions of IDIV and MOD, as discussed in

  Raymond Boute:
  "The Euclidean definition of the functions div and mod". 
  ACM TOPLAS vol. 14 (1992), pp127-144

He used this definition for HOL Light.

Following Boute paper, we introduce explicit Kinds for each of the 
3 kinds of div and mod.

IDIV_T, MOD_T:  Division truncates, rounds towards zero.
IDIV_F, MOD_F:  Division takes integer floor, rounds towards -infinity. 
IDIV_E, MOD_E:  Euclidean division.  
                Floor if divisor +ve
                Ceiling if divisor -ve
                Ensures that 0 <= MOD_E x y < |y|.

All 3 obey remainder law:

  x = y * (x div y) + x mod y

Coming from FDL, IDIV is IDIV_T, MOD is MOD_F.
*/

// i | j  =def   j mod_e i == 0
#define nDIVIDES(i,j) nEQ(nMOD_E((j),(i)), nNATNUM("0"), nINT_TY)

// Axioms in terms of primitive IDIV and MOD, so abstraction of these 
// should occur after.

void addEuclideanIdivModAxioms(Node* unit) {

    Node* rules = unit->child(1);

    // Axioms phrased so more awkward cases are as restricted as
    // possible, they have stronger preconditions.


// y > 0 => 
//   MOD x y = MOD_E x y

    rules->addChild(
        nRULE("mod mod_e 1",
              nFORALL2("X", nINT_TY, "Y", nINT_TY,
                       nIMPLIES(nI_GT(nVAR("Y"),nNATNUM("0")),
                                nI_EQ(nMOD(nVAR("X"), nVAR("Y")),
                                      nMOD_E(nVAR("X"), nVAR("Y")))))
            ));

// y < 0 & y | x =>
//   MOD x y = MOD_E x y

    rules->addChild(
        nRULE("mod mod_e 2",
              nFORALL2("X", nINT_TY, "Y", nINT_TY,
                       nIMPLIES(nAND(nI_LT(nVAR("Y"),nNATNUM("0")),
                                     nDIVIDES(nVAR("Y"),nVAR("X"))),
                                nI_EQ(nMOD(nVAR("X"), nVAR("Y")),
                                      nMOD_E(nVAR("X"), nVAR("Y")))))
            ));



// y < 0 & ~(y | x) =>
//   MOD x y = (MOD_E x y) + y 

    rules->addChild(
        nRULE("mod mod_e 3",
              nFORALL2("X", nINT_TY, "Y", nINT_TY,
                       nIMPLIES(nAND(nI_LT(nVAR("Y"),nNATNUM("0")),
                                     nNOT(nDIVIDES(nVAR("Y"),nVAR("X")))),
                                nI_EQ(nMOD(nVAR("X"), nVAR("Y")),
                                      nI_PLUS(nMOD_E(nVAR("X"), nVAR("Y")),
                                              nVAR("Y")))))
            ));



// x >= 0 & y > 0 => 
//  IDIV x y = IDIV_E x y        Both rounding down

    rules->addChild(
        nRULE("idiv idiv_e 1",
              nFORALL2("X", nINT_TY, "Y", nINT_TY,
                       nIMPLIES(nAND(nI_GE(nVAR("X"),nNATNUM("0")),
                                     nI_GT(nVAR("Y"),nNATNUM("0"))),
                                nI_EQ(nIDIV(nVAR("X"), nVAR("Y")),
                                      nIDIV_E(nVAR("X"), nVAR("Y")))))
            ));


// x < 0 & y > 0 & y | x =>
//  IDIV x y = IDIV_E x y

    rules->addChild(
        nRULE("idiv idiv_e 2",
              nFORALL2("X", nINT_TY, "Y", nINT_TY,
                       nIMPLIES(nAND(nI_LT(nVAR("X"),nNATNUM("0")),
                                     nAND(nI_GT(nVAR("Y"),nNATNUM("0")),
                                          nDIVIDES(nVAR("Y"), nVAR("X")))),
                                nI_EQ(nIDIV(nVAR("X"), nVAR("Y")),
                                      nIDIV_E(nVAR("X"), nVAR("Y")))))
            ));

// x < 0 & y > 0 & ~(y | x) =>
//  IDIV x y = IDIV_E x y + 1     
//
// IDIV -ve so rounding up, IDIV_E rounding down

    rules->addChild(
        nRULE("idiv idiv_e 3",
              nFORALL2("X", nINT_TY, "Y", nINT_TY,
                       nIMPLIES(nAND(nI_LT(nVAR("X"),nNATNUM("0")),
                                     nAND(nI_GT(nVAR("Y"),nNATNUM("0")),
                                          nNOT(nDIVIDES(nVAR("Y"),
                                                        nVAR("X"))))),
                                nI_EQ(nIDIV(nVAR("X"), nVAR("Y")),
                                      nI_PLUS(nIDIV_E(nVAR("X"), nVAR("Y")),
                                              nNATNUM("1")))))
            ));



// x >= 0 & y < 0 => 
//  IDIV x y = IDIV_E x y        Both rounding up

    rules->addChild(
        nRULE("idiv idiv_e 4",
              nFORALL2("X", nINT_TY, "Y", nINT_TY,
                       nIMPLIES(nAND(nI_GE(nVAR("X"),nNATNUM("0")),
                                     nI_LT(nVAR("Y"),nNATNUM("0"))),
                                nI_EQ(nIDIV(nVAR("X"), nVAR("Y")),
                                      nIDIV_E(nVAR("X"), nVAR("Y")))))
            ));


// x < 0 & y < 0 & y | x =>
//  IDIV x y = IDIV_E x y

    rules->addChild(
        nRULE("idiv idiv_e 5",
              nFORALL2("X", nINT_TY, "Y", nINT_TY,
                       nIMPLIES(nAND(nI_LT(nVAR("X"),nNATNUM("0")),
                                     nAND(nI_LT(nVAR("Y"),nNATNUM("0")),
                                          nDIVIDES(nVAR("Y"), nVAR("X")))),
                                nI_EQ(nIDIV(nVAR("X"), nVAR("Y")),
                                      nIDIV_E(nVAR("X"), nVAR("Y")))))
            ));


// x < 0 & y < 0 & ~(y | x) =>
//  IDIV x y = IDIV_E x y - 1    
//
// IDIV rounding down, IDIV_E rounding up

    rules->addChild(
        nRULE("idiv idiv_e 6",
              nFORALL2("X", nINT_TY, "Y", nINT_TY,
                       nIMPLIES(nAND(nI_LT(nVAR("X"),nNATNUM("0")),
                                     nAND(nI_LT(nVAR("Y"),nNATNUM("0")),
                                          nNOT(nDIVIDES(nVAR("Y"),
                                                        nVAR("X"))))),
                                nI_EQ(nIDIV(nVAR("X"), nVAR("Y")),
                                      nI_MINUS(nIDIV_E(nVAR("X"), nVAR("Y")),
                                               nNATNUM("1")))))
            ));

    return;
}


//========================================================================
// Abstract RDIV 
//========================================================================

Node* abstractRealDiv(Node* n) {
    switch(n->kind) {
    case RDIV: return n->updateKindAndId(FUN_AP,"real___div");
    default:    return n;
    }
}

void abstractRealDivs(Node* unit) {
    unit->mapOver1(abstractRealDiv);
}

//========================================================================
// Abstract Reals
//========================================================================
// Abstract all real operators and real type

// Assume declarations added already in prelude.fdl

Node* abstractRealOpRelType(Node* n) {
    switch(n->kind) {
    case TO_REAL:  return n->updateKindAndId(FUN_AP,"int___to_real");
    case R_UMINUS: return n->updateKindAndId(FUN_AP,"real___uminus");
    case R_PLUS:   return n->updateKindAndId(FUN_AP,"real___plus");
    case R_MINUS:  return n->updateKindAndId(FUN_AP,"real___minus");
    case R_TIMES:  return n->updateKindAndId(FUN_AP,"real___times");
    case RDIV:     return n->updateKindAndId(FUN_AP,"real___div");
    case R_LT:     return n->updateKindAndId(FUN_AP,"real___lt");
    case R_LE:     return n->updateKindAndId(FUN_AP,"real___le");
    case REAL_TY:  return new Node(TYPE_ID, "real___type");
    default:       return n;
    }
}

Node* abstractRealType(Node* n) {
    switch(n->kind) {
    case REAL_TY: return new Node(TYPE_ID, "real___type");
    default:    return n;
    }
}

void abstractRealOpsRelsType(FDLContext* ctxt, Node* unit) {
    unit->mapOver1(abstractRealOpRelType);
    ctxt->termSeq.mapOver1(abstractRealType);
    ctxt->typeSeq.mapOver1(abstractRealType);
    ctxt->insert(new Node(DEF_TYPE, "real___type"));
    ctxt->insert(new Node(DECL_FUN, "real___le",
                          new Node(SEQ,
                                   new Node(TYPE_ID, "real___type"),
                                   new Node(TYPE_ID, "real___type")),
                          Node::bool_ty));
    ctxt->insert(new Node(DECL_FUN, "real___lt",
                          new Node(SEQ,
                                   new Node(TYPE_ID, "real___type"),
                                   new Node(TYPE_ID, "real___type")),
                          Node::bool_ty));
    return;
}


//========================================================================
// Define compound arithmetic transformation functions
//========================================================================

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// arithSimp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void arithSimp(FDLContext* ctxt, Node* unit) {
    
    //--------------------------------------------------------------------
    // Eliminate constants that have values supplied in rules
    //--------------------------------------------------------------------

    if (option("elim-consts")) {
        elimConsts(ctxt, unit);
    }

    //--------------------------------------------------------------------
    // Evaluate ground integer expressions.
    //--------------------------------------------------------------------

    if (option("ground-eval-exp")) {
        unit->mapOver(evalExp);
    }

    if (option("ground-eval")) {
        groundEval(unit);  // Eval all constant arith expressions, inc EXP.
    }



    //--------------------------------------------------------------------
    // Expand constant powers.
    //--------------------------------------------------------------------
    // Only expect this to be useful when prover can handle non-linear
    // arithmetic.

    if (option("expand-exp-const")) {
        unit->mapOver1(expandExpConst);
    }



    //--------------------------------------------------------------------
    // Make arithmetic expressions more linear
    //--------------------------------------------------------------------
    // Normalises expressions involving div and multiplication by constants.

    if (option("arith-eval")) constArithEval(unit);


    return;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// arithAbstract
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void
arithAbstract(FDLContext* ctxt, Node* unit) {

    //--------------------------------------------------------------------
    // Introduce symbolic constants for large integers
    //--------------------------------------------------------------------

    if (option("sym-consts")) introSymbolicConsts(ctxt, unit);

    //--------------------------------------------------------------------
    // Abstract non-linear multiplications (R_TIMES and I_TIMES) to UIFs
    //--------------------------------------------------------------------

    if (option("abstract-nonlin-times")) unit->mapOver(abstractNonLinMult);

    //--------------------------------------------------------------------
    // Replace exponent functions (I_EXP and R_EXP) by UIFs
    //--------------------------------------------------------------------

    if (option("abstract-exp")) abstractExps(unit);

    //--------------------------------------------------------------------
    // Relate (IDIV) and modulus (MOD) to Euclidean definitions
    //--------------------------------------------------------------------

    if (option("use-euclidean-divmod")) addEuclideanIdivModAxioms(unit);
    
    //--------------------------------------------------------------------
    // Replace integer division (IDIV) and modulus (MOD) by UIFs
    //--------------------------------------------------------------------

    if (option("abstract-divmod")) abstractDivMods(unit);

    //--------------------------------------------------------------------
    // Replace real division (RDIV) by UIF
    //--------------------------------------------------------------------

    if (option("abstract-real-div")) abstractRealDivs(unit);

    //--------------------------------------------------------------------
    // Abstract real operators, relations and type
    //--------------------------------------------------------------------

    if (option("abstract-reals")) abstractRealOpsRelsType(ctxt, unit);
    
    return;
}

