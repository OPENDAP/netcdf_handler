
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

// This file contains functions which read the variables and their attributes
// from a netcdf file and build the in-memeory DAS. These functions form the
// core of the server-side software necessary to extract the DAS from a
// netcdf data file.
//
// It also contains test code which will print the in-memory DAS to
// stdout. It uses both the DAS class as well as the netcdf library.
// In addition, parts of these functions were taken from the netcdf program
// ncdump, from the netcdf standard distribution (ver 2.3.2)
//
// jhrg 9/23/94

#include "config_nc.h"

static char not_used rcsid[]={"$Id$"};

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <netcdf.h>

#include <cgi_util.h>
#include <util.h>
#include <escaping.h>
#include <DAS.h>

#define ATTR_STRING_QUOTE_FIX

using namespace libdap ;

// These are used as the return values for print_type().

static const char STRING[]="String";
static const char BYTE[]="Byte";
static const char INT32[]="Int32";
static const char INT16[]="Int16";
static const char FLOAT64[]="Float64";
static const char FLOAT32[]="Float32";

/** Given the type, array index number and pointer to the associated attribute,
    Return the string representation of the attribute's value.

    This function is modeled on code from ncdump. I modified the original
    version to use C++ string streams and also to call escattr() so that
    attributes with quotes would be handled correctly.

    @param type The nc_type of this attribute
    @param loc The offset within \e vals
    @param vals a void* to the array of values */
static string
print_attr(nc_type type, int loc, void *vals)
{
    ostringstream rep;
    union {
        char *cp;
        short *sp;
        nclong *lp;
        float *fp;
        double *dp;
    } gp;

    switch (type) {
      case NC_BYTE:
        unsigned char uc;
        gp.cp = (char *) vals;

        uc = *(gp.cp+loc);
        rep << (int)uc;
        return rep.str();

      case NC_CHAR:
#ifndef ATTR_STRING_QUOTE_FIX
        rep << "\"" << escattr(static_cast<const char*>(vals)) << "\"";
        return rep.str();
#else
        return escattr(static_cast<const char*>(vals));
#endif
      case NC_SHORT:
        gp.sp = (short *) vals;
        rep << *(gp.sp+loc);
        return rep.str();

      case NC_LONG:
        gp.lp = (nclong *) vals; // warning: long int format, int arg (arg 3)
        rep << *(gp.lp+loc);
        return rep.str();

      case NC_FLOAT: {
          gp.fp = (float *) vals;
          rep << std::showpoint;
          rep << std::setprecision(10);
          rep << *(gp.fp+loc);
          // If there's no decimal point and the rep does not use scientific
          // notation, add a decimal point. This little jaunt was taken because
          // this code is modeled after older code and that's what it did. I'm
          // trying to keep the same behavior as the old code without it's
          // problems. jhrg 8/11/2006
          if (rep.str().find('.') == string::npos
              && rep.str().find('e') == string::npos)
                rep << ".";
          return rep.str();
      }
      case NC_DOUBLE: {
          gp.dp = (double *) vals;
          rep << std::showpoint;
          rep << std::setprecision(17);
          rep << *(gp.dp+loc);
          if (rep.str().find('.') == string::npos
              && rep.str().find('e') == string::npos)
                rep << ".";
          return rep.str();
          break;
      }
      default:
        return string("\"\"");
    }
}

/** Return the printed representation of a netcdf type -- in a form the
    handler can use.

    @param datatype A nc_type
    @return A string that holds the type name.
 */
static string
print_type(nc_type datatype)
{
    switch (datatype) {
      case NC_CHAR:
	return STRING;

      case NC_BYTE:
	return BYTE;

      case NC_SHORT:
	return INT16;

      case NC_LONG:
	return INT32;

      case NC_FLOAT:
	return FLOAT32;

      case NC_DOUBLE:
	return FLOAT64;

      default:
	return STRING;
    }
}

/** Read the vector of attribute values from the netcdf file.

    @param ncid The netCDF file id
    @param varid The variable id
    @param name THe attribute name
    @param value A value-result parameter which will point to the vector of
    values.
    @see print_attr
  */
