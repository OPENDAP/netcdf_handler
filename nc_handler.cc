
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
 
#include "config_nc.h"

static char not_used rcsid[]={"$Id$"};

#include <iostream>
#include <string>

#include "DODSFilter.h"
#include "DDS.h"
#include "DAS.h"
#include "DataDDS.h"

#include "ObjectType.h"
#include "cgi_util.h"

#include "NCTypeFactory.h"

extern void read_variables(DAS &das, const string &filename) throw (Error);
extern void read_descriptors(DDS &dds, const string &filename)  throw (Error);

const string cgi_version = PACKAGE_VERSION;

int 
main(int argc, char *argv[])
{
    NCTypeFactory *nctf = new NCTypeFactory;

    try { 
	DODSFilter df(argc, argv);
	if (df.get_cgi_version() == "")
	    df.set_cgi_version(cgi_version);

	switch (df.get_response()) {
	  case DODSFilter::DAS_Response: {
	    DAS das;

	    read_variables(das, df.get_dataset_name());
	    df.read_ancillary_das(das);
	    df.send_das(das);
	    break;
	  }

	  case DODSFilter::DDS_Response: {
	    DDS dds(nctf);

	    read_descriptors(dds, df.get_dataset_name());
	    df.read_ancillary_dds(dds);
	    df.send_dds(dds, true);
	    break;
	  }

	  case DODSFilter::DataDDS_Response: {
	    DDS dds(nctf);

	    dds.filename(df.get_dataset_name());
	    read_descriptors(dds, df.get_dataset_name()); 
	    df.read_ancillary_dds(dds);
	    df.send_data(dds, stdout);
	    break;
	  }

	  case DODSFilter::DDX_Response: {
	    DDS dds(nctf);
	    DAS das;

	    dds.filename(df.get_dataset_name());

	    read_descriptors(dds, df.get_dataset_name()); 
	    df.read_ancillary_dds(dds);

	    read_variables(das, df.get_dataset_name());
	    df.read_ancillary_das(das);

	    dds.transfer_attributes(&das);

	    df.send_ddx(dds, stdout);
	    break;
	  }

	  case DODSFilter::Version_Response: {
	    df.send_version_info();

	    break;
	  }

	  default:
	    df.print_usage();	// Throws Error
	}
    }
    catch (Error &e) {
	delete nctf; nctf = 0;
	set_mime_text(stdout, dods_error, cgi_version);
	e.print(stdout);
	return 1;
    }

    delete nctf; nctf = 0;
    return 0;
}

// $Log: nc_handler.cc,v $
// Revision 1.5  2005/05/23 22:09:41  jimg
// Rearrangement for automake.
//
// Revision 1.4  2005/03/31 00:04:51  jimg
// Modified to use the factory class in libdap++ 3.5.
//
// Revision 1.3  2003/12/08 18:06:37  edavis
// Merge release-3-4 into trunk
//
// Revision 1.2  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.1  2003/05/13 22:03:34  jimg
// Created. This works with newly modified DODS_Dispatch.pm script and
// DODSFilter class.
//
