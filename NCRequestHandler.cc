
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
#include "NCTypeFactory.h"
#include "BESResponseHandler.h"
#include "BESResponseNames.h"
#include "BESDASResponse.h"
#include "BESDDSResponse.h"
#include "BESDataDDSResponse.h"
#include "BESConstraintFuncs.h"
#include "BESVersionInfo.h"
#include "Error.h"
#include "BESDapHandlerException.h"
#include "BESDataNames.h"
#include "BESContextManager.h"

#include "config_nc.h"
#include "ncdas.h"
#include "ncdds.h"

NCRequestHandler::NCRequestHandler(string name)
:  BESRequestHandler(name)
{
    add_handler(DAS_RESPONSE, NCRequestHandler::nc_build_das);
    add_handler(DDS_RESPONSE, NCRequestHandler::nc_build_dds);
    add_handler(DATA_RESPONSE, NCRequestHandler::nc_build_data);
    add_handler(HELP_RESPONSE, NCRequestHandler::nc_build_help);
    add_handler(VERS_RESPONSE, NCRequestHandler::nc_build_version);
}

NCRequestHandler::~NCRequestHandler()
{
}

bool NCRequestHandler::nc_build_das(BESDataHandlerInterface & dhi)
{
    BESResponseObject *response =
        dhi.response_handler->get_response_object();
    BESDASResponse *bdas = dynamic_cast < BESDASResponse * >(response);
    DAS *das = bdas->get_das();

    // determine if this is a dap2 response format, where there won't be
    // multiple files. If there are no multiple files (this is a dap2
    // response format) then the data structures won't be nested
    bool found = false ;
    bool multi = true ;
    string val = BESContextManager::TheManager()->get_context( "format", found ) ;
    if( val == "dap2" )
    {
	multi = false ;
    }

    try {
        nc_read_variables(*das, dhi.container->access(),
	                  dhi.container->get_symbolic_name(), multi);
    }
    catch(Error & e) {
        BESDapHandlerException ex( e.get_error_message(), __FILE__, __LINE__,
				   e.get_error_code() ) ;
        throw ex;
    }
    catch(...) {
        string s = "unknown exception caught building DAS";
        BESHandlerException ex(s, __FILE__, __LINE__);
        throw ex;
    }

    return true;
}

bool NCRequestHandler::nc_build_dds(BESDataHandlerInterface & dhi)
{
    BESResponseObject *response =
        dhi.response_handler->get_response_object();
    BESDDSResponse *bdds = dynamic_cast < BESDDSResponse * >(response);
    DDS *dds = bdds->get_dds();

    // determine if this is a dap2 response format, where there won't be
    // multiple files. If there are no multiple files (this is a dap2
    // response format) then the data structures won't be nested
    bool found = false ;
    bool multi = true ;
    string val = BESContextManager::TheManager()->get_context( "format", found ) ;
    if( val == "dap2" )
    {
	multi = false ;
    }

    try {
        NCTypeFactory *factory = new NCTypeFactory;
        dds->set_factory(factory);

        dds->filename(dhi.container->access());
        nc_read_descriptors(*dds, dhi.container->access(),
	                    dhi.container->get_symbolic_name(), multi);
        DAS das;
        nc_read_variables(das, dhi.container->access(),
			  dhi.container->get_symbolic_name(), multi);

        dds->transfer_attributes(&das);

	BESConstraintFuncs::post_append( dhi, multi ) ;
        //dhi.data[POST_CONSTRAINT] = dhi.container->get_constraint();
#if 0
        // see ticket 720
        dds->set_factory(NULL);
        delete factory;
#endif
    }
    catch(Error & e) {
        BESDapHandlerException ex( e.get_error_message(), __FILE__, __LINE__,
				   e.get_error_code() ) ;
        throw ex;
    }
    catch(...) {
        string s = "unknown exception caught building DDS";
        BESHandlerException ex(s, __FILE__, __LINE__);
        throw ex;
    }

    return true;
}

bool NCRequestHandler::nc_build_data(BESDataHandlerInterface & dhi)
{
    BESResponseObject *response =
        dhi.response_handler->get_response_object();
    BESDataDDSResponse *bdds =
        dynamic_cast < BESDataDDSResponse * >(response);
    DataDDS *dds = bdds->get_dds();

    // determine if this is a dap2 response format, where there won't be
    // multiple files. If there are no multiple files (this is a dap2
    // response format) then the data structures won't be nested
    bool found = false ;
    bool multi = true ;
    string val = BESContextManager::TheManager()->get_context( "format", found ) ;
    if( val == "dap2" )
    {
	multi = false ;
    }

    try {
        NCTypeFactory *factory = new NCTypeFactory;
        dds->set_factory(factory);

        dds->filename(dhi.container->access());
        nc_read_descriptors(*dds, dhi.container->access(),
	                    dhi.container->get_symbolic_name(), multi);
        DAS das;
        nc_read_variables(das, dhi.container->access(),
			  dhi.container->get_symbolic_name(), multi);

        dds->transfer_attributes(&das);

	BESConstraintFuncs::post_append( dhi, multi ) ;
        //dhi.data[POST_CONSTRAINT] = dhi.container->get_constraint();

#if 0
        // see ticket 720
        dds->set_factory(NULL);
        delete factory;
#endif
    }
    catch(Error & e) {
        BESDapHandlerException ex( e.get_error_message(), __FILE__, __LINE__,
				   e.get_error_code() ) ;
        throw ex;
    }
    catch(...) {
        string s = "unknown exception caught building DAS";
        BESHandlerException ex(s, __FILE__, __LINE__);
        throw ex;
    }

    return true;
}

bool NCRequestHandler::nc_build_help(BESDataHandlerInterface & dhi)
{
    BESInfo *info =
        (BESInfo *) dhi.response_handler->get_response_object();
    info->begin_tag("Handler");
    info->add_tag("name", PACKAGE_NAME);
    string handles = (string) DAS_RESPONSE
        + "," + DDS_RESPONSE
        + "," + DATA_RESPONSE + "," + HELP_RESPONSE + "," + VERS_RESPONSE;
    info->add_tag("handles", handles);
    info->add_tag("version", PACKAGE_STRING);
    info->end_tag("Handler");

    return true;
}

bool NCRequestHandler::nc_build_version(BESDataHandlerInterface & dhi)
{
    BESVersionInfo *info =
        (BESVersionInfo *) dhi.response_handler->get_response_object();
    info->addHandlerVersion(PACKAGE_NAME, PACKAGE_VERSION);
    return true;
}
