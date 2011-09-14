
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

// (c) COPYRIGHT URI/MIT 1994-1999
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      reza            Reza Nekovei (rnekovei@ieee.org)

// netCDF sub-class implementation for NCByte,...NCGrid.
// The files are patterned after the subcalssing examples
// Test<type>.c,h files.
//
// ReZa 1/12/95

#include "config_nc.h"

static char rcsid[] not_used =
    {"$Id$"
    };

#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <netcdf.h>

// #define DODS_DEBUG 1

#include <Error.h>
#include <InternalErr.h>
#include <util.h>
#include <debug.h>

#include "NCArray.h"
#include "NCStructure.h"

BaseType *
NCArray::ptr_duplicate()
{
    return new NCArray(*this);
}

/** Build an NCArray instance.
    @param n The name of the array.
    @param v Use this variable as a template for type of array elements.
    The variable will be copied, so the caller is responsible for freeing
    storage used by the actual parameter. Also, if the actual parameter
    is an Array, libdap++ code will use the template of that Array as the
    template for this NCArray. */
NCArray::NCArray(const string &n, const string &d, BaseType *v)
    : Array(n, d, v)
{}

NCArray::NCArray(const NCArray &rhs) : Array(rhs)
{}

NCArray::~NCArray()
{
}

NCArray &
NCArray::operator=(const NCArray &rhs)
{
    if (this == &rhs)
        return *this;

    dynamic_cast<Array &>(*this) = rhs;

    return *this;
}


// Should this be a private method? jhrg 11/3/04
/** Transfer information from the NCArray instance to C arrays which can be
    used with the netCDF C library calls. This method sets the constraints
    for all of the dimensions of the array. The value-result parameters must
    point to arrays large enough to hold values for all of the array's
    dimensions (i.e., if the instance is a three dimensional array, each of
    \e cor, \e step and \e egd must have at least three dimensions.

    @param cor A value-result parameter of 'corner values' for the hyperslab.
    @param step A value-result parameter of step values for the hyperslab.
    @param edg A value-result parameter of edge lengths for the hyperslab.
    @param has_stride A value-result parameter; true if the constraint
    includes a stride value. If the
    @return The number of elements in the constraint. */
long
NCArray::format_constraint(size_t *cor, ptrdiff_t *step, size_t *edg,
                           bool *has_stride)
{
    int start, stride, stop;
    int id = 0;
    long nels = 1;

    *has_stride = false;

    for (Dim_iter p = dim_begin(); p != dim_end(); ++p) {
        start = dimension_start(p, true);
        stride = dimension_stride(p, true);
        stop = dimension_stop(p, true);
        // Check for an empty constraint and use the whole dimension if so.
        if (start + stop + stride == 0) {
            start = dimension_start(p, false);
            stride = dimension_stride(p, false);
            stop = dimension_stop(p, false);
        }

        cor[id] = start;
        step[id] = stride;
        edg[id] = ((stop - start) / stride) + 1; // count of elements
        nels *= edg[id++];      // total number of values for variable

        if (stride != 1)
            *has_stride = true;
    }

    return nels;
}

void NCArray::append_compound_values(int ncid, int varid, nc_type datatype,
        int nfields, int nels, size_t size, vector<char> &values)
{
    for (int element = 0; element < nels; ++element) {
        try {
            NCStructure *ncs = dynamic_cast<NCStructure*> (var()->ptr_duplicate());
            for (int i = 0; i < nfields; ++i) {
                char field_name[NC_MAX_NAME];
                nc_type field_typeid;
                size_t field_offset;
                // TODO: these are unused... should they be used?
                int field_ndims;
                int field_sizes[MAX_NC_DIMS];
                nc_inq_compound_field(ncid, datatype, i, field_name, &field_offset, &field_typeid, &field_ndims, &field_sizes[0]);
                ncs->var(field_name)->val2buf(values.data() + (element * size) + field_offset);
                ncs->var(field_name)->set_read_p(true);
            }
            ncs->set_read_p(true);
            set_vec(element, ncs);
        }
        catch (std::exception &e) {
            throw InternalErr(__FILE__, __LINE__, e.what());
        }
    }
}

