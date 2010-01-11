
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

#include "NCRequestHandler.h"
#include <BESResponseHandler.h>
#include <BESResponseNames.h>
#include <BESDapNames.h>
#include <BESDASResponse.h>
#include <BESDDSResponse.h>
#include <BESDataDDSResponse.h>
#include <BESVersionInfo.h>
#include <InternalErr.h>
#include <BESDapError.h>
#include <BESInternalFatalError.h>
#include <BESDataNames.h>
#include <TheBESKeys.h>
#include <BESUtil.h>
#include <Ancillary.h>
#include <BESServiceRegistry.h>
#include <BESUtil.h>

#include "config_nc.h"

#define NC_NAME "nc"

using namespace libdap ;

bool NCRequestHandler::_show_shared_dims = false ;
bool NCRequestHandler::_show_shared_dims_set = false ;

extern void nc_read_variables(DAS & das,
			      const string & filename) throw(Error);
extern void nc_read_descriptors(DDS & dds, const string & filename,
        bool elide_dimension_arrays);

NCRequestHandler::NCRequestHandler(const string &name)
:  BESRequestHandler(name)
{
    add_handler(DAS_RESPONSE, NCRequestHandler::nc_build_das);
    add_handler(DDS_RESPONSE, NCRequestHandler::nc_build_dds);
    add_handler(DATA_RESPONSE, NCRequestHandler::nc_build_data);
    add_handler(HELP_RESPONSE, NCRequestHandler::nc_build_help);
    add_handler(VERS_RESPONSE, NCRequestHandler::nc_build_version);

    if( NCRequestHandler::_show_shared_dims_set == false )
    {
        bool found = false ;
	string key = "NC.ShowSharedDimensions" ;
        string doset ;
	TheBESKeys::TheKeys()->get_value( key, doset, found ) ;
	if( found )
	{
	    doset = BESUtil::lowercase( doset ) ;
	    if( doset == "true" || doset == "yes" )
	    {
		NCRequestHandler::_show_shared_dims = true ;
	    }
	}
	NCRequestHandler::_show_shared_dims_set = true ;
    }
}

NCRequestHandler::~NCRequestHandler()
{
}

bool NCRequestHandler::nc_build_das(BESDataHandlerInterface & dhi)
{
    BESResponseObject *response = dhi.response_handler->get_response_object() ;
    BESDASResponse *bdas = dynamic_cast < BESDASResponse * >(response) ;
    if( !bdas )
	throw BESInternalError( "cast error", __FILE__, __LINE__ ) ;
    try {
	bdas->set_container( dhi.container->get_symbolic_name() ) ;
	DAS *das = bdas->get_das();
	string accessed = dhi.container->access();
        nc_read_variables(*das, accessed);
	Ancillary::read_ancillary_das( *das, accessed ) ;
	bdas->clear_container( ) ;
    }
    catch( BESError &e ) {
	throw e ;
    }
    catch(InternalErr & e) {
        BESDapError ex( e.get_error_message(), true, e.get_error_code(),
	                __FILE__, __LINE__ ) ;
        throw ex;
    }
    catch(Error & e) {
        BESDapError ex( e.get_error_message(), false, e.get_error_code(),
	                __FILE__, __LINE__ ) ;
        throw ex;
    }
    catch(...) {
        string s = "unknown exception caught building DAS";
        BESInternalFatalError ex(s, __FILE__, __LINE__);
        throw ex;
    }

    return true;
}

