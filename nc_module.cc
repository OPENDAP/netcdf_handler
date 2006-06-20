
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

#include "BESInitList.h"
#include "BESRequestHandlerList.h"
#include "NCRequestHandler.h"
#include "ContainerStorageList.h"
#include "ContainerStorageCatalog.h"
#include "BESLog.h"
#include "DirectoryCatalog.h"
#include "CatalogList.h"

#define NC_NAME "nc"
#define NC_CATALOG "catalog"

static bool
NCInit(int, char**)
{
    if( BESLog::TheLog()->is_verbose() )
	(*BESLog::TheLog()) << "Initializing NC:" << endl ;

    if( BESLog::TheLog()->is_verbose() )
	(*BESLog::TheLog()) << "    adding " << NC_NAME << " request handler" 
		      << endl ;
    BESRequestHandlerList::TheList()->add_handler( NC_NAME, new NCRequestHandler( NC_NAME ) ) ;

    if( BESLog::TheLog()->is_verbose() )
	(*BESLog::TheLog()) << "    adding " << NC_NAME << " catalog" 
		      << endl ;
    CatalogList::TheCatalogList()->add_catalog( new DirectoryCatalog( NC_CATALOG ) ) ;

    if( BESLog::TheLog()->is_verbose() )
	(*BESLog::TheLog()) << "Adding Catalog Container Storage" << endl;
    ContainerStorageCatalog *csc = new ContainerStorageCatalog( NC_CATALOG ) ;
    ContainerStorageList::TheList()->add_persistence( csc ) ;

    return true ;
}

static bool
NCTerm(void)
{
    if( BESLog::TheLog()->is_verbose() )
	(*BESLog::TheLog()) << "Removing NC Handlers" << endl;
    BESRequestHandler *rh = BESRequestHandlerList::TheList()->remove_handler( NC_NAME ) ;
    if( rh ) delete rh ;

    if( BESLog::TheLog()->is_verbose() )
	(*BESLog::TheLog()) << "Removing catalog Container Storage" << endl;
    ContainerStorageList::TheList()->rem_persistence( "catalog" ) ;

    return true ;
}

// Global initialization. Call NCInit when initializing and NCTerm when shutting
// down. The '3' means these are run after functions registered in groups 1 and 
// 2. FUNINITQUIT is a macro defined in BESInitList.h
FUNINITQUIT( NCInit, NCTerm, 3 ) ;

