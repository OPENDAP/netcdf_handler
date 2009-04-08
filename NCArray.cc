
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

#include "NCArray.h"

#include <debug.h>

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
    DBG(cerr << "delete d_source " << d_source << endl);
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

bool NCArray::read()
{
    DBG(cerr << "In NCArray::read" << endl);

    if (read_p())  // Nothing to do
        return false;

    DBG(cerr << "In NCArray, opening the dataset, reading " << name() << endl);

    int ncid;
    int errstat = nc_open(dataset().c_str(), NC_NOWRITE, &ncid); /* netCDF id */

    if (errstat != NC_NOERR)
        throw Error(errstat, string("Could not open the dataset's file (")
                    + dataset().c_str() + string(")"));

    int varid;                  /* variable Id */
    errstat = nc_inq_varid(ncid, name().c_str(), &varid);
    if (errstat != NC_NOERR)
        throw Error(errstat, "Could not get variable ID.");

    nc_type datatype;           /* variable data type */
    int vdimids[MAX_VAR_DIMS];  // variable dimension ids
    size_t vdims[MAX_VAR_DIMS];    /* variable dimension sizes */
    int num_dim;                /* number of dim. in variable */
    errstat = nc_inq_var(ncid, varid, (char *)0, &datatype, &num_dim, vdimids,
                         (int *)0);
    if (errstat != NC_NOERR)
        throw Error(errstat,
                    string("Could not read information about the variable `")
                    + name() + string("'."));

    for (int i = 0; i < num_dim; ++i)
        if ((errstat = nc_inq_dimlen(ncid, vdimids[i], &vdims[i])) != NC_NOERR)
            throw Error(errstat,
                    string("Could not read dimension information about the variable `")
                    + name() + string("'."));

    size_t cor[MAX_NC_DIMS];      /* corner coordinates */
    size_t edg[MAX_NC_DIMS];      /* edges of hyper-cube */
    ptrdiff_t step[MAX_NC_DIMS];  /* stride of hyper-cube */
    bool has_stride;
    long nels = format_constraint(cor, step, edg, &has_stride);

    // Correct data types to match with the local machine data types
    switch (datatype) {
    case NC_FLOAT: {
            float *fltbuf = (float *) new char[(nels * nctypelen(datatype))];

            if (has_stride)
                errstat = nc_get_vars_float(ncid, varid, cor, edg, step, fltbuf);
            else
                errstat = nc_get_vara_float(ncid, varid, cor, edg, fltbuf);

            if (errstat != NC_NOERR) {
            	delete[] fltbuf;
                throw Error(errstat, string("Could not read the variable `") + name()
                            + string("'."));
            }

            if (nctypelen(datatype) != sizeof(dods_float32)) {
                dods_float32 *flt32 = new dods_float32[nels];

                for (int id = 0; id < nels; id++)
                    *(flt32 + id) = (dods_float32) * (fltbuf + id);

                val2buf((void *) flt32);
                delete[]flt32;
            }
            else {
                val2buf((void *) fltbuf);
            }

            set_read_p(true);
            delete[]fltbuf;

            break;
        }

    case NC_DOUBLE: {

            double *dblbuf = (double *) new char [(nels*nctypelen(datatype))];

            if (has_stride)
                errstat = nc_get_vars_double(ncid, varid, cor, edg, step, dblbuf);
            else
                errstat = nc_get_vara_double(ncid, varid, cor, edg, dblbuf);

            if (errstat != NC_NOERR) {
            	delete[] dblbuf;
                throw Error(errstat,
                            string("Could not read the variable `") + name() + string("'."));
            }
            if (nctypelen(datatype) != sizeof(dods_float64)) {
                dods_float64 *flt64 = new dods_float64 [nels];

                for (int id = 0; id < nels; id++)
                    *(flt64 + id) = (dods_float64) * (dblbuf + id);

                val2buf((void *)flt64);
                delete [] flt64;
            }
            else {
                val2buf((void *)dblbuf);
            }

            set_read_p(true);
            delete [] dblbuf;

            break;
        }

    case NC_SHORT: {

            short *shtbuf = (short *)new char [(nels*nctypelen(datatype))];

            if (has_stride)
                errstat = nc_get_vars_short(ncid, varid, cor, edg, step, shtbuf);
            else
                errstat = nc_get_vara_short(ncid, varid, cor, edg, shtbuf);

            if (errstat != NC_NOERR) {
            	delete[] shtbuf;
                throw Error(errstat,
                            string("Could not read the variable `") + name() + string("'."));
            }

            if (nctypelen(datatype) != sizeof(dods_int16)) {
                dods_int16 *intg16 = new dods_int16 [nels];

                for (int id = 0; id < nels; id++)
                    *(intg16 + id) = (dods_int16) * (shtbuf + id);

                val2buf((void *)intg16);
                delete [] intg16;
            }
            else {
                val2buf((void *)shtbuf);
            }
            set_read_p(true);
            delete [] shtbuf;

            break;
        }

    case NC_INT: {
            int *lngbuf = (int *)new char [(nels*nctypelen(datatype))];

            if (has_stride)
                errstat = nc_get_vars_int(ncid, varid, cor, edg, step, lngbuf);
            else
                errstat = nc_get_vara_int(ncid, varid, cor, edg, lngbuf);

            if (errstat != NC_NOERR) {
            	delete[] lngbuf;
                throw Error(errstat, string("Could not read the variable `")
                            + name() + string("'."));
            }

            if (nctypelen(datatype) != sizeof(dods_int32)) {
                dods_int32 *intg32 = new dods_int32 [nels];

                for (int id = 0; id < nels; id++)
                    *(intg32 + id) = (dods_int32) * (lngbuf + id);

                val2buf((void *) intg32);
                delete [] intg32;
            }
            else {
                val2buf((void*)lngbuf);
            }

            set_read_p(true);
            delete [] lngbuf;

            break;
        }

    case NC_CHAR: {
            // Use the dimension info from netcdf since that's the place where
            // this variable has N-dims. In the DAP representation it's a N-1
            // dimensional variable.
            int nth_dim_size = vdims[num_dim - 1];

            char *chrbuf = (char *)new char [(nels*nth_dim_size*nctypelen(datatype))];

            cor[num_dim-1] = 0;
            edg[num_dim-1] = nth_dim_size;
            if (has_stride)
                step[num_dim-1] = 1;

            // read the values in from the local netCDF file.
            if (has_stride)
                errstat = nc_get_vars_text(ncid, varid, cor, edg, step, chrbuf);
            else
                errstat = nc_get_vara_text(ncid, varid, cor, edg, chrbuf);

            if (errstat != NC_NOERR) {
            	delete[] chrbuf;
                throw Error(errstat, string("Could not read the variable `") + name()
                        + string("'."));
            }

            if (num_dim < 2) { // one-dim --> DAP String and we should not be here
                delete[] chrbuf;
                throw Error(string("A one-dimensional NC_CHAR array should now map to a DAP string: '")
                        + name() + string("'."));
            }

            // How large is the Nth dimension? Allocate space for the N-1 dims.
            string *strg = new string[nels]; // array of strings
            char *buf = new char[nth_dim_size+1];

            // put the char values in the string array
            for (int i = 0; i < nels; i++) {
                strncpy(buf, (chrbuf + (i * nth_dim_size)), nth_dim_size);
                buf[nth_dim_size] = '\0';
                strg[i] = (string)buf;
            }

            // reading is done (don't need to read each individual array value)
            set_read_p(true);
            // put values in the buffers
            val2buf(strg);

            // clean up
            delete [] buf;
            delete [] strg;
            delete [] chrbuf;

            break;
        }

    case NC_BYTE: {
            //default (no type conversion needed and the type Byte)
            char *convbuf = new char [(nels*nctypelen(datatype))];

            if (has_stride)
                errstat = nc_get_vars_uchar(ncid, varid, cor, edg, step, (unsigned char*)convbuf);
            else
                errstat = nc_get_vara_uchar(ncid, varid, cor, edg, (unsigned char*)convbuf);

            if (errstat != NC_NOERR) {
            	delete[] convbuf;
                throw Error(errstat,
                            string("Could not read the variable `") + name() + string("'."));
            }

            set_read_p(true);
            val2buf((void *)convbuf);

            delete [] convbuf;

            break;
        }

    default:
        throw InternalErr(__FILE__, __LINE__,
                          string("Unknown data type for the variable '")
                          + name() + string("'."));
    }

    if (nc_close(ncid) != NC_NOERR)
        throw InternalErr(__FILE__, __LINE__, "Could not close the dataset!");

    return false;
}
