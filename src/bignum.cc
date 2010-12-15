//==========================================================================
//==========================================================================
// BIGNUM.CC
//==========================================================================
//==========================================================================
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



#include "bignum.hh"
using std::string;
using namespace z;


#include <stdexcept>
#include <iostream>
using std::endl;

#include "utility.hh"


//========================================================================
// MyInt functions
//========================================================================
// GMP documentation is vague about how storage for mpz_class objects is
// managed.  Assume some kind of reference counting going on.

// Assume n satisfies isInt(). 

// id of NATNUM{id} is guaranteed string of digits by lexer, so no need
// to check this.

// 
MyInt::MyInt(const std::string& s) {
    num = mpz_class(s);
}

MyInt::MyInt(Node* n) {

    Node* nat = (n->kind == I_UMINUS) ? n->child(0) : n;
    num = mpz_class(nat->id);

    if (n->kind == I_UMINUS) num = num * -1;
}

bool 
MyInt::isInt(Node* n) {
    return (n->kind == NATNUM ||
            (n->kind == I_UMINUS && n->child(0)->kind == NATNUM)
            );
    }


Node* MyInt::toNode() {

    if (num < 0) {
        return new Node(I_UMINUS, new Node(NATNUM,
                                         mpz_class(num * (-1)).get_str()));
    } else {
        return new Node(NATNUM, num.get_str());
    }
}

bool 
MyInt::operator==(const MyInt& a) const {
    return num == a.num;
}

bool 
MyInt::operator<(const MyInt& a) const {
    return num < a.num;
}

std::string 
MyInt::toString() const {
    return num.get_str();
}


MyInt
MyInt::uminus(const MyInt& a) {

    return MyInt(a.num * (-1));
}

MyInt
MyInt::plus(const MyInt& a, const MyInt& b) {
    return MyInt(a.num + b.num);
}

MyInt
MyInt::minus(const MyInt& a, const MyInt& b) {
    return MyInt(a.num - b.num);
}

MyInt
MyInt::times(const MyInt& a, const MyInt& b) {
    return MyInt(a.num * b.num);
}

// Named gmp functions used for clarity.

MyInt
MyInt::div(const MyInt& a, const MyInt& b) {
    if (b.num == 0) 
        throw std::runtime_error("MyInt::div: divide by zero");
    
    mpz_class q;    
    // tdiv_q returns quotient when rounding (t = truncating) towards zero.
    mpz_tdiv_q (q.get_mpz_t(), a.num.get_mpz_t(), b.num.get_mpz_t());
    return MyInt(q);
}

// Sign of modulus here follows that of 1st arg.
MyInt
MyInt::rem(const MyInt& a, const MyInt& b) {
    if (b.num == 0) 
        throw std::runtime_error("MyInt::rem: second argument zero");
    mpz_class r;    
    // tdiv_r returns remainder when division is rounding (t =
    // truncating) towards zero.
    mpz_tdiv_r (r.get_mpz_t(), a.num.get_mpz_t(), b.num.get_mpz_t());
    return MyInt(r);
}

// mpz_mod is *not* used here.  It always ignores sign of divisor and
// always returns a +ve result.

// Mimic Ada functionality where sign of modulus follows that of 2nd arg.
MyInt
MyInt::mod(const MyInt& a, const MyInt& b) {
    if (b.num == 0) 
        throw std::runtime_error("MyInt::mod: second argument zero");

    mpz_class r;    
    // tdiv_r returns remainder when division is rounding (t =
    // truncating) towards zero.
    mpz_tdiv_r (r.get_mpz_t(), a.num.get_mpz_t(), b.num.get_mpz_t());
    if (r != 0 && ((a.num > 0 && b.num < 0) || (a.num < 0 && b.num > 0)))
        r = r + b.num;
    return MyInt(r);

}

// Returns 1 if b <= 0;
MyInt
MyInt::exp(const MyInt& a, const MyInt& b) {
    if (b.num <= 0) return MyInt(mpz_class(1));
    if (b.num > mpz_class(256)) // Exact value unimportant.  Make smaller?
        throw std::runtime_error("MyInt::exp: too large exponent "
                                 + b.num.get_str());
    mpz_class val;    
    mpz_pow_ui(val.get_mpz_t(), a.num.get_mpz_t(), b.num.get_ui());
    return MyInt(val);
}



//========================================================================
// Elimination of ground integer arithmetic expressions
//========================================================================


