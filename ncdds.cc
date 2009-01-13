
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


// (c) COPYRIGHT URI/MIT 1994-1996
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      reza            Reza Nekovei (reza@intcomm.net)

// This file contains functions which read the variables and their description
// from a netcdf file and build the in-memeory DDS. These functions form the
// core of the server-side software necessary to extract the DDS from a
// netcdf data file.
//
// It also contains test code which will print the in-memeory DDS to
// stdout.
//
// ReZa 10/20/94

#include "config_nc.h"

static char not_used rcsid[]={"$Id$"};

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#include <netcdf.h>

#include <DDS.h>
#include <cgi_util.h>
#include <util.h>

#include "NCInt32.h"
#include "NCUInt32.h"
#include "NCUInt16.h"
#include "NCInt16.h"
#include "NCFloat64.h"
#include "NCFloat32.h"
#include "NCByte.h"
#include "NCArray.h"
#include "NCGrid.h"
#include "NCStr.h"

using namespace libdap ;

// Used by ErrMsgT

static char Msgt[255];

// This function returns the appropriate DODS BaseType for the given
// netCDF data type.
//

static BaseType *
Get_bt(const string &varname, const string &dataset, nc_type datatype)
{
    switch (datatype) {
        case NC_CHAR:
            return (new NCStr(varname, dataset));

        case NC_BYTE:
            return (new NCByte(varname, dataset));

        case NC_SHORT:
            return (new NCInt16(varname, dataset));

        case NC_LONG:
            return (new NCInt32(varname, dataset));

        case NC_FLOAT:
            return (new NCFloat32(varname, dataset));

        case NC_DOUBLE:
            return (new NCFloat64(varname, dataset));

        default: // Maybe this should be an error? jhrg 1/12/09
            return (new NCStr(varname, dataset));
    }
}

// Build a grid given that one has been found. The Grid's Array is already
// allocated and is passed in along with a number of arrays containing
// information about the dimensions of the Grid.
//
// Note: The dim_szs and dim_nms arrays could be removed since that information
// is already in the Array ar.

static Grid *build_grid(Array *ar,
        const int ndims,
        char *var_match[MAX_NC_VARS],
        const nc_type typ_match[MAX_NC_VARS],
        const int dim_szs[MAX_VAR_DIMS],
        const char dim_nms[MAX_VAR_DIMS][MAX_NC_NAME])
{
    const string &filename = ar->dataset();
    Grid *gr = new NCGrid(ar->name(), filename);
    gr->add_var(ar, array);

    for (int d = 0; d < ndims; ++d) {
        BaseType *local_bt = Get_bt(var_match[d], filename, typ_match[d]);
        NCArray *local_ar = new NCArray(local_bt->name(), filename, local_bt);
        delete local_bt;
        ar->append_dim(dim_szs[d], dim_nms[d]);
        gr->add_var(local_ar, maps);
        delete local_ar;
    }

    return gr;
}

// Read given number of variables (nvars) from the opened netCDF file
// (ncid) and add them with their appropriate type and dimensions to
// the given instance of the DDS class.
//
// Returns: false if an error accessing the netcdf file was detected, true
// otherwise.

