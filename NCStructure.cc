
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
 

// (c) COPYRIGHT URI/MIT 1994-1996
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      reza            Reza Nekovei (reza@intcomm.net)

// netCDF sub-class implementation for NCByte,...NCGrid.
// The files are patterned after the subcalssing examples 
// Test<type>.c,h files.
//
// ReZa 1/12/95

#include "config_nc.h"

static char rcsid[] not_used ={"$Id$"};

#include <algorithm>

#include <netcdf.h>
#include <InternalErr.h>

#include "NCStructure.h"

BaseType *
NCStructure::ptr_duplicate()
{
    return new NCStructure(*this);
}

NCStructure::NCStructure(const string &n, const string &d) : Structure(n, d)
{
}

NCStructure::NCStructure(const NCStructure &rhs) : Structure(rhs)
{
}

NCStructure::~NCStructure()
{
}

NCStructure &
NCStructure::operator=(const NCStructure &rhs)
{
    if (this == &rhs)
        return *this;

    dynamic_cast<Structure&>(*this) = rhs; // run Structure assignment

    return *this;
}

/** When flattening a Structure, make sure to add an attribute to the 
    new variables that indicate they are the product of translation. */

class AddAttribute: public unary_function<BaseType *, void> {
    
public:
    AddAttribute() {}

    void operator()(BaseType *a) {
        AttrTable *at;
        AttrTable::Attr_iter aiter;
        a->get_attr_table().find("translation", &at, &aiter);
        if (a->get_attr_table().attr_end() == aiter) {
            a->get_attr_table().append_attr("translation", "String",
                                            "\"flatten\"");
        }
    }
};

/**
 * Transfer attributes from a separately built DAS to the DDS. This method
 * overrides the implementation found in libdap to accommodate the special
 * characteristics of the NC handler's DAS object. The noteworthy feature
 * of this handler's DAS is that it lacks the specified structure that
 * provides an easy way to match DAS and DDS items. Instead this DAS is
 * flat.
 *
 * Because this handler produces a flat DAS, the nested structures (like
 * NCStructure) pass the entire top level AttrTable to each of their children
 * so that they can search for their attribute table. The default implementation
 * of this method would find the Structure's table and pass only that to the
 * children since it 'knows' that their tables would all be found within it.
 *
 * @param at An AttrTable for the entire DAS. Search this for attributes
 * by name.
 * @see NCSequence::transfer_attributes
 * @see NCGrid::transfer_attributes
 * @see NCArray::transfer_attributes
 */
void NCStructure::transfer_attributes(AttrTable *at)
{
    if (at) {
        Vars_iter var = var_begin();
        while (var != var_end()) {
            (*var)->transfer_attributes(at);
            var++;
        }
    }
}


