
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of nc_handler, a data handler for the OPeNDAP data
// server. 

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
// 
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.
 
#include <string>

#include "NCByte.h"
#include "NCInt16.h"
#include "NCUInt16.h"
#include "NCInt32.h"
#include "NCUInt32.h"
#include "NCFloat32.h"
#include "NCFloat64.h"
#include "NCStr.h"
#include "NCUrl.h"
#include "NCArray.h"
#include "NCStructure.h"
#include "NCSequence.h"
#include "NCGrid.h"

#include "NCTypeFactory.h"
#include "debug.h"

Byte *
NCTypeFactory::NewByte(const string &n ) const 
{ 
    return new NCByte(n);
}

Int16 *
NCTypeFactory::NewInt16(const string &n ) const 
{ 
    return new NCInt16(n); 
}

UInt16 *
NCTypeFactory::NewUInt16(const string &n ) const 
{ 
    return new NCUInt16(n);
}

Int32 *
NCTypeFactory::NewInt32(const string &n ) const 
{ 
    DBG(cerr << "Inside NCTypeFactory::NewInt32" << endl);
    return new NCInt32(n);
}

UInt32 *
NCTypeFactory::NewUInt32(const string &n ) const 
{ 
    return new NCUInt32(n);
}

Float32 *
NCTypeFactory::NewFloat32(const string &n ) const 
{ 
    return new NCFloat32(n);
}

Float64 *
NCTypeFactory::NewFloat64(const string &n ) const 
{ 
    return new NCFloat64(n);
}

Str *
NCTypeFactory::NewStr(const string &n ) const 
{ 
    return new NCStr(n);
}

Url *
NCTypeFactory::NewUrl(const string &n ) const 
{ 
    return new NCUrl(n);
}

Array *
NCTypeFactory::NewArray(const string &n , BaseType *v) const 
{ 
    return new NCArray(n, v);
}

Structure *
NCTypeFactory::NewStructure(const string &n ) const 
{ 
    return new NCStructure(n);
}

Sequence *
NCTypeFactory::NewSequence(const string &n ) const 
{ 
    DBG(cerr << "Inside NCTypeFactory::NewSequence" << endl);
    return new NCSequence(n);
}

Grid *
NCTypeFactory::NewGrid(const string &n ) const 
{ 
    return new NCGrid(n);
}