static void
read_class(DDS &dds_table, const string &filename, int ncid, int nvars)
{
    // add all the variables in this file to DDS table
    for (int v1 = 0; v1 < nvars; ++v1) {
        char varname1[MAX_NC_NAME];
        nc_type nctype;
        int ndims;
        int dim_ids[MAX_VAR_DIMS];

        int errstat = nc_inq_var(ncid, v1, varname1, &nctype, &ndims, dim_ids,
                (int *) 0);
        if (errstat != NC_NOERR) {
            string msg = "netcdf 3: could not get name or dimension number for variable ";
            msg += long_to_string(v1);
            throw Error(msg);
        }

        BaseType *bt = Get_bt(varname1, filename, nctype);

        // is an Atomic-class ?
        if (ndims == 0) {
            dds_table.add_var(bt);
            delete bt;
        }
        else {       // Grid vs. Array type matching
            int dim_match = 0;
            int dim_szs[MAX_VAR_DIMS];
            char dim_nms[MAX_VAR_DIMS][MAX_NC_NAME];

            char *var_match[MAX_NC_VARS];
            nc_type typ_match[MAX_NC_VARS];

            // match all the dimensions of this variable to other variables
            for (int d = 0; d < ndims; ++d) {
                char dimname[MAX_NC_NAME];
                size_t dim_sz;

                errstat = nc_inq_dim(ncid, dim_ids[d], dimname, &dim_sz);
                if (errstat != NC_NOERR) {
                    string msg = "netcdf 3: could not get size for dimension ";
                    msg += long_to_string(d);
                    msg += " in variable ";
                    msg += long_to_string(v1);
                    throw Error(msg);
                }
                dim_szs[d] = (int) dim_sz;
                strncpy(dim_nms[d], dimname, MAX_NC_NAME-1);

                // Look at every variable in the data set to see if there's
                // a one dimensional array that is the map for the current
                // variable.
                for (int v2 = 0; v2 < nvars; ++v2) {
                    char varname2[MAX_NC_VARS][MAX_NC_NAME];
                    int ndims2;
                    int tmp_dim_ids[MAX_VAR_DIMS];

                    errstat = nc_inq_var(ncid, v2, varname2[v2], &nctype,
                            &ndims2, tmp_dim_ids, (int *) 0);
                    if (errstat != NC_NOERR) {
                        string msg = "netcdf 3: could not get name or dimension number for variable ";
                        msg += long_to_string(v1);
                        throw Error(msg);
                    }

                    // Is it a Grid ?     1) variable name = the dimension name
                    //                    2) The variable has only one dimension
                    //                    3) It is not itself
                    //                    4) They are the same size
                    if ((v1 != v2) && (strcmp(dimname, varname2[v2]) == 0)
                            && (ndims2 == 1)) {
                        size_t tmp_sz;

                        errstat = nc_inq_dim(ncid, tmp_dim_ids[0],
                                (char *) 0, &tmp_sz);
                        if (errstat != NC_NOERR) {
                            string msg = "netcdf 3: could not get size for dimension ";
                            msg += long_to_string(d);
                            msg += " in variable ";
                            msg += long_to_string(v1);
                            throw Error(msg);
                        }
                        if (tmp_sz == dim_sz) {
                            typ_match[dim_match] = nctype;
                            var_match[dim_match] = varname2[v2];
                            dim_match++;
                            break; // Stop var search, matching variable
                            // for the given dimension was found
                        }
                    }
                }

                // Stop dimensions search, the variable does not
                // fit into a grid, due to a dimension mis-match.
                // Also, get the size for the remainder of the
                // dimensions in the variable.
                if (dim_match != d + 1) {
                    for (int d2 = d + 1; d2 < ndims; ++d2) {
                        char dimname2[MAX_NC_NAME];

                        errstat = nc_inq_dim(ncid, dim_ids[d2], dimname2,
                                &dim_sz);
                        if (errstat != NC_NOERR) {
                            string msg = "netcdf 3: could not get size for dimension ";
                            msg += long_to_string(d);
                            msg += " in variable ";
                            msg += long_to_string(v1);
                            throw Error(msg);
                        }
                        dim_szs[d2] = (int) dim_sz;
                        strncpy(dim_nms[d2], dimname2, MAX_NC_NAME-1);
                    }
                    break;
                }
            }

            // Create common array for both Array and Grid types

            // Here is where the handler makes an array of NCStr objects
            // when it has found an array of NC_CHAR. This would need
            // to change the rep of NC_CHAR arrays to String. 1/8/09 jhrg
            Array *ar = new NCArray(varname1, filename, bt);
            delete bt;
            for (int d = 0; d < ndims; ++d)
                ar->append_dim(dim_szs[d], dim_nms[d]);

            if (ndims == dim_match) { // Found Grid type, add it
                Grid *gr = build_grid(ar, ndims, var_match, typ_match, dim_szs, dim_nms);
                dds_table.add_var(gr);
                delete ar;
                delete gr;
            }
            else { // must be an Array, add it
                dds_table.add_var(ar);
                delete ar;
            }
        }
    }
}

// Given a reference to an instance of class DDS and a filename that refers
// to a netcdf file, read the netcdf file and extract all the dimensions of
// each of its variables. Add the variables and their dimensions to the
// instance of DDS.
//
// Returns: false if an error accessing the netcdf file was detected, true
// otherwise.

