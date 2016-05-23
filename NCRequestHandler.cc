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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

// NCRequestHandler.cc

#include "config_nc.h"

#include <string>
#include <sstream>
#include <exception>

#include <DMR.h>
#include <mime_util.h>
#include <D4BaseTypeFactory.h>

#include <BESResponseHandler.h>
#include <BESResponseNames.h>
#include <BESDapNames.h>
#include <BESDASResponse.h>
#include <BESDDSResponse.h>
#include <BESDataDDSResponse.h>
#include <BESVersionInfo.h>

#include <BESDapError.h>
#include <BESInternalFatalError.h>
#include <BESDataNames.h>
#include <TheBESKeys.h>
#include <BESServiceRegistry.h>
#include <BESUtil.h>
#include <BESDebug.h>
#include <BESStopWatch.h>
#include <BESContextManager.h>
#include <BESDMRResponse.h>

#include <ObjMemCache.h>

#include <InternalErr.h>
#include <Ancillary.h>

#include "NCRequestHandler.h"

#define NC_NAME "nc"

using namespace libdap;

bool NCRequestHandler::_show_shared_dims = true;
bool NCRequestHandler::_show_shared_dims_set = false;

bool NCRequestHandler::_ignore_unknown_types = false;
bool NCRequestHandler::_ignore_unknown_types_set = false;

bool NCRequestHandler::_promote_byte_to_short = false;
bool NCRequestHandler::_promote_byte_to_short_set = false;

unsigned int NCRequestHandler::_das_cache_entries = 100;
float NCRequestHandler::_das_cache_purge_level = 0.2;

ObjMemCache *NCRequestHandler::das_cache = 0;

extern void nc_read_dataset_attributes(DAS & das, const string & filename);
extern void nc_read_dataset_variables(DDS & dds, const string & filename);

/** Is the version number string greater than or equal to the value.
 * @note Works only for versions with zero or one dot. If the conversion of
 * the string to a float fails for any reason, this returns false.
 * @param version The string value (e.g., 3.2)
 * @param value A floating point value.
 */
static bool version_ge(const string &version, float value)
{
    try {
        float v;
        istringstream iss(version);
        iss >> v;
        //cerr << "version: " << v << ", value: " << value << endl;
        return (v >= value);
    }
    catch (...) {
        return false;
    }

    return false; // quiet warnings...
}

/**
 * Stolen from the HDF5 handler code
 */
static bool get_bool_key(const string &key, bool def_val)
{
    bool found = false;
    string doset = "";
    const string dosettrue = "true";
    const string dosetyes = "yes";

    TheBESKeys::TheKeys()->get_value(key, doset, found);
    if (true == found) {
        doset = BESUtil::lowercase(doset);
        return (dosettrue == doset || dosetyes == doset);
    }
    return def_val;
}

static unsigned int get_uint_key(const string &key, unsigned int def_val)
{
    bool found = false;
    string doset = "";

    TheBESKeys::TheKeys()->get_value(key, doset, found);
    if (true == found) {
        return atoi(doset.c_str()); // use better code TODO
    }
    else {
        return def_val;
    }
}

static float get_float_key(const string &key, float def_val)
{
    bool found = false;
    string doset = "";

    TheBESKeys::TheKeys()->get_value(key, doset, found);
    if (true == found) {
        return atof(doset.c_str()); // use better code TODO
    }
    else {
        return def_val;
    }
}