bool NCRequestHandler::nc_build_dds(BESDataHandlerInterface & dhi)
{
    BESResponseObject *response = dhi.response_handler->get_response_object();
    BESDDSResponse *bdds = dynamic_cast < BESDDSResponse * >(response);
    if( !bdds )
	throw BESInternalError( "cast error", __FILE__, __LINE__ ) ;
    try {
	bdds->set_container( dhi.container->get_symbolic_name() ) ;
	DDS *dds = bdds->get_dds();
	string accessed = dhi.container->access() ;
        dds->filename( accessed );

        bool elide_dimension_arrays = !(NCRequestHandler::_show_shared_dims);
        nc_read_descriptors(*dds, accessed, elide_dimension_arrays);
	Ancillary::read_ancillary_dds( *dds, accessed ) ;

        DAS *das = new DAS ;
	BESDASResponse bdas( das ) ;
	bdas.set_container( dhi.container->get_symbolic_name() ) ;
        nc_read_variables( *das, accessed ) ;
	Ancillary::read_ancillary_das( *das, accessed ) ;

        dds->transfer_attributes( das ) ;

	bdds->set_constraint( dhi ) ;

	bdds->clear_container( ) ;
    }
    catch( BESError &e ) {
	throw e ;
    }
    catch(InternalErr & e) {
        BESDapError ex( e.get_error_message(), true, e.get_error_code(),
	                __FILE__, __LINE__ ) ;
        throw ex;
    }
    catch(Error & e) {
        BESDapError ex( e.get_error_message(), false, e.get_error_code(),
	                __FILE__, __LINE__ ) ;
        throw ex;
    }
    catch(...) {
        string s = "unknown exception caught building DDS";
        BESInternalFatalError ex(s, __FILE__, __LINE__);
        throw ex;
    }

    return true;
}

bool NCRequestHandler::nc_build_data(BESDataHandlerInterface & dhi)
{
    BESResponseObject *response = dhi.response_handler->get_response_object();
    BESDataDDSResponse *bdds = dynamic_cast < BESDataDDSResponse * >(response);
    if( !bdds )
	throw BESInternalError( "cast error", __FILE__, __LINE__ ) ;

    try {
	bdds->set_container( dhi.container->get_symbolic_name() ) ;
	DataDDS *dds = bdds->get_dds();
	string accessed = dhi.container->access() ;
        dds->filename(accessed);

        bool elide_dimension_arrays = !(NCRequestHandler::_show_shared_dims);
        nc_read_descriptors(*dds, accessed, elide_dimension_arrays);
	Ancillary::read_ancillary_dds( *dds, accessed ) ;

        DAS *das = new DAS ;
	BESDASResponse bdas( das ) ;
	bdas.set_container( dhi.container->get_symbolic_name() ) ;
        nc_read_variables( *das, accessed ) ;
	Ancillary::read_ancillary_das( *das, accessed ) ;

        dds->transfer_attributes( das ) ;

	bdds->set_constraint( dhi ) ;

	bdds->clear_container( ) ;
    }
    catch( BESError &e ) {
	throw e ;
    }
    catch(InternalErr & e) {
        BESDapError ex( e.get_error_message(), true, e.get_error_code(),
	                __FILE__, __LINE__ ) ;
        throw ex;
    }
    catch(Error & e) {
        BESDapError ex( e.get_error_message(), false, e.get_error_code(),
	                __FILE__, __LINE__ ) ;
        throw ex;
    }
    catch(...) {
        string s = "unknown exception caught building DAS";
        BESInternalFatalError ex(s, __FILE__, __LINE__);
        throw ex;
    }

    return true;
}

bool NCRequestHandler::nc_build_help(BESDataHandlerInterface & dhi)
{
    BESResponseObject *response = dhi.response_handler->get_response_object();
    BESInfo *info = dynamic_cast<BESInfo *>(response);
    if( !info )
	throw BESInternalError( "cast error", __FILE__, __LINE__ ) ;

    map<string,string> attrs ;
    attrs["name"] = PACKAGE_NAME ;
    attrs["version"] = PACKAGE_VERSION ;
    list<string> services ;
    BESServiceRegistry::TheRegistry()->services_handled( NC_NAME, services );
    if( services.size() > 0 )
    {
	string handles = BESUtil::implode( services, ',' ) ;
	attrs["handles"] = handles ;
    }
    info->begin_tag( "module", &attrs ) ;
    info->end_tag( "module" ) ;

    return true;
}

bool NCRequestHandler::nc_build_version(BESDataHandlerInterface & dhi)
{
    BESResponseObject *response = dhi.response_handler->get_response_object();
    BESVersionInfo *info = dynamic_cast < BESVersionInfo * >(response);
    if( !info )
	throw BESInternalError( "cast error", __FILE__, __LINE__ ) ;
  
    info->add_module( PACKAGE_NAME, PACKAGE_VERSION ) ;

    return true;
}
