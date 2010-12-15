//========================================================================
//========================================================================
// PROCESSOR.HH
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

// Intermediate solver-independent processing of FDL unit components.

#ifndef PROCESSOR_HH
#define PROCESSOR_HH

#include "node.hh"

std::string elaborateUnit(Node* unit);

#endif // ! defined PROCESSOR_HH