NCRequestHandler::NCRequestHandler(const string &name) :
    BESRequestHandler(name)
{
    BESDEBUG("nc", "In NCRequestHandler::NCRequestHandler" << endl);

    add_handler(DAS_RESPONSE, NCRequestHandler::nc_build_das);
    add_handler(DDS_RESPONSE, NCRequestHandler::nc_build_dds);
    add_handler(DATA_RESPONSE, NCRequestHandler::nc_build_data);

    add_handler(DMR_RESPONSE, NCRequestHandler::nc_build_dmr);
    add_handler(DAP4DATA_RESPONSE, NCRequestHandler::nc_build_dmr);

    add_handler(HELP_RESPONSE, NCRequestHandler::nc_build_help);
    add_handler(VERS_RESPONSE, NCRequestHandler::nc_build_version);

    // TODO replace with get_bool_key above 5/21/16 jhrg

    // Look for the SHowSharedDims property, if it has not been set
    if (NCRequestHandler::_show_shared_dims_set == false) {
        bool key_found = false;
        string doset;
        TheBESKeys::TheKeys()->get_value("NC.ShowSharedDimensions", doset, key_found);
        if (key_found) {
            // It was set in the conf file
            NCRequestHandler::_show_shared_dims_set = true;

            doset = BESUtil::lowercase(doset);
            if (doset == "true" || doset == "yes") {
                NCRequestHandler::_show_shared_dims = true;
            }
            else
                NCRequestHandler::_show_shared_dims = false;
        }
    }

    if (NCRequestHandler::_ignore_unknown_types_set == false) {
        bool key_found = false;
        string doset;
        TheBESKeys::TheKeys()->get_value("NC.IgnoreUnknownTypes", doset, key_found);
        if (key_found) {
            doset = BESUtil::lowercase(doset);
            if (doset == "true" || doset == "yes")
                NCRequestHandler::_ignore_unknown_types = true;
            else
                NCRequestHandler::_ignore_unknown_types = false;

            NCRequestHandler::_ignore_unknown_types_set = true;
        }
    }

    if (NCRequestHandler::_promote_byte_to_short_set == false) {
        bool key_found = false;
        string doset;
        TheBESKeys::TheKeys()->get_value("NC.PromoteByteToShort", doset, key_found);
        if (key_found) {
            doset = BESUtil::lowercase(doset);
            if (doset == "true" || doset == "yes")
                NCRequestHandler::_promote_byte_to_short = true;
            else
                NCRequestHandler::_promote_byte_to_short = false;

            NCRequestHandler::_promote_byte_to_short_set = true;
        }
    }

    NCRequestHandler::_das_cache_entries = get_uint_key("NC.DASCacheEntries", 0);
    NCRequestHandler::_das_cache_purge_level = get_float_key("NC.DASCachePurgeLevel", 0.2);

    if (get_das_cache_entries())    // else it stays at its default of null
        das_cache = new ObjMemCache();

    BESDEBUG("nc", "Exiting NCRequestHandler::NCRequestHandler" << endl);
}

NCRequestHandler::~NCRequestHandler()
{
}

