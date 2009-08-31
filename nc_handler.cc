
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of nc_handler, a data handler for the OPeNDAP data
// server.

// Copyright (c) 2002,2003,2006 OPeNDAP, Inc.
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

#include "config_nc.h"

static char not_used rcsid[] =
    { "$Id$" };

#include <iostream>
#include <string>

#include <DODSFilter.h>
#include <DDS.h>
#include <DAS.h>
#include <DataDDS.h>
#include <ConstraintEvaluator.h>

#include <ObjectType.h>
#include <mime_util.h>

using namespace libdap ;

extern void nc_read_variables(DAS & das, const string & filename);
extern void nc_read_descriptors(DDS & dds, const string & filename,
        bool elide_dimension_arrays);

const string cgi_version = PACKAGE_VERSION;

/** Build a DDS which holds attributes.
    @param dds A DDS constructed with the correct factory.
    @param filter The DODSFilter built using the input arguments
    @return The built-up DDS */
DDS & build_dds(DDS & dds, const DODSFilter & filter)
{
    dds.filename(filter.get_dataset_name());
    // The handler can now show/hide arrays that are also Grid Maps. But this
    // code, the CGI handler, only works the old way.
    nc_read_descriptors(dds, filter.get_dataset_name(), false);
    filter.read_ancillary_dds(dds);

    DAS das;
    nc_read_variables(das, filter.get_dataset_name());
    filter.read_ancillary_das(das);

    dds.transfer_attributes(&das);

    return dds;
}

int main(int argc, char *argv[])
{
    try {
        DODSFilter df(argc, argv);
        if (df.get_cgi_version() == "")
            df.set_cgi_version(cgi_version);

        switch (df.get_response()) {
        case DODSFilter::DAS_Response:{
                DAS das;

                nc_read_variables(das, df.get_dataset_name());
                df.read_ancillary_das(das);
                df.send_das(das);
                break;
            }

        case DODSFilter::DDS_Response:{
                DDS dds(NULL);
                dds = build_dds(dds, df);
                ConstraintEvaluator ce;
                df.send_dds(dds, ce, true);
                break;
            }

        case DODSFilter::DataDDS_Response:{
                DDS dds(NULL);
                dds = build_dds(dds, df);
                ConstraintEvaluator ce;
                df.send_data(dds, ce, cout);
                break;
            }

        case DODSFilter::DDX_Response:{
                DDS dds(NULL);
                dds = build_dds(dds, df);
                ConstraintEvaluator ce;
                df.send_ddx(dds, ce, cout);
                break;
            }

        case DODSFilter::Version_Response:{
                df.send_version_info();

                break;
            }

        default:
            df.print_usage();   // Throws Error
        }
    }
    catch(Error & e) {
        // This call replaces many other calls inside the handler code that
        // have been removed since this is where all of the exceptions
        // ultimately wind up. 1/23/09 jhrg
        ErrMsgT(e.get_error_message());

        set_mime_text(stdout, dods_error, cgi_version);
        e.print(stdout);
        return 1;
    }

    return 0;
}
