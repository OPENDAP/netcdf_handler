
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

static char not_used rcsid[]={"$Id: ncdas.cc,v 1.6 2003/09/25 23:09:36 jimg Exp $"};

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <string>

#include "cgi_util.h"
#include "Dnetcdf.h"
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

// Given the type, array number and pointer to a attribute, build the printed
// representation of that attribute. 
//
// Largely taken from ncdump. Copyright 1993, University Corporation for
// Atmospheric Research.
//
// Return a char * to newly allocated memory. The caller must call delete [].

static char *
print_attr(nc_type type, int loc, void *vals)
{
    char *rep;			// return value
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
	//char uc;  
	rep = new char [32];
	gp.cp = (char *) vals;

	uc = *(gp.cp+loc);
	sprintf (rep, "%d",uc);

	return rep;
	break;

      case NC_CHAR:
	rep = new char [strlen((const char *)vals) + 3];
	sprintf(rep, "\"%s\"", (char *)vals);
	return rep;
	break;

      case NC_SHORT:
	rep = new char [32];
	gp.sp = (short *) vals;
	sprintf (rep, "%d",*(gp.sp+loc));
	return rep;
	break;

      case NC_LONG:
	rep = new char [32];
	gp.lp = (nclong *) vals; // warning: long int format, int arg (arg 3)
	sprintf (rep, "%d",*(gp.lp+loc));
	return rep;
	break;

      case NC_FLOAT: {
	  char *f_fmt = "%.10g";
	  char gps[30];		// for ascii of a float and double precision

	  rep = new char [32];
	  gp.fp = (float *) vals;
	  int ll;
	  (void) sprintf(gps, f_fmt, * (gp.fp+loc));
	  // make sure we get a decimal point for float type attributes
	  ll = strlen(gps);
	  if (!strchr(gps, '.') && !strchr(gps,'e')) 
	      gps[ll++] = '.';
	  gps[ll] = '\0';
	  sprintf (rep, "%s", gps);
	  return rep;
	  break;
      }
      case NC_DOUBLE: {
	  char *d_fmt = "%.17g";
	  char gps[30];		// for ascii of a float and double precision

	  rep = new char [32];
	  gp.dp = (double *) vals;
	  (void) sprintf(gps, d_fmt, *(gp.dp+loc));
	  // make sure we get a decimal point for float type attributes
	  int ll = strlen(gps);
	  if (!strchr(gps, '.') && !strchr(gps,'e')) 
	      gps[ll++] = '.';
	  gps[ll] = '\0';
	  sprintf (rep, "%s", gps);
	  return rep;
	  break;
      }
      default:
        ErrMsgT("nc_das: print_attr: bad type");
	return NULL;
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
        errstat = lnc_inq_attname(ncid, v, a, attrname);
        if (errstat != NC_NOERR) {
            sprintf (Msgt,"nc_das server: could not get the name for attribute %d",a);
	    ErrMsgT(Msgt); //local error messag
	    *error = (string)"\"" + (string)Msgt + (string)"\"";
	    return errstat;
	}
	errstat = lnc_inq_att(ncid, v, attrname, &datatype, &len);
	if (errstat != NC_NOERR) {
      	    sprintf(Msgt,
		    "nc_das server: could not gettype or length for attribute %s",
		    attrname);
	    ErrMsgT(Msgt); //local error message
	    *error = (string)"\"" + (string)Msgt + (string)"\"";
	    return errstat;
	}

	value = new char [(len + 1) * lnctypelen(datatype)];

	if (!value) {
            ErrMsgT("nc_das server: Out of memory!");
	    *(error) =  (string)"\"nc_das: Out of memory! \"";
	    (void) lnc_close(ncid);
	    return ENOMEM;
	}
	errstat = lnc_get_att(ncid, v, attrname, (void *)value);
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
	for (int loc=0; loc < len ; loc++) {
	    char *print_rep = print_attr(datatype, loc, (void *)value);	
	    at->append_attr(attrname, print_type(datatype), print_rep);
	    delete [] print_rep;
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
read_variables(DAS &das, const string &filename) throw (Error) 
{
    ncopts = 0;
    int ncid, errstat; 
    int nvars, ngatts, natts = 0 ;
    string *error = NULL ;
    AttrTable *attr_table_ptr = NULL ;

    errstat = lnc_open(filename.c_str(), NC_NOWRITE, &ncid);

    if (errstat != NC_NOERR) {
        sprintf (Msgt,"nc_das server: could not open file %s", filename.c_str());
        ErrMsgT(Msgt); //local error message
        string msg = (string)"Could not open " + path_to_filename(filename) + "."; 
        throw Error(errstat, msg); 
    }

    // how many variables? how many global attributes? 
    errstat = lnc_inq(ncid, (int *)0, &nvars, &ngatts, (int *)0);

    if (errstat != NC_NOERR) {
        ErrMsgT("nc_das: Could not inquires about netcdf file");
	string msg = (string)"Could not inquire about netcdf file: " 
	+ path_to_filename(filename) + "."; 
	throw Error(errstat, msg);
    }

    // for each variable
    char varname[MAX_NC_NAME];
    for (int v = 0; v < nvars; ++v) {
        errstat = lnc_inq_var(ncid, v, varname, (nc_type *)0, (int *)0, (int *)0, &natts);
	if (errstat != NC_NOERR) {
            sprintf (Msgt, "nc_das server: could not get information for variable %d",v);
            ErrMsgT(Msgt); //local error message 
	    string msg = (string)Msgt;
	    throw Error(errstat, msg);
	}

	attr_table_ptr = das.get_table(varname);
	if (!attr_table_ptr)
	    attr_table_ptr = das.add_table(varname, new AttrTable);

	errstat = read_attributes(ncid, v, natts, attr_table_ptr, error);
	if (errstat != NC_NOERR){
	  string msg = (string) *error;
	  throw Error(errstat, msg);
	}
    }

    // global attributes
    if (ngatts > 0) {
	attr_table_ptr = das.add_table("NC_GLOBAL", new AttrTable);

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
    (void) lncinquire(ncid, (int *)0, (int *)0, (int *)0, &xdimid);
    if (xdimid != -1){
	(void) lncdiminq(ncid, xdimid, dimname, (long *)0);
	char *print_rep = print_attr(datatype, 0, dimname);	
	attr_table_ptr = das.add_table("DODS_EXTRA", new AttrTable);
	attr_table_ptr->append_attr("Unlimited_Dimension", 
				    print_type(datatype), print_rep);
	delete [] print_rep;
    }

}

#ifdef TEST

int
main(int argc, char *argv[])
{
    DAS das;

    if(!read_variables(das, argv[1], ""))
	abort();

    das.print();
}

#endif

// $Log: ncdas.cc,v $
// Revision 1.6  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.5.4.1  2003/06/06 08:23:41  reza
// Updated the servers to netCDF-3 and fixed error handling between client and server.
//
// Revision 1.5  2003/01/28 07:08:24  jimg
// Merged with release-3-2-8.
//
// Revision 1.3.4.3  2002/12/27 00:39:09  jimg
// Replaced %ld with %d for integer argument in sprintf.
//
// Revision 1.3.4.2  2002/11/06 22:53:23  pwest
// Cleaned up some uninitialized memory read warnings from purify
//
// Revision 1.4  2001/08/30 23:08:24  jimg
// Merged with 3.2.4
//
// Revision 1.3.4.1  2001/06/22 04:45:28  reza
// Added/Fixed exception handling.
//
// Revision 1.3  2000/10/06 01:22:03  jimg
// Moved the CVS Log entries to the ends of files.
// Modified the read() methods to match the new definition in the dap library.
// Added exception handlers in various places to catch exceptions thrown
// by the dap library.
//
// Revision 1.2  1999/11/05 05:15:07  jimg
// Result of merge woth 3-1-0
//
// Revision 1.1.2.1  1999/10/29 05:05:23  jimg
// Reza's fixes plus the configure & Makefile update
//
// Revision 1.1  1999/07/28 00:22:47  jimg
// Added
//
// Revision 1.21  1999/05/08 00:38:20  jimg
// Fixes for the String --> string changes
//
// Revision 1.20  1999/05/07 23:45:33  jimg
// String --> string fixes
//
// Revision 1.19  1999/03/30 05:20:56  reza
// Added support for the new data types (Int16, UInt16, and Float32).
//
// Revision 1.18  1997/03/10 16:24:22  reza
// Added error object and upgraded to DODS core release 2.12.
//
// Revision 1.17  1996/09/17 17:07:05  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.16.2.2  1996/07/10 21:44:44  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.16.2.1  1996/06/25 22:05:07  jimg
// Version 2.0 from Reza.
//
// Revision 1.15  1995/07/09  21:34:01  jimg
// Added copyright notice.
//
// Revision 1.14  1995/06/29  20:29:50  jimg
// Added delete [] of the return value from print_attr(). This patched a
// reported memory leak (reported from dbnew).
//
// Revision 1.13  1995/06/28  20:22:42  jimg
// Replaced malloc calls with calls to new (and calls to free with calls to
// delete).
//
// Revision 1.12  1995/06/27  20:48:52  jimg
// Changed the scope of print_type() and print_attr() to static.
// Fixed an off-by-one error in read_attribtes() when working with NC_CHAR
// attributes (which we represent as strings).
//
// Revision 1.11  1995/06/23  15:35:23  jimg
// Added netio.h.
// Fixed some misc problems which may have come about do to the modifications
// in nc_das.cc and nc_dds.cc (they went from CGIs to simple UNIX filters).
//
// Revision 1.10  1995/03/16  16:38:35  reza
// Updated byte transfer. Bytes are no longer transmitted in binary but in
// an ASCII string (0-255).
//
// Revision 1.9  1995/02/10  04:47:37  reza
// Updated to use type subclasses, NCArray, NCByte, NCInt32, NCGrid....
//
// Revision 1.8  1994/12/22  04:46:20  reza
// Updated to use DODS new attribute array capability.
//
// Revision 1.7  1994/11/03  05:18:27  reza
// Modified the redundant trailing type-code and illegal (in das parser)
// single quotation marks used for byte type.
//
// Revision 1.6  1994/10/28  15:14:15  reza
// Changed some comments
//
// Revision 1.5  1994/10/06  17:53:07  jimg
// Some reformatting of code for emacs, ...
// Fixed Makefile.in so that two targets are built: client and server.
// Fixed cast away of const in ncdas.cc.
//
// Revision 1.4  1994/10/06  16:29:54  jimg
// Fixed printed representation of Strings read from a netcdf file -- they
// now are correctly enclosed in double quotes.
//
// Revision 1.3  1994/10/06  15:52:10  reza
// Changed error messeges
//
// Revision 1.2  1994/10/05  18:00:17  jimg
// Modified so that types (as represented by the DAS) are now handled
// correctly.
//
// Revision 1.1  1994/09/27  23:19:37  jimg
// First version of the netcdf software which used libdas++.a.
//