static int
dap_get_att(int ncid, int varid, const char *name, void *value)
{
    int status;
    nc_type attrp;

    status = nc_inq_atttype(ncid, varid, name, &attrp);
    if(status != NC_NOERR)
	return status;

    switch (attrp) {
      case NC_BYTE:
	return nc_get_att_schar(ncid, varid, name, (signed char *)value);
      case NC_CHAR:
	return nc_get_att_text(ncid, varid, name, (char *)value);
      case NC_SHORT:
	return nc_get_att_short(ncid, varid, name, (short *)value);
      case NC_INT:
#if (SIZEOF_INT >= X_SIZEOF_INT)
	return nc_get_att_int(ncid, varid, name, (int *)value);
#elif SIZEOF_LONG == X_SIZEOF_INT
	return nc_get_att_long(ncid, varid, name, (long *)value);
#endif
      case NC_FLOAT:
	return nc_get_att_float(ncid, varid, name, (float *)value);
      case NC_DOUBLE:
	return nc_get_att_double(ncid, varid, name, (double *)value);
      default:
	return NC_EBADTYPE;
    }
}

/** Given the netcdf file id, variable id, number of attributes for the
    variable, and an attribute table pointer, read the attributes and store
    their names and values in the attribute table.

    @note Attribute values in the DAP are stored only as strings.
    @param ncid The netcdf file id
    @param v The netcdf variable id
    @param natts The number of attributes
    @param at A value-result parameter; a point to the attribute table to which
    the new information will be added.
*/
static void
read_attributes(int ncid, int v, int natts, AttrTable *at)
{
    char attrname[MAX_NC_NAME];
    nc_type datatype;
    size_t len;
    int errstat = NC_NOERR;
    char *value;

    for (int a = 0; a < natts; ++a) {
        errstat = nc_inq_attname(ncid, v, a, attrname);
        if (errstat != NC_NOERR) {
	    string msg = "Could not get the name for attribute ";
	    msg += long_to_string(a);
	    throw Error(errstat, msg);
        }
	errstat = nc_inq_att(ncid, v, attrname, &datatype, &len);
	if (errstat != NC_NOERR) {
	    string msg = "Could not get the name for attribute '";
            msg += attrname + string("'");
            throw Error(errstat, msg);
	}

	value = new char [(len + 1) * nctypelen(datatype)];
	errstat = dap_get_att(ncid, v, attrname, (void *)value);
	if (errstat != NC_NOERR) {
            delete[] value;
   	    throw Error(errstat, "Could not get the attribute value");
	}

	// If the datatype is NC_CHAR then we have a string. netCDF
	// represents strings as arrays of char, but we represent them as X
	// strings. So... Add the null and set the length to 1
	if (datatype == NC_CHAR) {
	    *(value + len) = '\0';
	    len = 1;
	}

	// add all the attributes in the array
	for (unsigned int loc=0; loc < len ; loc++) {
	    string print_rep = print_attr(datatype, loc, (void *)value);
	    at->append_attr(attrname, print_type(datatype), print_rep);
	}

	delete [] value;
    }
}

