
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
#include "DODSResponseHandler.h"
#include "DODSResponseNames.h"
#if 0
#include "NCreadAttributes.h"
#include "NCreadDescriptors.h"
#endif
#include "DAS.h"
#include "DDS.h"
#include "DODSConstraintFuncs.h"
#include "DODSInfo.h"
#include "TheDODSKeys.h"
#include "DODSResponseException.h"

#if 0
#include "version.h"
#endif

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
}

NCRequestHandler::~NCRequestHandler()
{
}

bool
NCRequestHandler::nc_build_das( DODSDataHandlerInterface &dhi )
{
    DAS *das = (DAS *)dhi.response_handler->get_response_object() ;

    read_variables( *das, dhi.container->get_real_name() );

#if 0
    read_variables(das, df.get_dataset_name());
    df.read_ancillary_das(das);
#endif
    
#if 0
    if( !readAttributes( *das, dhi.container->get_real_name() ) )
    {
	throw DODSResponseException( "NC could not build the DAS response" ) ;
    }
#endif

    return true ;
}

bool
NCRequestHandler::nc_build_dds( DODSDataHandlerInterface &dhi )
{
    DDS *dds = (DDS *)dhi.response_handler->get_response_object() ;

    read_descriptors( *dds, dhi.container->get_real_name() );
    dds->parse_constraint( dhi.container->get_constraint() );

#if 0
    read_descriptors(dds, df.get_dataset_name());
    df.read_ancillary_dds(dds);
#endif

#if 0
    if( !readDescriptors( *dds, dhi.container->get_real_name(),
			  dhi.container->get_symbolic_name() ) )
    {
	throw DODSResponseException( "NC could not build the DDS response" ) ;
    }
    DODSConstraintFuncs::post_append( dhi ) ;
#endif

    return true ;
}

bool
NCRequestHandler::nc_build_data( DODSDataHandlerInterface &dhi )
{
    DDS *dds = (DDS *)dhi.response_handler->get_response_object() ;

    dds->filename( dhi.container->get_real_name() );
    read_descriptors( *dds, dhi.container->get_real_name() ); 
#if 0
    // There's no post_constraint field in the struct
    // DODSDataHandlerInterface (See the header in bes/dispatch. I'm not sure
    // what this does anyway. jhrg 9/13/05
    dhi.post_constraint = dhi.container->get_constraint();
#endif

#if 0
    df.read_ancillary_dds(dds);
    df.send_data(dds, stdout);
#endif
#if 0
    DDS *dds = (DDS *)dhi.response_handler->get_response_object() ;
#endif
#if 0
    if( !readDescriptors( *dds, dhi.container->get_real_name(),
			  dhi.container->get_symbolic_name() ) )
    {
	throw DODSResponseException( "NC could not build the DDS response" ) ;
    }
    DODSConstraintFuncs::post_append( dhi ) ;
#endif

    return true ;
}

bool
NCRequestHandler::nc_build_help( DODSDataHandlerInterface &dhi )
{
    DODSInfo *info = (DODSInfo *)dhi.response_handler->get_response_object() ;
    info->add_data( (string)"No help for netCDF handler.\n" ) ;

#if 0
    info->add_data( (string)"cdf-dods help: " + nc_version() + "\n" ) ;
    bool found = false ;
    string key = (string)"NC.Help." + dhi.transmit_protocol ;
    string file = TheDODSKeys->get_key( key, found ) ;
    if( found == false )
    {
	info->add_data( "no help information available for cdf-dods\n" ) ;
    }
    else
    {
	ifstream ifs( file.c_str() ) ;
	if( !ifs )
	{
	    info->add_data( "cdf-dods help file not found, help information not available\n" ) ;
	}
	else
	{
	    char line[4096] ;
	    while( !ifs.eof() )
	    {
		ifs.getline( line, 4096 ) ;
		if( !ifs.eof() )
		{
		    info->add_data( line ) ;
		    info->add_data( "\n" ) ;
		}
	    }
	    ifs.close() ;
	}
    }
#endif

    return true ;
}

bool
NCRequestHandler::nc_build_version( DODSDataHandlerInterface &dhi )
{
    DODSInfo *info = (DODSInfo *)dhi.response_handler->get_response_object() ;
    info->add_data( (string)"    0.9\n" ) ;
    info->add_data( (string)"    libnc-dods 0.9\n" ) ;
#if 0
    info->add_data( (string)"    " + nc_version() + "\n" ) ;
    info->add_data( (string)"        libcdf2.7\n" ) ;
#endif
    return true ;
}

