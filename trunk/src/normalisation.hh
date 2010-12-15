//========================================================================
//========================================================================
// NORMALISATION.HH
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

#ifndef NORMALISATION_HH
#define NORMALISATION_HH

#include "node.hh"
#include "context.hh"

FDLContext* 
putUnitInStandardForm(Node* unit);

#endif // ! defined NORMALISATION_HH
