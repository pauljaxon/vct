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

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <cassert>
#include <stdexcept>

#include "bignum.hh"

#include "formatter.hh"


// 2^30 = 1073741824
// 2^31 = 2147483648
// 2^32 = 4294967296

using namespace z;
int
main (int argc, char *argv[]) {
    Node* n0 = new Node(NATNUM, string("0"));
    Node* n1 = new Node(NATNUM, string("1"));
    Node* n2 = new Node(NATNUM, string("2"));
    Node* n3 = new Node(NATNUM, string("3"));
    Node* n4 = new Node(NATNUM, string("4"));
    Node* n6 = new Node(NATNUM, string("6"));
    Node* n8 = new Node(NATNUM, string("8"));
    Node* m1 = new Node(UMINUS, n1);
    Node* m2 = new Node(UMINUS, n2);
    Node* m3 = new Node(UMINUS, n3);
    Node* m6 = new Node(UMINUS, n6);
    Node* m8 = new Node(UMINUS, n8);

    MyInt i0(n0);
    MyInt i1(n1);
    MyInt i2(n2);
    MyInt i3(n3);
    MyInt i4(n4);
    MyInt i6(n6);
    MyInt i8(n8);

    MyInt j1(m1);
    MyInt j2(m2);
    MyInt j3(m3);
    MyInt j6(m6);
    MyInt j8(m8);

    assert(MyInt::uminus(i1) == j1);

    assert(MyInt::plus(i1,i2) == i3);
    assert(MyInt::plus(i3,j2) == i1);
    assert(MyInt::plus(j2,i3) == i1);

    assert(MyInt::minus(i1,i2) == j1);
    assert(MyInt::minus(i3,i1) == i2);

    assert(MyInt::times(i2,j3) == j6);

    assert(MyInt::div(i8,i3) == i2);
    assert(MyInt::div(i8,j6) == j1);

    assert(MyInt::rem(i8,i3) == i2);
    assert(MyInt::rem(i8,j3) == i2);
    assert(MyInt::rem(j8,i3) == j2);
    assert(MyInt::rem(j8,j3) == j2);

    assert(MyInt::mod(i8,i3) == i2);
    assert(MyInt::mod(i8,j3) == j1);
    assert(MyInt::mod(j8,i3) == i1);
    assert(MyInt::mod(j8,j3) == j2);

    assert(MyInt::mod(i6,i3) == i0);
    assert(MyInt::mod(i6,j3) == i0);
    assert(MyInt::mod(j6,i3) == i0);
    assert(MyInt::mod(j6,j3) == i0);

    assert(MyInt::exp(j2,j3) == i1);

    assert(MyInt::exp(j2,i3) == j8);
    assert(MyInt::exp(j2,j3) == i1);

    Node* na;
    Node* nb;
    Node* nc;
    Node* nd;

    na = new Node(PLUS, new Node(TIMES, m2, m3), n2);

    assert(! (* na == * n8) );

    groundEval(na);

    assert(* na == * n8);
    assert(! (* na == * n6) );



    Node* idW = new Node(ID, new string("w"));
    Node* idX = new Node(ID, new string("x"));
    Node* idY = new Node(ID, new string("y"));
    Node* idZ = new Node(ID, new string("z"));

    // [(x * 3) * 4 ] div 6  -->  2 * x
    na = new Node(IDIV,
                 new Node(TIMES,
                          new Node (TIMES, idX, n3),
                          n4),
                 n6
                 );

    constArithEval(na);
    
    Formatter::setFormatter(VanillaFormatter::getFormatter());
    //    cout << "Node na = " << * na << endl;

    nb = new Node(TIMES, n2, idX);

    //    cout << "Node nb = " << * nn << endl;
    assert(* na == * nb);

    // [(2 * x)  * y ] * [z * (w * 3)] --> 6 * [(x * y) * (z * w)]

    nc= new Node (TIMES,
                  new Node (TIMES,
                            new Node (TIMES, n2, idX),
                            idY),
                  new Node (TIMES,
                            idZ,
                            new Node (TIMES, idW, n3)
                            )
                  );
                   
    nd= new Node (TIMES,
                  n6,
                  new Node (TIMES,
                            new Node (TIMES, idX, idY),
                            new Node (TIMES, idZ, idW)
                            )
                  );

    assert ( ! (* nc == * nd));

    constArithEval(nc);
    
    assert (* nc == * nd);

    cout << "All tests passed" << endl;
    return 0;
}
