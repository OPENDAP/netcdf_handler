
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
// from a netcdf file and build the in-memory DDS. These functions form the
// core of the server-side software necessary to extract the DDS from a
// netcdf data file.
//
// It also contains test code which will print the in-memory DDS to
// stdout.
//
// ReZa 10/20/94

#include "config_nc.h"

static char not_used rcsid[]={"$Id$"};

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>

#include <netcdf.h>

#include <DDS.h>
#include <mime_util.h>
#include <util.h>

#include "NCInt32.h"
#include "NCUInt32.h"
#include "NCInt16.h"
#include "NCUInt16.h"
#include "NCFloat64.h"
#include "NCFloat32.h"
#include "NCByte.h"
#include "NCArray.h"
#include "NCGrid.h"
#include "NCStr.h"

using namespace libdap ;

/** This function returns the appropriate DODS BaseType for the given
    netCDF data type. */
static BaseType *
Get_bt(const string &varname, const string &dataset, nc_type datatype)
{
    switch (datatype) {
#if NETCDF_VERSION >= 4
        case NC_STRING:
#endif
        case NC_CHAR:
            return (new NCStr(varname, dataset));

        case NC_BYTE:
            return (new NCByte(varname, dataset));

#if NETCDF_VERSION >= 4
        case NC_UBYTE:
            // NB: the dods_byte type is unsigned
            return (new NCByte(varname, dataset));
#endif
        case NC_SHORT:
            return (new NCInt16(varname, dataset));

#if NETCDF_VERSION >= 4
        case NC_USHORT:
            return (new NCUInt16(varname, dataset));
#endif
       case NC_INT:
            return (new NCInt32(varname, dataset));

#if NETCDF_VERSION >= 4
        case NC_UINT:
            return (new NCUInt32(varname, dataset));
#endif
        case NC_FLOAT:
            return (new NCFloat32(varname, dataset));

        case NC_DOUBLE:
            return (new NCFloat64(varname, dataset));

        default: // Maybe this should be an error? jhrg 1/12/09
            throw Error("netcdf: Unknown type for variable '" + varname + "'");
    }
}

/** Build a grid given that one has been found. The Grid's Array is already
    allocated and is passed in along with a number of arrays containing
    information about the dimensions of the Grid.

    Note: The dim_szs and dim_nms arrays could be removed since that information
    is already in the Array ar. */
static Grid *build_grid(Array *ar, int ndims, const nc_type array_type,
        const char map_names[MAX_NC_VARS][MAX_NC_NAME],
        const nc_type map_types[MAX_NC_VARS],
        const size_t map_sizes[MAX_VAR_DIMS],
        vector<string> *all_maps)
{
    // Grids of NC_CHARs are treated as Grids of strings; the outermost
    // dimension (the char vector) becomes the string.
    if (array_type == NC_CHAR)
        --ndims;
#if 0
    // If this is a String (NC_CHAR Array), or an Array of String,
    // add all but the Nth dim
    if (ar->var()->type() == dods_str_c)
        --ndims;
#endif

    for (int d = 0; d < ndims; ++d) {
        ar->append_dim(map_sizes[d], map_names[d]);
        // Save the map names for latter use, which might not happen...
        all_maps->push_back(string(map_names[d]));
    }

    const string &filename = ar->dataset();
    Grid *gr = new NCGrid(ar->name(), filename);
    gr->add_var(ar, array);

    // Build and add BaseType/Array instances for the maps
    for (int d = 0; d < ndims; ++d) {
        BaseType *local_bt = Get_bt(map_names[d], filename, map_types[d]);
        NCArray *local_ar = new NCArray(local_bt->name(), filename, local_bt);
        delete local_bt;
        local_ar->append_dim(map_sizes[d], map_names[d]);
        gr->add_var(local_ar, maps);
        delete local_ar;
    }

    return gr;
}

/**  Iterate over all of the variables in the data set looking for a one
     dimensional variable whose name and size match the name and size of the
     dimension 'dimname' of variable 'var'. If one is found, return it's
     name and size. It is considered to be a Map for 'var' (i.e., a matching
     coordinate variable).

     @param ncid Used with all netCDF API calls
     @param var Variable number: Look for a map for this variable.
     @param dimname Name of the current dimension
     @param dim_sz Size of the current dimension
     @param match_type Value-result parameter, holds dimension type if match found
     @return true if a match coordinate variable (i.e., map) was found, false
     otherwise

     @note In this code I scan all the variables, maybe there's a way to look
     at (only) all the shared dimensions?
 */
