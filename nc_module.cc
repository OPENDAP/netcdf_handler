
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
#include "DODSRequestHandlerList.h"
#include "NCRequestHandler.h"
#include "ContainerStorageList.h"
#include "ContainerStorageCatalog.h"
#include "DODSLog.h"
#include "DirectoryCatalog.h"
#include "CatalogList.h"

#define NC_NAME "nc"
#define NC_CATALOG_ROOT_KEY "Catalog.catalog.RootDirectory"

static bool
NCInit(int, char**)
{
    if( DODSLog::TheLog()->is_verbose() )
	(*DODSLog::TheLog()) << "Initializing NC:" << endl ;

    if( DODSLog::TheLog()->is_verbose() )
	(*DODSLog::TheLog()) << "    adding " << NC_NAME << " request handler" 
		      << endl ;
    DODSRequestHandlerList::TheList()->add_handler( NC_NAME, new NCRequestHandler( NC_NAME ) ) ;

    if( DODSLog::TheLog()->is_verbose() )
	(*DODSLog::TheLog()) << "    adding " << NC_NAME << " catalog" 
		      << endl ;
    CatalogList::TheCatalogList()->add_catalog( new DirectoryCatalog( NC_CATALOG_ROOT_KEY ) ) ;

    if( DODSLog::TheLog()->is_verbose() )
	(*DODSLog::TheLog()) << "Adding Catalog Container Storage" << endl;
    ContainerStorageCatalog *csc = new ContainerStorageCatalog( "catalog" ) ;
    ContainerStorageList::TheList()->add_persistence( csc ) ;

    return true ;
}

static bool
NCTerm(void)
{
    if( DODSLog::TheLog()->is_verbose() )
	(*DODSLog::TheLog()) << "Removing NC Handlers" << endl;
    DODSRequestHandler *rh = DODSRequestHandlerList::TheList()->remove_handler( NC_NAME ) ;
    if( rh ) delete rh ;

    if( DODSLog::TheLog()->is_verbose() )
	(*DODSLog::TheLog()) << "Removing catalog Container Storage" << endl;
    ContainerStorageList::TheList()->rem_persistence( "catalog" ) ;

    return true ;
}

FUNINITQUIT( NCInit, NCTerm, 3 ) ;