void NCArray::do_array_read(int ncid, int varid, nc_type datatype,
        vector<char> &values, bool has_values, int values_offset,
        int nels, size_t cor[], size_t edg[], ptrdiff_t step[], bool has_stride)
{
    int errstat;
    if (datatype >= NC_FIRSTUSERTYPEID) {
        char type_name[NC_MAX_NAME];
        size_t size;
        nc_type base_type;
        size_t nfields;
        int class_type;
        errstat = nc_inq_user_type(ncid, datatype, type_name, &size, &base_type, &nfields, &class_type);
        //cerr << "User-defined attribute type size: " << size << ", nfields: " << nfields << endl;
        if (errstat != NC_NOERR)
            throw(InternalErr(__FILE__, __LINE__, "Could not get information about a user-defined type (" + long_to_string(errstat) + ")."));

        switch (class_type) {
            case NC_COMPOUND: {
                if (!has_values) {
                    values.resize(size * nels);
                    if (has_stride)
                        errstat = nc_get_vars(ncid, varid, cor, edg, step, values.data() /*&values[0]*/);
                    else
                        errstat = nc_get_vara(ncid, varid, cor, edg, values.data() /*&values[0]*/);
                    if (errstat != NC_NOERR)
                        throw Error(errstat, string("Could not get the value for variable '") + name() + string("'"));
                    has_values = true;
                }

                for (int element = 0; element < nels; ++element) {
                    try {
                        NCStructure *ncs = dynamic_cast<NCStructure*> (var()->ptr_duplicate());
                        for (int i = 0; i < nfields; ++i) {
                            char field_name[NC_MAX_NAME];
                            nc_type field_typeid;
                            size_t field_offset;
                            // TODO: these are unused... should they be used?
                            int field_ndims;
                            int field_sizes[MAX_NC_DIMS];
                            nc_inq_compound_field(ncid, datatype, i, field_name, &field_offset, &field_typeid, &field_ndims, &field_sizes[0]);

                            if (field_typeid >= NC_FIRSTUSERTYPEID) {
                                // Interior user defined types have names, but not field_names
                                // so use the type name as the field name (matches the
                                // behavior of the ncdds.cc code).
                                nc_inq_compound_name(ncid, field_typeid, field_name);
                                NCStructure &child_ncs = dynamic_cast<NCStructure&>(*ncs->var(field_name));
                                child_ncs.do_structure_read(ncid, varid, field_typeid, values, has_values, field_offset + values_offset);
                            }
                            else {
                                // Because the netcdf api reads data 'atomically' from
                                // compounds, this call works for both cardinal and
                                // array variables.
                                ncs->var(field_name)->val2buf(values.data() + (element * size) + field_offset);
                            }


                            ncs->var(field_name)->set_read_p(true);
                        }
                        ncs->set_read_p(true);
                        set_vec(element, ncs);
                    }
                    catch (std::exception &e) {
                        throw InternalErr(__FILE__, __LINE__, e.what());
                    }
                }

                set_read_p(true);
                break;
            }

            case NC_VLEN:
                cerr << "in build_user_defined; found a vlen." << endl;
                break;
            case NC_OPAQUE:
                cerr << "in build_user_defined; found a opaque." << endl;
                break;
            case NC_ENUM:
                cerr << "in build_user_defined; found a enum." << endl;
                break;
            default:
                throw InternalErr(__FILE__, __LINE__, "Expected one of NC_COMPOUND, NC_VLEN, NC_OPAQUE or NC_ENUM");
        }

    }
    else {
        // Correct data types to match with the local machine data types
        switch (datatype) {
            case NC_FLOAT:
            case NC_DOUBLE:
            case NC_SHORT:
            case NC_INT:
            case NC_BYTE:
#if NETCDF_VERSION >= 4
            case NC_USHORT:
            case NC_UINT:
            case NC_UBYTE:
#endif
            {
                //vector<char> values(nels * nctypelen(datatype));
                values.resize(nels * nctypelen(datatype));
                if (has_stride)
                    errstat = nc_get_vars(ncid, varid, cor, edg, step, &values[0]);
                else
                    errstat = nc_get_vara(ncid, varid, cor, edg, &values[0]);

                if (errstat != NC_NOERR)
                    throw Error(errstat, string("Could not get the value for variable '") + name() + string("'"));

                val2buf((void *) &values[0]);
                set_read_p(true);
                break;
            }

            case NC_CHAR: {
                // Use the dimension info from netcdf since that's the place where
                // this variable has N-dims. In the DAP representation it's a N-1
                // dimensional variable.
                //nc_type datatype;           // variable data type
                int num_dim;                // number of dim. in variable
                int vdimids[MAX_VAR_DIMS];  // variable dimension ids
                errstat = nc_inq_var(ncid, varid, (char *)0, (nc_type*)0, &num_dim, vdimids, (int *)0);
                if (errstat != NC_NOERR)
                    throw Error(errstat, string("Could not read information about the variable `") + name() + string("'."));

                size_t vdims[MAX_VAR_DIMS]; // variable dimension sizes
                for (int i = 0; i < num_dim; ++i)
                    if ((errstat = nc_inq_dimlen(ncid, vdimids[i], &vdims[i])) != NC_NOERR)
                        throw Error(errstat, string("Could not read dimension information about the variable `") + name() + string("'."));

                int nth_dim_size = vdims[num_dim - 1];

                vector<char> chrbuf(nels * nth_dim_size * nctypelen(datatype));

                cor[num_dim - 1] = 0;
                edg[num_dim - 1] = nth_dim_size;
                if (has_stride)
                    step[num_dim - 1] = 1;

                // read the values in from the local netCDF file.
                if (has_stride)
                    errstat = nc_get_vars_text(ncid, varid, cor, edg, step, &chrbuf[0]);
                else
                    errstat = nc_get_vara_text(ncid, varid, cor, edg, &chrbuf[0]);

                if (errstat != NC_NOERR)
                    throw Error(errstat, string("Could not read the variable `") + name() + string("'."));

                if (num_dim < 2)    // one-dim --> DAP String and we should not be here
                    throw Error(string("A one-dimensional NC_CHAR array should now map to a DAP string: '") + name() + string("'."));

                // How large is the Nth dimension? Allocate space for the N-1 dims.
                vector < string > strg(nels);
                vector<char> buf(nth_dim_size + 1);
                // put the char values in the string array
                for (int i = 0; i < nels; i++) {
                    strncpy(&buf[0], &chrbuf[(i * nth_dim_size)], nth_dim_size);
                    buf[nth_dim_size] = '\0';
                    strg[i] = (string) & buf[0];
                }

                // reading is done (don't need to read each individual array value)
                set_read_p(true);
                // put values in the buffers
                val2buf(&strg[0]);

                break;
            }
#if NETCDF_VERSION >= 4
                case NC_STRING: {
                    vector<char*> chrbuf(nels*nctypelen(datatype));

                    // read the values in from the local netCDF file.
                    if (has_stride)
                        errstat = nc_get_vars_string(ncid, varid, cor, edg, step, &chrbuf[0]);
                    else
                        errstat = nc_get_vara_string(ncid, varid, cor, edg, &chrbuf[0]);

                    if (errstat != NC_NOERR)
                        throw Error(errstat, string("Could not read the variable `") + name() + string("'."));

                    vector<string> strg(nels);

                    // put the char values in the string array
                    for (int i = 0; i < nels; i++) {
                        strg[i] = chrbuf[i];
                    }

                    // reading is done (don't need to read each individual array value)
                    set_read_p(true);
                    // put values in the buffers
                    val2buf(&strg[0]);

                    break;
                }
#endif

            default:
                throw InternalErr(__FILE__, __LINE__, string("Unknown data type for the variable '") + name() + string("'."));
        }
    }

    // cerr << "Leaving do_arry_read()" << endl;
}