static bool find_matching_coordinate_variable(int ncid, int var,
        char dimname[], size_t dim_sz, nc_type *match_type)
{
    // For netCDF, a Grid's Map must be a netCDF dimension
    int id;
    // get the id matching the name.
    int status = nc_inq_dimid(ncid, dimname, &id);
    if (status == NC_NOERR) {
        // get the length, the name was matched above
        size_t length;
        status = nc_inq_dimlen(ncid, id, &length);
        if (status != NC_NOERR) {
            string msg = "netcdf 3: could not get size for dimension ";
            msg += long_to_string(id);
            msg += " in variable ";
            msg += string(dimname);
            throw Error(msg);
        }
        if (length == dim_sz) {
            // Both the name and size match and it's a dimension, so we've
            // found our 'matching coordinate variable'. To get the type,
            // Must find the variable with the name that matches the dimension.
            int varid = -1;
            status = nc_inq_varid(ncid, dimname, &varid);
            // A variable cannot be its own coordinate variable.
            // The unlimited dimension does not correspond to a variable,
            // hence the status error is means the named thing is not a
            // coordinate; it's not an error as far as the handler is concerned.
            if (var == varid || status != NC_NOERR)
                return false;

            status = nc_inq_vartype(ncid, varid, match_type);
            if (status != NC_NOERR) {
                string msg = "netcdf 3: could not get type variable ";
                msg += string(dimname);
                throw Error(msg);
            }

            return true;
        }
    }
    return false;
}

/** Is the variable a DAP Grid?
     @param ncid The open netCDF file id
     @param nvars The number of variables in the file. Needed by the function
     that looks for maps.
     @param var The id of the variable we're asking about.
     @param ndims The number of dimensions in 'var'.
     @param dim_ids The dimension ids of 'var'.
     @param map_sizes Value-result parameter; the size of each map.
     @param map_names Value-result parameter; the name of each map.
     @param map_types Value-result parameter; the type of each map.
 */
static bool is_grid(int ncid, int var, int ndims, const int dim_ids[MAX_VAR_DIMS],
        size_t map_sizes[MAX_VAR_DIMS],
        char map_names[MAX_NC_VARS][MAX_NC_NAME],
        nc_type map_types[MAX_NC_VARS])
{

    // Look at each dimension of the variable.
    for (int d = 0; d < ndims; ++d) {
        char dimname[MAX_NC_NAME];
        size_t dim_sz;

        int errstat = nc_inq_dim(ncid, dim_ids[d], dimname, &dim_sz);
        if (errstat != NC_NOERR) {
            string msg = "netcdf 3: could not get size for dimension ";
            msg += long_to_string(d);
            msg += " in variable ";
            msg += long_to_string(var);
            throw Error(msg);
        }

        nc_type match_type;
        if (find_matching_coordinate_variable(ncid, var, dimname, dim_sz, &match_type)) {
            map_types[d] = match_type;
            map_sizes[d] = dim_sz;
            strncpy(map_names[d], dimname, MAX_NC_NAME - 1);
            map_names[d][MAX_NC_NAME - 1] = '\0';
        }
        else {
            return false;
        }
    }

    return true;
}

static bool is_dimension(const string &name, vector<string> maps)
{
    vector<string>::iterator i = find(maps.begin(), maps.end(), name);
    if (i != maps.end())
        return true;
    else
        return false;
}

static NCArray *build_array(BaseType *bt, int ncid, int var,
        const nc_type array_type, int ndims,
        const int dim_ids[MAX_NC_DIMS])
{
    NCArray *ar = new NCArray(bt->name(), bt->dataset(), bt);

    if (array_type == NC_CHAR)
        --ndims;
#if 0
    if (ar->var()->type() == dods_str_c)
        --ndims;
#endif

    char dimname[MAX_NC_NAME];
    size_t dim_sz;

    for (int d = 0; d < ndims; ++d) {
        int errstat = nc_inq_dim(ncid, dim_ids[d], dimname, &dim_sz);
        if (errstat != NC_NOERR) {
        	delete ar;

            string msg = "netcdf 3: could not get size for dimension ";
            msg += long_to_string(d);
            msg += " in variable ";
            msg += long_to_string(var);
            throw Error(msg);
        }

        ar->append_dim(dim_sz, dimname);
    }

    return ar;
}

/** Read given number of variables (nvars) from the opened netCDF file
     (ncid) and add them with their appropriate type and dimensions to
     the given instance of the DDS class.

     @param dds_table Add variables to this DDS object
     @param filename When making new variables, record this as the source
     @param ncid The id of the netcdf file
     @param nvars The number of variables in the opened file
 */
