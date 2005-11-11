
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
 
// NCRequestHandler.cc

#include <iostream>
#include <fstream>

using std::ifstream ;
using std::cerr ;
using std::endl ;

#include "NCRequestHandler.h"
#include "NCTypeFactory.h"
#include "DODSResponseHandler.h"
#include "DODSResponseNames.h"
#include "DAS.h"
#include "DDS.h"
#include "DODSConstraintFuncs.h"
#include "DODSVersionInfo.h"
#include "DODSResponseException.h"
#include "OPeNDAPDataNames.h"

extern void read_variables(DAS &das, const string &filename) throw (Error);
extern void read_descriptors(DDS &dds, const string &filename)  throw (Error);

NCRequestHandler::NCRequestHandler( string name )
    : DODSRequestHandler( name )
{
    add_handler( DAS_RESPONSE, NCRequestHandler::nc_build_das ) ;
    add_handler( DDS_RESPONSE, NCRequestHandler::nc_build_dds ) ;
    add_handler( DATA_RESPONSE, NCRequestHandler::nc_build_data ) ;
    add_handler( HELP_RESPONSE, NCRequestHandler::nc_build_help ) ;
    add_handler( VERS_RESPONSE, NCRequestHandler::nc_build_version ) ;
    add_handler( NODES_RESPONSE, NCRequestHandler::nc_build_nodes ) ;
    add_handler( LEAVES_RESPONSE, NCRequestHandler::nc_build_leaves ) ;
}

NCRequestHandler::~NCRequestHandler()
{
}

bool
NCRequestHandler::nc_build_das( DODSDataHandlerInterface &dhi )
{
    DAS *das = (DAS *)dhi.response_handler->get_response_object() ;

    read_variables( *das, dhi.container->get_real_name() );

    return true ;
}

bool
NCRequestHandler::nc_build_dds( DODSDataHandlerInterface &dhi )
{
    DDS *dds = (DDS *)dhi.response_handler->get_response_object() ;
    NCTypeFactory *factory = new NCTypeFactory ;
    dds->set_factory( factory ) ;

    read_descriptors( *dds, dhi.container->get_real_name() );
    dhi.data[POST_CONSTRAINT] = dhi.container->get_constraint();

    dds->set_factory( NULL ) ;
    delete factory ;

    return true ;
}

bool
NCRequestHandler::nc_build_data( DODSDataHandlerInterface &dhi )
{
    DDS *dds = (DDS *)dhi.response_handler->get_response_object() ;
    NCTypeFactory *factory = new NCTypeFactory ;
    dds->set_factory( factory ) ;

    dds->filename( dhi.container->get_real_name() );
    read_descriptors( *dds, dhi.container->get_real_name() ); 
    dhi.data[POST_CONSTRAINT] = dhi.container->get_constraint();

    dds->set_factory( NULL ) ;
    delete factory ;

    return true ;
}

bool
NCRequestHandler::nc_build_help( DODSDataHandlerInterface &dhi )
{
    DODSInfo *info = (DODSInfo *)dhi.response_handler->get_response_object() ;
    info->add_data( (string)"No help currently available for netCDF handler.\n" ) ;

    return true ;
}

bool
NCRequestHandler::nc_build_version( DODSDataHandlerInterface &dhi )
{
    DODSVersionInfo *info = (DODSVersionInfo *)dhi.response_handler->get_response_object() ;
    info->addHandlerVersion( "libnc-dods", "0.9" ) ;
    return true ;
}

bool
NCRequestHandler::nc_build_nodes( DODSDataHandlerInterface &dhi )
{
    DODSInfo *info = (DODSInfo *)dhi.response_handler->get_response_object() ;
    info->add_data( "adding nodes\n" ) ;
    return true ;
}

bool
NCRequestHandler::nc_build_leaves( DODSDataHandlerInterface &dhi )
{
    DODSInfo *info = (DODSInfo *)dhi.response_handler->get_response_object() ;
    info->add_data( "adding leaves\n" ) ;
    return true ;
}