bool NCArray::read()
{
    if (read_p())  // Nothing to do
        return false;

    int ncid;
    int errstat = nc_open(dataset().c_str(), NC_NOWRITE, &ncid); /* netCDF id */
    if (errstat != NC_NOERR)
        throw Error(errstat, string("Could not open the dataset's file (") + dataset().c_str() + string(")"));

    int varid;                  /* variable Id */
    errstat = nc_inq_varid(ncid, name().c_str(), &varid);
    if (errstat != NC_NOERR)
        throw InternalErr(__FILE__, __LINE__, "Could not get variable ID for: " + name() + ". (error: " + long_to_string(errstat) + ").");

    nc_type datatype;           // variable data type
#if 0
    int num_dim;                // number of dim. in variable
    int vdimids[MAX_VAR_DIMS];  // variable dimension ids
#endif
    errstat = nc_inq_vartype(ncid, varid, &datatype);
    if (errstat != NC_NOERR)
        throw Error(errstat, string("Could not read information about the variable `") + name() + string("'."));
#if 0
    size_t vdims[MAX_VAR_DIMS]; // variable dimension sizes
    for (int i = 0; i < num_dim; ++i)
        if ((errstat = nc_inq_dimlen(ncid, vdimids[i], &vdims[i])) != NC_NOERR)
            throw Error(errstat, string("Could not read dimension information about the variable `") + name() + string("'."));
#endif
    size_t cor[MAX_NC_DIMS];      /* corner coordinates */
    size_t edg[MAX_NC_DIMS];      /* edges of hyper-cube */
    ptrdiff_t step[MAX_NC_DIMS];  /* stride of hyper-cube */
    bool has_stride;
    long nels = format_constraint(cor, step, edg, &has_stride);

    vector<char> values;
    do_array_read(ncid, varid, datatype, values, false /*has_values*/, 0 /*values_offset*/,
            nels, cor, edg, step, has_stride);
    //cerr << "After do_array_read" <<endl;
    // set_read_p(true);
#if 0
    if (datatype >= NC_FIRSTUSERTYPEID) {
        char type_name[NC_MAX_NAME];
        size_t size;
        nc_type base_type;
        size_t nfields;
        int class_type;
        errstat = nc_inq_user_type(ncid, datatype, type_name, &size, &base_type, &nfields, &class_type);
        //cerr << "User-defined attribute type size: " << size << ", nfields: " << nfields << endl;
        if (errstat != NC_NOERR)
            throw(InternalErr(__FILE__, __LINE__, "Could not get information about a user-defined type (" + long_to_string(errstat) + ")."));

        switch (class_type) {
            case NC_COMPOUND: {
                //cerr << "in read_attributes_netcdf4; found a compound attribute. (" << type_name << ")" << endl;
                vector<unsigned char> values(nels * size);
                if (has_stride)
                    errstat = nc_get_vars(ncid, varid, cor, edg, step, &values[0]);
                else
                    errstat = nc_get_vara(ncid, varid, cor, edg, &values[0]);

                if (errstat != NC_NOERR)
                    throw Error(errstat, string("Could not get the value for variable '") + name() + string("'"));

                append_compound_values(ncid, varid, datatype, nfields, nels, size, values);

                set_read_p(true);
                break;
            }

            case NC_VLEN:
                cerr << "in build_user_defined; found a vlen." << endl;
                break;
            case NC_OPAQUE:
                cerr << "in build_user_defined; found a opaque." << endl;
                break;
            case NC_ENUM:
                cerr << "in build_user_defined; found a enum." << endl;
                break;
            default:
                throw InternalErr(__FILE__, __LINE__, "Expected one of NC_COMPOUND, NC_VLEN, NC_OPAQUE or NC_ENUM");
        }

    }
    else {
        // Correct data types to match with the local machine data types
        switch (datatype) {
            case NC_FLOAT:
            case NC_DOUBLE:
            case NC_SHORT:
            case NC_INT:
            case NC_BYTE:
#if NETCDF_VERSION >= 4
            case NC_USHORT:
            case NC_UINT:
            case NC_UBYTE:
#endif
            {
                vector<unsigned char> values(nels * nctypelen(datatype));
                if (has_stride)
                    errstat = nc_get_vars(ncid, varid, cor, edg, step, &values[0]);
                else
                    errstat = nc_get_vara(ncid, varid, cor, edg, &values[0]);

                if (errstat != NC_NOERR)
                    throw Error(errstat, string("Could not get the value for variable '") + name() + string("'"));

                val2buf((void *) &values[0]);
                set_read_p(true);
                break;
            }

            case NC_CHAR: {
                // Use the dimension info from netcdf since that's the place where
                // this variable has N-dims. In the DAP representation it's a N-1
                // dimensional variable.
                int nth_dim_size = vdims[num_dim - 1];

                vector<char> chrbuf(nels * nth_dim_size * nctypelen(datatype));

                cor[num_dim - 1] = 0;
                edg[num_dim - 1] = nth_dim_size;
                if (has_stride)
                    step[num_dim - 1] = 1;

                // read the values in from the local netCDF file.
                if (has_stride)
                    errstat = nc_get_vars_text(ncid, varid, cor, edg, step, &chrbuf[0]);
                else
                    errstat = nc_get_vara_text(ncid, varid, cor, edg, &chrbuf[0]);

                if (errstat != NC_NOERR)
                    throw Error(errstat, string("Could not read the variable `") + name() + string("'."));

                if (num_dim < 2)    // one-dim --> DAP String and we should not be here
                    throw Error(string("A one-dimensional NC_CHAR array should now map to a DAP string: '") + name() + string("'."));

                // How large is the Nth dimension? Allocate space for the N-1 dims.
                vector < string > strg(nels);
                vector<char> buf(nth_dim_size + 1);
                // put the char values in the string array
                for (int i = 0; i < nels; i++) {
                    strncpy(&buf[0], &chrbuf[(i * nth_dim_size)], nth_dim_size);
                    buf[nth_dim_size] = '\0';
                    strg[i] = (string) & buf[0];
                }

                // reading is done (don't need to read each individual array value)
                set_read_p(true);
                // put values in the buffers
                val2buf(&strg[0]);

                break;
            }
#if NETCDF_VERSION >= 4
                case NC_STRING: {
                    vector<char*> chrbuf(nels*nctypelen(datatype));

                    // read the values in from the local netCDF file.
                    if (has_stride)
                        errstat = nc_get_vars_string(ncid, varid, cor, edg, step, &chrbuf[0]);
                    else
                        errstat = nc_get_vara_string(ncid, varid, cor, edg, &chrbuf[0]);

                    if (errstat != NC_NOERR)
                        throw Error(errstat, string("Could not read the variable `") + name() + string("'."));

                    vector<string> strg(nels);

                    // put the char values in the string array
                    for (int i = 0; i < nels; i++) {
                        strg[i] = chrbuf[i];
                    }

                    // reading is done (don't need to read each individual array value)
                    set_read_p(true);
                    // put values in the buffers
                    val2buf(&strg[0]);

                    break;
                }
#endif

            default:
                throw InternalErr(__FILE__, __LINE__, string("Unknown data type for the variable '") + name() + string("'."));
        }
    }
#endif
    if (nc_close(ncid) != NC_NOERR)
        throw InternalErr(__FILE__, __LINE__, "Could not close the dataset!");

    //cerr << "Leaving NCArrary::read()" << endl;

    return false;
}
