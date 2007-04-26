
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
// It also contains test code which will print the in-memeory DAS to
// stdout. It uses both the DAS class as well as the netcdf library.
// In addition, parts of these functions were taken from the netcdf program
// ncdump, from the netcdf standard distribution (ver 2.3.2)
//
// jhrg 9/23/94

#include "config_nc.h"

static char not_used rcsid[]={"$Id$"};

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <netcdf.h>

#include "cgi_util.h"
#include "util.h"
#include "escaping.h"
#include "DAS.h"

// These are used as the return values for print_type().

static const char STRING[]="String";
static const char BYTE[]="Byte";
static const char INT32[]="Int32";
static const char INT16[]="Int16";
static const char FLOAT64[]="Float64";
static const char FLOAT32[]="Float32";

// Used by ErrMsgT

static char Msgt[255];

/** Given the type, array number and pointer to the associated attribute,
    Return the string representaion of the attribute's value. 
    
    This function is modeled on code from ncdump. I modified the original 
    version to use C++ string streams and also to call escattr() so that
    attributes with quotes would be handled correctly. */
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
        rep << "\"" << escattr(static_cast<const char*>(vals)) << "\"";
        return rep.str();

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

// Return the printed representation of a netcdf type -- in a form that dods
// can use.
//
// Returns: a char * referencing the datatype's printed representation. The
// char * is static.

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

// Code borrowed from the netcdf 2 interface compat code. Replace this with a
// direct block of statements and look at maybe using stringstreams to
// generate the print representations. 4/19/05 jhrg
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

// Given the netcdf file id, variable id, number of attributes for the
// variable, and an attribute table pointer, read the attributes and store
// their names and values in the attribute table.
//
// NB: currently values are stored only as strings.
//
// Returns: false if an error was detected reading from the netcdf file, true
// otherwise. 

int
read_attributes(int ncid, int v, int natts, AttrTable *at, string *error)
{
    char attrname[MAX_NC_NAME];
    nc_type datatype;
    size_t len;
    int errstat = NC_NOERR;
    char *value;

    for (int a = 0; a < natts; ++a) {
        errstat = nc_inq_attname(ncid, v, a, attrname);
        if (errstat != NC_NOERR) {
            sprintf (Msgt,"nc_das server: could not get the name for attribute %d",a);
	    ErrMsgT(Msgt); //local error messag
	    *error = (string)"\"" + (string)Msgt + (string)"\"";
	    return errstat;
	}
	errstat = nc_inq_att(ncid, v, attrname, &datatype, &len);
	if (errstat != NC_NOERR) {
      	    sprintf(Msgt,
		    "nc_das server: could not gettype or length for attribute %s",
		    attrname);
	    ErrMsgT(Msgt); //local error message
	    *error = (string)"\"" + (string)Msgt + (string)"\"";
	    return errstat;
	}

	value = new char [(len + 1) * nctypelen(datatype)];

	if (!value) {
            ErrMsgT("nc_das server: Out of memory!");
	    *(error) =  (string)"\"nc_das: Out of memory! \"";
	    (void) nc_close(ncid);
	    return ENOMEM;
	}
	errstat = dap_get_att(ncid, v, attrname, (void *)value);
	if (errstat != NC_NOERR) {
            ErrMsgT("nc_das server: could not read attribute value");
	    *(error) =  (string)"\"nc_das: Could not read attribute value \"";
   	    return errstat;
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
    
    return errstat;
}

// Given a reference to an instance of class DAS and a filename that refers
// to a netcdf file, read the netcdf file and extract all the attributes of
// each of its variables. Add the variables and their attributes to the
// instance of DAS.
//
// Returns: false if an error accessing the netcdf file was detected, true
// otherwise. 

void
nc_read_variables(DAS &das, const string &filename,
		  const string &name, bool multi) throw(Error)
{
    ncopts = 0;
    int ncid, errstat; 
    int nvars, ngatts, natts = 0 ;
    string *error = NULL ;
    AttrTable *attr_table_ptr = NULL ;
    AttrTable *virtual_table_ptr = NULL ;
    if( multi )
    {
	virtual_table_ptr = das.add_table( name, new AttrTable ) ;
    }

    errstat = nc_open(filename.c_str(), NC_NOWRITE, &ncid);

    if (errstat != NC_NOERR) {
        sprintf (Msgt,"nc_das server: could not open file %s", filename.c_str());
        ErrMsgT(Msgt); //local error message
        string msg = (string)"Could not open " + path_to_filename(filename) + "."; 
        throw Error(errstat, msg); 
    }

    // how many variables? how many global attributes? 
    errstat = nc_inq(ncid, (int *)0, &nvars, &ngatts, (int *)0);

    if (errstat != NC_NOERR) {
        ErrMsgT("nc_das: Could not inquires about netcdf file");
	string msg = (string)"Could not inquire about netcdf file: " 
	+ path_to_filename(filename) + "."; 
	throw Error(errstat, msg);
    }

    // for each variable
    char varname[MAX_NC_NAME];
    for (int v = 0; v < nvars; ++v) {
        errstat = nc_inq_var(ncid, v, varname, (nc_type *)0, (int *)0, (int *)0, &natts);
	if (errstat != NC_NOERR) {
            sprintf (Msgt, "nc_das server: could not get information for variable %d",v);
            ErrMsgT(Msgt); //local error message 
	    string msg = (string)Msgt;
	    throw Error(errstat, msg);
	}

	if( multi )
	{
	    attr_table_ptr = virtual_table_ptr->find_container( varname ) ;
	    if( !attr_table_ptr )
		attr_table_ptr = virtual_table_ptr->append_container( varname );
	}
	else
	{
	    attr_table_ptr = das.get_table(varname);
	    if (!attr_table_ptr)
		attr_table_ptr = das.add_table(varname, new AttrTable);
	}

	errstat = read_attributes(ncid, v, natts, attr_table_ptr, error);
	if (errstat != NC_NOERR){
	  string msg = (string) *error;
	  throw Error(errstat, msg);
	}
    }

    // global attributes
    if (ngatts > 0) {
	if( multi )
	{
	    attr_table_ptr = virtual_table_ptr->append_container( "NC_GLOBAL" );
	}
	else
	{
	    attr_table_ptr = das.add_table("NC_GLOBAL", new AttrTable);
	}

	errstat = read_attributes(ncid, NC_GLOBAL, ngatts, attr_table_ptr, error);
	if (errstat != NC_NOERR){
	  string msg = (string) *error;
	  throw Error(errstat, msg);
	}
    }

    // Add unlimited dimension name in DODS_EXTRA attribute table
    int xdimid;
    char dimname[MAX_NC_NAME];
    nc_type datatype = NC_CHAR;
    nc_inq(ncid, (int *)0, (int *)0, (int *)0, &xdimid);
    if (xdimid != -1){
	nc_inq_dim(ncid, xdimid, dimname, (size_t *)0);
	string print_rep = print_attr(datatype, 0, dimname);	
	if( multi )
	{
	    attr_table_ptr = virtual_table_ptr->append_container( "DODS_EXTRA");
	}
	else
	{
	    attr_table_ptr = das.add_table("DODS_EXTRA", new AttrTable);
	}
	attr_table_ptr->append_attr("Unlimited_Dimension", 
				    print_type(datatype), print_rep);
    }

    if (nc_close(ncid) != NC_NOERR)
	throw InternalErr(__FILE__, __LINE__,
	                  "ncdds: Could not close the dataset!");
}