void nc_read_descriptors(DDS &dds_table, const string &filename)
{
    ncopts = 0;
    int ncid, errstat;
    int nvars;

    errstat = nc_open(filename.c_str(), NC_NOWRITE, &ncid);
    if (errstat != NC_NOERR) {
        //local error
        snprintf(Msgt, 255, "netCDF server: Could not open file %s ",
                filename.c_str());
        ErrMsgT(Msgt); //local error message
        string msg = (string) "Could not open " + path_to_filename(filename)
                + ".";
        throw Error(errstat, msg);
    }

    // how many variables?
    errstat = nc_inq_nvars(ncid, &nvars);
    if (errstat != NC_NOERR) {
        ErrMsgT("Could not inquire about netcdf file (ncdds)");
        string msg = (string) "Could not inquire about netcdf file: "
                + path_to_filename(filename) + ".";
        throw Error(errstat, msg);
    }

    // dataset name
    dds_table.set_dataset_name(name_path(filename));

    // read variables class
    try {
        read_class(dds_table, filename, ncid, nvars); // remove error.
    }
    catch (Error &e) {
        ErrMsgT(e.get_error_message().c_str());
        throw e;
    }

    if (nc_close(ncid) != NC_NOERR)
        throw InternalErr(__FILE__, __LINE__,
                "ncdds: Could not close the dataset!");
}

#ifdef TEST

int
main(int argc, char *argv[])
{
    DDS dds;

    try {
        nc_read_descriptors(dds, (string)argv[1]);
        dds.print();
    }
    catch (Error &e) {
        e.display_message();
        return 1;
    }
}

#endif

// $Log: ncdds.cc,v $
// Revision 1.8  2005/04/19 23:16:18  jimg
// Removed client side parts; the client library is now in libnc-dap.
//
// Revision 1.7  2005/03/31 00:04:51  jimg
// Modified to use the factory class in libdap++ 3.5.
//
// Revision 1.6  2003/12/08 18:06:37  edavis
// Merge release-3-4 into trunk
//
// Revision 1.5  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.4.4.1  2003/06/06 08:23:41  reza
// Updated the servers to netCDF-3 and fixed error handling between client and server.
//
// Revision 1.4  2003/01/28 07:08:24  jimg
// Merged with release-3-2-8.
//
// Revision 1.2.4.2  2002/12/18 23:40:33  pwest
// gcc3.2 compile corrections, mainly regarding using statements. Also,
// problems with multi line string literatls.
//
// Revision 1.3  2001/08/30 23:08:24  jimg
// Merged with 3.2.4
//
// Revision 1.2.4.1  2001/06/22 04:45:28  reza
// Added/Fixed exception handling.
//
// Revision 1.2  2000/10/06 01:22:03  jimg
// Moved the CVS Log entries to the ends of files.
// Modified the read() methods to match the new definition in the dap library.
// Added exception handlers in various places to catch exceptions thrown
// by the dap library.
//
// Revision 1.1  1999/07/28 00:22:47  jimg
// Added
//
// Revision 1.15.2.1  1999/05/27 17:43:23  reza
// Fixed bugs in string changes
//
// Revision 1.15  1999/05/08 00:38:21  jimg
// Fixes for the String --> string changes
//
// Revision 1.14  1999/05/07 23:45:33  jimg
// String --> string fixes
//
// Revision 1.13  1999/04/08 13:45:52  reza
// Added support for more data types.
//
// Revision 1.12  1999/03/30 05:20:56  reza
// Added support for the new data types (Int16, UInt16, and Float32).
//
// Revision 1.11  1998/03/20 00:31:07  jimg
// Changed calls to the set_mime_*() to match the new parameters
//
// Revision 1.10  1997/03/10 16:24:23  reza
// Added error object and upgraded to DODS core release 2.12.
//
// Revision 1.9  1996/09/17 17:07:07  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.8.2.2  1996/07/10 21:44:45  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.8.2.1  1996/06/25 22:05:08  jimg
// Version 2.0 from Reza.
//
// Revision 1.7  1995/07/09  21:34:02  jimg
// Added copyright notice.
//
// Revision 1.6  1995/06/23  15:35:24  jimg
// Added netio.h.
// Fixed some misc problems which may have come about do to the modifications in
// nc_das.cc and nc_dds.cc (they went from CGIs to simple UNIX filters).
//
// Revision 1.5  1995/02/10  04:47:35  reza
// Updated to use type subclasses, NCArray, NCByte, NCInt32, NCGrid....
//
// Revision 1.4  1995/01/31  20:21:34  reza
//  Modified for new type subclassed DAP implementation.
//
// Revision 1.3  1994/12/22  04:51:44  reza
// Updated to use DODS new named dimension capability.
//
// Revision 1.2  1994/11/03  05:22:02  reza
// Added variable names to variable types Grid and Array.
//
// Revision 1.1  1994/10/28  14:34:03  reza
// First version
//