/** Given a reference to an instance of class DAS and a filename that refers
    to a netcdf file, read the netcdf file and extract all the attributes of
    each of its variables. Add the variables and their attributes to the
    instance of DAS.
*/
void
nc_read_variables(DAS &das, const string &filename)
{
    int ncid, errstat;
    errstat = nc_open(filename.c_str(), NC_NOWRITE, &ncid);
    if (errstat != NC_NOERR) {
        string msg = (string)"Could not open " + path_to_filename(filename) + ".";
        throw Error(errstat, msg);
    }

    // how many variables? how many global attributes?
    int nvars, ngatts;
    errstat = nc_inq(ncid, (int *)0, &nvars, &ngatts, (int *)0);
    if (errstat != NC_NOERR) {
        string msg = (string)"Could not inquire about netcdf file: "
	+ path_to_filename(filename) + ".";
	throw Error(errstat, msg);
    }
#if 0
    nc_type datatype;           /* variable data type */
    int vdimids[MAX_VAR_DIMS];  // variable dimension ids
    size_t vdims[MAX_VAR_DIMS];    /* variable dimension sizes */
    int num_dim;                /* number of dim. in variable */
    errstat = nc_inq_var(ncid, varid, (char *)0, &datatype, &num_dim, vdimids,
                         (int *)0);
#endif
    // for each variable
    char varname[MAX_NC_NAME];
    int natts = 0;
    nc_type var_type;
#if 0
    int num_dim;                /* number of dim. in variable */
#endif
    AttrTable *attr_table_ptr = NULL ;
    for (int v = 0; v < nvars; ++v) {
        errstat = nc_inq_var(ncid, v, varname, &var_type, (int*)0, (int*)0, &natts);
	if (errstat != NC_NOERR) {
	    string msg = "Could not get information for variable ";
	    msg += long_to_string(v);
	    throw Error(errstat, msg);
	}

	attr_table_ptr = das.get_table(varname);
	if (!attr_table_ptr)
	    attr_table_ptr = das.add_table(varname, new AttrTable);

	read_attributes(ncid, v, natts, attr_table_ptr);

        // Add a special attribute for string lengths
	if (var_type == NC_CHAR) {
	    // number of dimensions and size of Nth dimension
	    int num_dim;
	    int vdimids[MAX_VAR_DIMS];  // variable dimension ids
	    errstat = nc_inq_var(ncid, v, (char *)0, (nc_type *)0, &num_dim,
	            vdimids, (int *)0);
	    if (errstat != NC_NOERR)
	        throw Error(errstat, string("Could not read information about a NC_CHAR variable while building the DAS."));

	    if (num_dim == 0) {
	        // a scalar NC_CHAR is stuffed into a string of length 1
	        int size = 1;
                string print_rep = print_attr(NC_LONG, 0, (void *)&size);
                attr_table_ptr->append_attr("string_length", print_type(NC_LONG), print_rep);
	    }
	    else {
                size_t *dim_sizes = new size_t[num_dim];

                for (int i = 0; i < num_dim; ++i)
                    if ((errstat = nc_inq_dimlen(ncid, vdimids[i], &dim_sizes[i])) != NC_NOERR) {
                    	delete[] dim_sizes;
                        throw Error(errstat,
                                string("Could not read dimension information about the variable `")
                                + varname + string("'."));
                    }
#if 0
                errstat = nc_inq_vardimid(ncid, v, dim_sizes);
                if (errstat != NC_NOERR) {
                	delete[] dim_sizes;
                    throw Error(errstat, string("Could not read information about a NC_CHAR variable while building the DAS."));
                }
#endif
                // add attribute
                string print_rep = print_attr(NC_LONG, 0, (void *) (dim_sizes + num_dim - 1));
                attr_table_ptr->append_attr("string_length", print_type(NC_LONG), print_rep);

                delete[] dim_sizes;
            }
	}
    }

    // global attributes
    if (ngatts > 0) {
	attr_table_ptr = das.add_table("NC_GLOBAL", new AttrTable);

	read_attributes(ncid, NC_GLOBAL, ngatts, attr_table_ptr);
    }

    // Add unlimited dimension name in DODS_EXTRA attribute table
    int xdimid;
    char dimname[MAX_NC_NAME];
    nc_type datatype = NC_CHAR;
    nc_inq(ncid, (int *)0, (int *)0, (int *)0, &xdimid);
    if (xdimid != -1){
	nc_inq_dim(ncid, xdimid, dimname, (size_t *)0);
	string print_rep = print_attr(datatype, 0, dimname);
	attr_table_ptr = das.add_table("DODS_EXTRA", new AttrTable);
	attr_table_ptr->append_attr("Unlimited_Dimension",
				    print_type(datatype), print_rep);
    }

    if (nc_close(ncid) != NC_NOERR)
	throw InternalErr(__FILE__, __LINE__,
	                  "ncdds: Could not close the dataset!");
}