static void read_class(DDS &dds_table, const string &filename, int ncid,
        int nvars, bool elide_dimension_arrays)
{
    // How this function works: The variables are scanned once but because
    // netCDF includes shared dimensions as variables there are two versions
    // of this function. One writes out the variables as they are found while
    // the other writes scalars and Grids as they are found and saves Arrays
    // for output last. Then, when writing the arrays, it checks to see if
    // an array variable is also a grid dimension and, if so, does not write
    // it out (thus in the second version of the function, all arrays appear
    // after the other variable types and only those arrays that do not
    // appear as Grid Maps are included.

    // These two vectors are used to record the ids of array variables and
    // the names of all of the Grid Map variables
    vector<int> array_vars;
    vector<string> all_maps;

    // These are defined here since they are used by both loops.
    char name[MAX_NC_NAME];
    nc_type nctype;
    int ndims;
    int dim_ids[MAX_VAR_DIMS];

    // Examine each variable in the file; if 'elide_grid_maps' is true, adds
    // only scalars and Grids (Arrays are added in the following loop). If
    // false, all variables are added in this loop.
    for (int var = 0; var < nvars; ++var) {
        int errstat = nc_inq_var(ncid, var, name, &nctype, &ndims, dim_ids,
                (int *) 0);
        if (errstat != NC_NOERR) {
            string msg = "netcdf: could not get name or dimension number for variable ";
            msg += long_to_string(var);
            throw Error(msg);
        }

        // These are defined here because they are value-result parameters for
        // is_grid() called below.
        size_t map_sizes[MAX_VAR_DIMS];
        char map_names[MAX_NC_VARS][MAX_NC_NAME];
        nc_type map_types[MAX_NC_VARS];

        // a scalar? NB a one-dim NC_CHAR array will have DAP type of
        // dods_str_c because it's really a scalar string, not an array.
        if (ndims == 0 || (ndims == 1 && nctype == NC_CHAR)) {
            BaseType *bt = Get_bt(name, filename, nctype);
            dds_table.add_var(bt);
            delete bt;
        }
        else if (is_grid(ncid, var, ndims, dim_ids, map_sizes,
                map_names, map_types)) {
            BaseType *bt = Get_bt(name, filename, nctype);
            Array *ar = new NCArray(name, filename, bt);
            delete bt;
            Grid *gr = build_grid(ar, ndims, nctype, map_names, map_types, map_sizes,
                    &all_maps);
            delete ar;
            dds_table.add_var(gr);
            delete gr;
        }
        else {
            if (elide_dimension_arrays)
                array_vars.push_back(var);
            else {
                BaseType *bt = Get_bt(name, filename, nctype);
                NCArray *ar = build_array(bt, ncid, var, nctype, ndims, dim_ids);
                delete bt;
                dds_table.add_var(ar);
                delete ar;
            }
        }
    }

    // This code is only run if elide_grid_map is true and in that case the
    // loop above did not create any simple arrays. Instead it pushed the
    // var ids of things that look like simple arrays onto a vector. This code
    // will add all of those that really are arrays and not the ones that are
    // dimensions used by a Grid.
    if (elide_dimension_arrays) {
        // Now just loop through the saved array variables, writing out only
        // those that are not Grid Maps
        nvars = array_vars.size();
        for (int i = 0; i < nvars; ++i) {
            int var = array_vars.at(i);

            int errstat = nc_inq_var(ncid, var, name, &nctype, &ndims,
                    dim_ids, (int *) 0);
            if (errstat != NC_NOERR) {
                string msg = "netcdf 3: could not get name or dimension number for variable ";
                msg += long_to_string(var);
                throw Error(msg);
            }

            // If an array already appears as a Grid Map, don't write it out
            // as an array too.
            if (is_dimension(string(name), all_maps))
                continue;

            BaseType *bt = Get_bt(name, filename, nctype);
            Array *ar = build_array(bt, ncid, var, nctype, ndims, dim_ids);
            delete bt;
            dds_table.add_var(ar);
            delete ar;
        }
    }
}

/** Given a reference to an instance of class DDS and a filename that refers
    to a netcdf file, read the netcdf file and extract all the dimensions of
    each of its variables. Add the variables and their dimensions to the
    instance of DDS.

    @param elide_dimension_arrays If true, don't include an array if it's
    really a dimension used by a Grid. */
void nc_read_dataset_variables(DDS &dds_table, const string &filename,
        bool elide_dimension_arrays)
{
    ncopts = 0;
    int ncid, errstat;
    int nvars;

    errstat = nc_open(filename.c_str(), NC_NOWRITE, &ncid);
    if (errstat != NC_NOERR) {
        string msg = "Could not open " + path_to_filename(filename) + ".";
        throw Error(errstat, msg);
    }

    // how many variables?
    errstat = nc_inq_nvars(ncid, &nvars);
    if (errstat != NC_NOERR) {
        string msg = "Could not inquire about netcdf file: " + path_to_filename(filename) + ".";
        throw Error(errstat, msg);
    }

    // dataset name
    dds_table.set_dataset_name(name_path(filename));

    // read variables class
    read_class(dds_table, filename, ncid, nvars, elide_dimension_arrays);

    if (nc_close(ncid) != NC_NOERR)
        throw InternalErr(__FILE__, __LINE__,
                "ncdds: Could not close the dataset!");
}

