
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
 
// NCContainerPersistence.h

#ifndef I_NCContainerPersistence_h_
#define I_NCContainerPersistence_h_ 1

#include <string>

using std::string ;

#include "DODSContainerPersistenceVolatile.h"

class NCContainerPersistence : public DODSContainerPersistenceVolatile
{
private:
    string			_nc_base ;
public:
    				NCContainerPersistence( const string &n);
    virtual			~NCContainerPersistence() ;

    virtual void		add_container( string s_name, string r_ame,
					       string type ) ;
};

#endif // I_NCContainerPersistence_h_

// $Log: NCContainerPersistence.h,v $
