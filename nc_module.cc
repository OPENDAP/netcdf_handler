
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
 
// cdf_module.cc

#include <iostream>

using std::endl ;

#include "DODSInitList.h"
#include "TheRequestHandlerList.h"
#include "NCRequestHandler.h"
#include "TheDODSLog.h"

#define NC_NAME "nc"

static bool
NCInit(int, char**)
{
    if( TheDODSLog->is_verbose() )
	(*TheDODSLog) << "Initializing NC:" << endl ;

    if( TheDODSLog->is_verbose() )
	(*TheDODSLog) << "    adding " << NC_NAME << " request handler" 
		      << endl ;
    TheRequestHandlerList->add_handler( NC_NAME,
					new NCRequestHandler( NC_NAME ) ) ;

    return true ;
}

static bool
NCTerm(void)
{
    if( TheDODSLog->is_verbose() )
	(*TheDODSLog) << "Removing NC Handlers" << endl;
    DODSRequestHandler *rh = TheRequestHandlerList->remove_handler( NC_NAME ) ;
    if( rh ) delete rh ;
    return true ;
}

FUNINITQUIT( NCInit, NCTerm, 3 ) ;