void groundEvalTop(Node* n) {
    int k = n->kind;
    switch (k) {
    case I_PLUS:
    case I_MINUS:
    case I_TIMES:
    case IDIV:
    case MOD:
    case I_EXP: {
        if (MyInt::isInt(n->child(0))
            && MyInt::isInt(n->child(1)) ) {

            MyInt a(n->child(0));
            MyInt b(n->child(1));
            MyInt res;
            if (k == I_PLUS) res = MyInt::plus(a,b);
            else if (k == I_MINUS) res = MyInt::minus(a,b);
            else if (k == I_TIMES) res = MyInt::times(a,b);
            else if (k == IDIV) res = MyInt::div(a,b);
            else if (k == MOD) res = MyInt::mod(a,b);
            else res = MyInt::exp(a,b);
            *n = * res.toNode();
        }
        break;
    }
    case I_UMINUS: {
        if (MyInt::isInt(n->child(0))) {

            MyInt a(n->child(0));
            MyInt res = MyInt::uminus(a);
            *n = * res.toNode();
        }
        break;
    }
    default:
        break;
    }
    return;
}


void groundEval(Node* n) {
    n->mapOver(groundEvalTop);
}


//========================================================================
// Evaluation of constant arith exps and mult and div by constants
//========================================================================
//
// Rules:  apply 1st that matches.

//
//   e * k  --> k * e                          (lin-times-1)
// 
//   k * (k' * e) --> kk' * e                  (lin-times-2)
//   (k * e) * k' --> kk' * e
//
// 
//   (k * e) * (k' * e') -->  kk' * (e * e')   (lin-times-3)
//   e * (k * e') -->  k * (e * e')            (lin-times-4)
//  (k * e) * e'  -->  k * (e * e')            (lin-times-5)
// 
// 
// 
// (k * e) div k' -->   (k div k') * e    if k' divides k.  (lin-times-div)
//
// Intent is that rules are applied in bottom up 


bool isIntNode(Node* n) {
    return MyInt::isInt(n);
}

// Require here that linear multiplication be always of form "k * e".

bool isLinTimesNode(Node* n) {
    return (n->kind == I_TIMES && isIntNode(n->child(0)));
}

void constArithEvalTop(Node* n) {
    groundEvalTop(n);

    // Applying this rule first reduces need for 2 versions of next rule
    // e * k  --> k * e
    if (n->kind == I_TIMES && isIntNode(n->child(1)) ) {
        Node* e = n->child(0);
        Node* k = n->child(1);

        printMessage(FINEm, "Applying arith-eval rule lin-times-1");

        n->child(0) = k;
        n->child(1) = e;
    } // NB: No else here.

    // k1 * (k2 * e) --> k1k2 * e

    if (isLinTimesNode(n)
        && isLinTimesNode(n->child(1))) {
        
        printMessage(FINEm, "Applying arith-eval rule lin-times-2");

        MyInt k1(n->child(0));
        MyInt k2(n->child(1)->child(0));
        Node* e = n->child(1)->child(1);

        n->child(0) = MyInt::times(k1,k2).toNode();
        n->child(1) = e;
    }
    //   (k * e) * (k' * e') -->  kk' * (e * e') 

    else if (n->kind == I_TIMES
             && isLinTimesNode(n->child(0))
             && isLinTimesNode(n->child(1)) ) {

        printMessage(FINEm, "Applying arith-eval rule lin-times-3");

        MyInt k1(n->child(0)->child(0));
        MyInt k2(n->child(1)->child(0));
        Node* e = n->child(0)->child(1);

        n->child(0) = MyInt::times(k1,k2).toNode();
        n->child(1)->child(0) = e;

    }
    //   e * (k * e') -->  k * (e * e') 
    else if (n->kind == I_TIMES
             && isLinTimesNode(n->child(1)) ) {

        printMessage(FINEm, "Applying arith-eval rule lin-times-4");

        Node* e = n->child(0);
        Node* k = n->child(1)->child(0);

        n->child(0) = k;
        n->child(1)->child(0) = e;
        
    }
    //  (k * e1) * e2  -->  k * (e1 * e2) 
    else if (n->kind == I_TIMES
             && isLinTimesNode(n->child(0)) ) {

        Node* k = n->child(0)->child(0);
        Node* e1 = n->child(0)->child(1);
        Node* e2 = n->child(1);

        printMessage(FINEm, "Applying arith-eval rule lin-times-5");

        n->child(1) = n->child(0);
        n->child(0) = k;
        n->child(1)->child(0) = e1;
        n->child(1)->child(1) = e2;
        
    }
    // (k1 * e) div k2 -->   (k1 div k2) * e    if k2 divides k1
    else if (n->kind == IDIV
             && isLinTimesNode(n->child(0))
             && isIntNode(n->child(1))
               ) {

        printMessage(FINEm, "Applying arith-eval rule lin-times-div");
        
        MyInt k1(n->child(0)->child(0));
        MyInt k2(n->child(1));
        if (MyInt::rem(k1, k2) == MyInt(new Node(NATNUM,"0")) ) {
            n->child(1) = n->child(0)->child(1);
            n->child(0) = MyInt::div(k1,k2).toNode();
            n->kind = I_TIMES;
        }
    }
    return;
}

void constArithEval(Node* n) {
    n->mapOver(constArithEvalTop);
}

