//==========================================================================
//==========================================================================
// PDRIVER.CC
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



#include "pdriver.hh"
#include "parser.tab.hh"

using namespace std;

pdriver::pdriver ()
  : trace_scanning (false), trace_parsing (false)
{
}

pdriver::~pdriver ()
{
}

int
pdriver::parse (const string &f)
{
  file = f;
  errorFlag = false;

  scan_begin ();
  if (errorFlag) return 1;

  yy::parser parser (*this);  // Construct a new parser object
  parser.set_debug_level (trace_parsing);
  int pval = parser.parse ();
  scan_end ();
  return pval;
}

void
pdriver::error (const std::string& m)
{
  std::cerr << m << std::endl;
  errorFlag = true;
}