bool NCRequestHandler::nc_build_das(BESDataHandlerInterface & dhi)
{
	BESStopWatch sw;
	if (BESISDEBUG( TIMING_LOG ))
		sw.start("NCRequestHandler::nc_build_das", dhi.data[REQUEST_ID]);

    BESDEBUG("nc", "In NCRequestHandler::nc_build_das" << endl);

    BESResponseObject *response = dhi.response_handler->get_response_object();
    BESDASResponse *bdas = dynamic_cast<BESDASResponse *> (response);
    if (!bdas)
        throw BESInternalError("cast error", __FILE__, __LINE__);

    try {
        bdas->set_container(dhi.container->get_symbolic_name());
        DAS *das = bdas->get_das();
        string accessed = dhi.container->access();

        // Look in memory cache if it's initialized
        DAS *cached_das_ptr = 0;
        if (das_cache && (cached_das_ptr = static_cast<DAS*>(das_cache->get(accessed)))) {
            // copy the cached DAS into the BES response object
            BESDEBUG("nc", "DAS Cached hit for : " << accessed << endl);
            *das = *cached_das_ptr;
        }
        else {
            nc_read_dataset_attributes(*das, accessed);
            Ancillary::read_ancillary_das(*das, accessed);
            // Purge cache and then add this to the cache (if its init'd)
            if (das_cache) {
                if (das_cache->size() > get_das_cache_entries()) das_cache->purge(get_das_cache_purge_level());
                // add a copy
                das_cache->add(new DAS(*das), accessed);
            }
        }

        bdas->clear_container();
    }
    catch (BESError &e) {
        throw;
    }
    catch (InternalErr & e) {
        BESDapError ex(e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
        throw ex;
    }
    catch (Error & e) {
        BESDapError ex(e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
        throw ex;
    }
    catch (std::exception &e) {
        string s = string("C++ Exception: ") + e.what();
        BESInternalFatalError ex(s, __FILE__, __LINE__);
        throw ex;
    }
    catch (...) {
        string s = "unknown exception caught building DAS";
        BESInternalFatalError ex(s, __FILE__, __LINE__);
        throw ex;
    }

    BESDEBUG("nc", "Exiting NCRequestHandler::nc_build_das" << endl);
    return true;
}

bool NCRequestHandler::nc_build_dds(BESDataHandlerInterface & dhi)
{

	BESStopWatch sw;
	if (BESISDEBUG( TIMING_LOG ))
		sw.start("NCRequestHandler::nc_build_dds", dhi.data[REQUEST_ID]);

    BESResponseObject *response = dhi.response_handler->get_response_object();
    BESDDSResponse *bdds = dynamic_cast<BESDDSResponse *> (response);
    if (!bdds)
        throw BESInternalError("cast error", __FILE__, __LINE__);

    try {
        // If there's no value for this set in the conf file, look at the context
        // and set the default behavior based on the protocol version clients say
        // they will accept.
        if (NCRequestHandler::_show_shared_dims_set == false) {
            bool context_found = false;
            string context_value = BESContextManager::TheManager()->get_context("xdap_accept", context_found);
            if (context_found) {
                BESDEBUG("nc", "xdap_accept: " << context_value << endl);
                if (version_ge(context_value, 3.2))
                    NCRequestHandler::_show_shared_dims = false;
                else
                    NCRequestHandler::_show_shared_dims = true;
            }
        }

        bdds->set_container(dhi.container->get_symbolic_name());
        DDS *dds = bdds->get_dds();
        string accessed = dhi.container->access();
        dds->filename(accessed);

        nc_read_dataset_variables(*dds, accessed);

        BESDEBUG("nc", "NCRequestHandler::nc_build_dds, accessed: " << accessed << endl);
#if 0
        Ancillary::read_ancillary_dds(*dds, accessed);
#endif

        DAS *das = new DAS;
        BESDASResponse bdas(das);
        bdas.set_container(dhi.container->get_symbolic_name());
        nc_read_dataset_attributes(*das, accessed);
        Ancillary::read_ancillary_das(*das, accessed);

        BESDEBUG("nc", "Prior to dds->transfer_attributes" << endl);

        dds->transfer_attributes(das);

#if 0
        ConstraintEvaluator & ce = bdds->get_ce();
        ce.add_function("ugrid_restrict", function_ugrid_restrict);
#endif

        bdds->set_constraint(dhi);

        bdds->clear_container();
    }
    catch (BESError &e) {
        throw e;
    }
    catch (InternalErr & e) {
        BESDapError ex(e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
        throw ex;
    }
    catch (Error & e) {
        BESDapError ex(e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
        throw ex;
    }
    catch (std::exception &e) {
        string s = string("C++ Exception: ") + e.what();
        BESInternalFatalError ex(s, __FILE__, __LINE__);
        throw ex;
    }
    catch (...) {
        string s = "unknown exception caught building DDS";
        BESInternalFatalError ex(s, __FILE__, __LINE__);
        throw ex;
    }

    return true;
}

bool NCRequestHandler::nc_build_data(BESDataHandlerInterface & dhi)
{
	BESStopWatch sw;
	if (BESISDEBUG( TIMING_LOG ))
		sw.start("NCRequestHandler::nc_build_data", dhi.data[REQUEST_ID]);

    BESResponseObject *response = dhi.response_handler->get_response_object();
    BESDataDDSResponse *bdds = dynamic_cast<BESDataDDSResponse *> (response);
    if (!bdds)
        throw BESInternalError("cast error", __FILE__, __LINE__);

    try {
        if (NCRequestHandler::_show_shared_dims_set == false) {
            bool context_found = false;
            string context_value = BESContextManager::TheManager()->get_context("xdap_accept", context_found);
            if (context_found) {
                BESDEBUG("nc", "xdap_accept: " << context_value << endl);
                if (version_ge(context_value, 3.2))
                    NCRequestHandler::_show_shared_dims = false;
                else
                    NCRequestHandler::_show_shared_dims = true;
            }
        }

        bdds->set_container(dhi.container->get_symbolic_name());
        DDS *dds = bdds->get_dds();
        string accessed = dhi.container->access();
        dds->filename(accessed);

        nc_read_dataset_variables(*dds, accessed);

#if 0
        Ancillary::read_ancillary_dds(*dds, accessed);
#endif
        DAS *das = new DAS;
        BESDASResponse bdas(das);
        bdas.set_container(dhi.container->get_symbolic_name());
        nc_read_dataset_attributes(*das, accessed);
        Ancillary::read_ancillary_das(*das, accessed);

        dds->transfer_attributes(das);

#if 0
        ConstraintEvaluator & ce = bdds->get_ce();
        ce.add_function("ugrid_restrict", function_ugrid_restrict);
#endif

        bdds->set_constraint(dhi);
        bdds->clear_container();
    }
    catch (BESError &e) {
        throw;
    }
    catch (InternalErr & e) {
        BESDapError ex(e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
        throw ex;
    }
    catch (Error & e) {
        BESDapError ex(e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
        throw ex;
    }
    catch (std::exception &e) {
        string s = string("C++ Exception: ") + e.what();
        BESInternalFatalError ex(s, __FILE__, __LINE__);
        throw ex;
    }
    catch (...) {
        string s = "unknown exception caught building DAS";
        BESInternalFatalError ex(s, __FILE__, __LINE__);
        throw ex;
    }

    return true;
}

bool NCRequestHandler::nc_build_dmr(BESDataHandlerInterface &dhi)
{

	BESStopWatch sw;
	if (BESISDEBUG( TIMING_LOG ))
		sw.start("NCRequestHandler::nc_build_dmr", dhi.data[REQUEST_ID]);

	// Because this code does not yet know how to build a DMR directly, use
	// the DMR ctor that builds a DMR using a 'full DDS' (a DDS with attributes).
	// First step, build the 'full DDS'
	string data_path = dhi.container->access();

	BaseTypeFactory factory;
	DDS dds(&factory, name_path(data_path), "3.2");
	dds.filename(data_path);

	try {
		nc_read_dataset_variables(dds, data_path);

		DAS das;
		nc_read_dataset_attributes(das, data_path);
		Ancillary::read_ancillary_das(das, data_path);
		dds.transfer_attributes(&das);
	}
	catch (InternalErr &e) {
		throw BESDapError(e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
	}
	catch (Error &e) {
		throw BESDapError(e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
	}
	catch (...) {
		throw BESDapError("Caught unknown error build NC DMR response", true, unknown_error, __FILE__, __LINE__);
	}

	// Extract the DMR Response object - this holds the DMR used by the
	// other parts of the framework.
	BESResponseObject *response = dhi.response_handler->get_response_object();
	BESDMRResponse &bdmr = dynamic_cast<BESDMRResponse &>(*response);

	// Get the DMR made by the BES in the BES/dap/BESDMRResponseHandler, make sure there's a
	// factory we can use and then dump the DAP2 variables and attributes in using the
	// BaseType::transform_to_dap4() method that transforms individual variables
	DMR *dmr = bdmr.get_dmr();
	dmr->set_factory(new D4BaseTypeFactory);
	dmr->build_using_dds(dds);

	// Instead of fiddling with the internal storage of the DHI object,
	// (by setting dhi.data[DAP4_CONSTRAINT], etc., directly) use these
	// methods to set the constraints. But, why? Ans: from Patrick is that
	// in the 'container' mode of BES each container can have a different
	// CE.
	bdmr.set_dap4_constraint(dhi);
	bdmr.set_dap4_function(dhi);

	// What about async and store_result? See BESDapTransmit::send_dap4_data()

	return true;
}

bool NCRequestHandler::nc_build_help(BESDataHandlerInterface & dhi)
{
	BESStopWatch sw;
	if (BESISDEBUG( TIMING_LOG ))
		sw.start("NCRequestHandler::nc_build_help", dhi.data[REQUEST_ID]);

    BESResponseObject *response = dhi.response_handler->get_response_object();
    BESInfo *info = dynamic_cast<BESInfo *> (response);
    if (!info)
        throw BESInternalError("cast error", __FILE__, __LINE__);

    map < string, string > attrs;
    attrs["name"] = MODULE_NAME ;
    attrs["version"] = MODULE_VERSION ;
#if 0
    attrs["name"] = PACKAGE_NAME;
    attrs["version"] = PACKAGE_VERSION;
#endif
    list < string > services;
    BESServiceRegistry::TheRegistry()->services_handled(NC_NAME, services);
    if (services.size() > 0) {
        string handles = BESUtil::implode(services, ',');
        attrs["handles"] = handles;
    }
    info->begin_tag("module", &attrs);
    info->end_tag("module");

    return true;
}

bool NCRequestHandler::nc_build_version(BESDataHandlerInterface & dhi)
{
	BESStopWatch sw;
	if (BESISDEBUG( TIMING_LOG ))
		sw.start("NCRequestHandler::nc_build_version", dhi.data[REQUEST_ID]);

    BESResponseObject *response = dhi.response_handler->get_response_object();
    BESVersionInfo *info = dynamic_cast<BESVersionInfo *> (response);
    if (!info)
        throw BESInternalError("cast error", __FILE__, __LINE__);

#if 0
    info->add_module(PACKAGE_NAME, PACKAGE_VERSION);
#endif
    info->add_module(MODULE_NAME, MODULE_VERSION);

    return true;
}
